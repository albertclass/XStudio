#include "defines.h"
#include "logger.h"
#include "xbuffer.h"
#include "xsystem.h"
#include "xutility.h"
#include "pugixml.hpp"
#include "ini_reader.h"

#ifdef _WINDOWS
#	include <io.h>
#	include <fcntl.h>
#endif

#ifdef _LINUX
#	include <sys/stat.h>
#	include <sys/fcntl.h>
#endif

#include <thread>
#include <mutex>
#include <atomic>

using namespace std::placeholders;

///////////////////////////////////////////////////////////////////////////////
// CExeLogger
///////////////////////////////////////////////////////////////////////////////
namespace xgc
{
	using common::datetime;
	using common::ring_buffer;
	using common::reference_buffer;
	using common::shared_memory_buffer;
	using common::union_buffer_recorder;

	using common::ini_reader;
	namespace common
	{
		//////////////////////////////////////////////////////////////////////////
		// logger adapters
		class stdout_adapter : public logger_adapter
		{
			xgc_void join()
			{
				fprintf( stdout, "stdout adapter join" );
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) 
			{
				xgc_long w = (xgc_long)fwrite( data, 1, size, stdout );
				XGC_ASSERT( size == w );

				return w;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class stderr_adapter : public logger_adapter
		{
			xgc_void join() override
			{
				fprintf( stderr, "stderr adapter join" );
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) override
			{
				xgc_long w = (xgc_long)fwrite( data, 1, size, stderr );
				XGC_ASSERT( size == w );

				return w;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class trace_adapter : public logger_adapter
		{
			xgc_void join() override
			{
				#ifdef _WINDOWS
				OutputDebugStringA( "trace adapter join" );
				#endif
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) override
			{
				#ifdef _WINDOWS
				OutputDebugStringA( (char*)data );
				#endif

				return (xgc_long)size;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class shared_adapter : public logger_adapter
		{
		private:
			ring_buffer< shared_memory_buffer > shared;

		public:
			shared_adapter( xgc_lpcstr name, xgc_size size, xgc_lpcstr path )
			{
				auto ret = shared.create(name, size, path);
				XGC_ASSERT_THROW( ret >= 0, std::runtime_error("shared memory create failed!") );
			}
		private:
			xgc_void join() override
			{
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) override
			{
				return (xgc_long)shared.write_overflow( data, size );
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class file_adapter : public logger_adapter
		{
		private:
			xgc_char file[XGC_MAX_FNAME];
			xgc_char path[XGC_MAX_PATH];

			// split by file size
			xgc_size split_size;
			// split by date
			xgc_bool split_date;

			// write size total
			xgc_size split_size_total;
			// split serial no.
			xgc_size split_size_sn;
			// file create time
			datetime create_time;

			// file handle
			FILE*	 file_p;
		public:
			file_adapter( xgc_lpcstr _file, xgc_lpcstr _path, xgc_size _split_size, xgc_bool _split_date )
				: split_size( _split_size ? XGC_RNG( _split_size, 1048576, 4294967296 ) : 4294967296 )
				, split_date( _split_date )
				, split_size_total( 0 )
				, split_size_sn( 0 )
				, file_p( xgc_nullptr )
			{
				strcpy_s( file, _file );
				strcpy_s( path, _path );
			}

			~file_adapter()
			{
				if( file_p )
				{
					fclose( file_p );
				}
			}
		private:
			virtual xgc_void join()
			{
				fflush( file_p );
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) override
			{
				if( split_date && datetime::now().date() != create_time.date() )
				{
					// split by date
					char date[64] = { 0 };
					create_time = datetime::now();
					create_time.now(date, "_%Y%m%d");

					// split filename and ext
					char filename[XGC_MAX_FNAME] = { 0 };
					strcpy_s( filename, file );
					xgc_lpstr dot = strchr( filename, '.' );

					if( dot ) { *dot++ = '\0'; }

					// normal path
					char absolute[XGC_MAX_PATH] = { 0 };
					if( split_size )
					{
						get_absolute_path( absolute, "%s/%s%s_%d.%s", path, filename, date, split_size_sn, dot ? dot : "" );
					}
					else
					{
						get_absolute_path( absolute, "%s/%s%s.%s", path, filename, date, dot ? dot : "" );
					}

					// get file size
					struct stat s;
					if( -1 != stat( absolute, &s ) )
					{
						split_size_total = s.st_size;
					}
					else
					{
						split_size_total = 0;
					}

					// open file
					FILE *file_n = xgc_nullptr;
					if( (file_n = fopen( absolute, "a" )) == xgc_nullptr )
					{
						write_file( "error.log", "open log file %s error.", absolute );
					}
					else
					{
						if( file_p )
							fclose( file_p );

						file_p = file_n;
					}
				}

				while( split_size && split_size_total >= split_size )
				{
					// split by size
					char date[64] = { 0 };
					create_time.to_string(date, "_%Y%m%d");

					// split filename and ext
					char filename[XGC_MAX_FNAME] = { 0 };
					strcpy_s( filename, file );
					xgc_lpstr dot = strchr( filename, '.' );
					if( dot ) { *dot++ = '\0'; }

					// normal path 
					char absolute[XGC_MAX_PATH] = { 0 };
					get_absolute_path( absolute, "%s/%s%s_%d.%s", path, filename, date, ++split_size_sn, dot ? dot : "" );

					// get file size
					struct stat s;
					if( -1 != stat( absolute, &s ) )
					{
						split_size_total = s.st_size;
					}
					else
					{
						split_size_total = 0;
					}

					// open new file.
					FILE *file_n = xgc_nullptr;

					if( (file_n = fopen( absolute, "a" )) == xgc_nullptr )
					{
						write_file( "error.log", "open log file %s error.", absolute );
						break;
					}

					if( file_p ) 
						fclose(file_p);

					file_p = file_n;
				}

				// write log to file
				xgc_long w = (xgc_long)fwrite( data, 1, size, file_p );

				if( size != w )
				{
					XGC_ASSERT( false );
					write_file( "error.log", "write file error total = %ul.", split_size_total );
				}

				split_size_total += w;

				return w;
			}
		};

		///
		/// \brief pipe adapter
		///
		class pipe_adapter : public logger_adapter
		{
		private:
			/// pipe file describtion
			int fd[2];

			/// pipe cache size
			xgc_uint32 pipe_cache_size;

			/// pipe mount adapters
			std::list<logger_adapter*> adapters;

			/// work thread exit flag
			std::atomic< xgc_ulong > work_thread_exit;

			/// work thread
			std::thread		work_thread;

			/// adapters lock guard.
			std::mutex		guard;

			/// write lock
			std::mutex		write_grard_;

			/// 发送缓冲结构
			struct chunk { xgc_lpcvoid data; xgc_size size; };
		public:
			pipe_adapter( xgc_uint32 _buffer_size )
				: fd{ -1, -1 }
				, pipe_cache_size( XGC_RNG(_buffer_size, 1024*64, 1024*1024) )
				, work_thread_exit( 0 )
			{
			}

			~pipe_adapter()
			{
				join();
				_close( fd[0] );
				_close( fd[1] );

				adapters.clear();
			}

			xgc_bool init()
			{
				#ifdef _WINDOWS
				if( -1 == _pipe( fd, (unsigned int)pipe_cache_size, O_BINARY ) )
				{
					write_file( "error.log", "open log pipe error." );
					return false;
				}
				#endif

				#ifdef _LINUX
				if( -1 == pipe( fd ) )
				{
					write_file( "error.log", "open log pipe error." );
					return false;
				}
				fcntl(fd[1], F_SETPIPE_SZ, pipe_cache_size);
				#endif

				work_thread = std::thread( &pipe_adapter::thread, this );
				return true;
			}

			xgc_void add_adapter( logger_adapter* adapter )
			{
				XGC_ASSERT_RETURN( adapter, XGC_NONE );
				std::unique_lock< std::mutex > lock( guard );
				adapters.push_back( adapter );
			}

		private:
			// 读取管道数据
			xgc_long read_bytes( xgc_lpvoid buffer, int bytes )
			{
				int read_bytes = 0;
				int read_total = 0;

				while( read_total < bytes )
				{
					read_bytes = _read( fd[0], (xgc_lpstr)buffer + read_total, bytes - read_total );
					if( read_bytes < 0 && errno != EAGAIN )
						return -1;

					read_total += read_bytes;
				}

				return read_total;
			};

			xgc_void thread()
			{
				logger_context context;

				// 读缓冲
				xgc_byte* buffer = (xgc_byte*)malloc(pipe_cache_size);

				xgc_long bytes = 0;
				xgc_long total = 0;

				while( work_thread_exit == 0 )
				{
					bytes = read_bytes( buffer + total, sizeof( xgc_long ) );
					if( bytes < 0 )
						break;

					total += bytes;

					if( total < sizeof(xgc_long) )
						continue;
					
					bytes = *(xgc_long*)buffer;
					if( bytes >= (xgc_long)pipe_cache_size )
					{
						auto siz = XGC_ALIGN_MEM( bytes, 128 );
						auto ptr = malloc( siz + 4 ); // 留给字符串结束符
						XGC_ASSERT_BREAK( ptr );

						pipe_cache_size = siz;

						buffer = (xgc_byte*)ptr;
					}

					if( total < bytes )
						continue;

					// 计算包长度
					xgc_byte* packet = buffer + 5;
					xgc_long  length = *(xgc_long*)buffer - sizeof( xgc_long ) - 1;

					// check it is exit command
					switch( buffer[4] )
					{
						case 1: // normal write.
						{
							std::lock_guard< std::mutex > lock( guard );
							for( auto adapter : adapters )
							{
								adapter->write( packet, length );
							}
						}
						break;
						case 2: // format write.
						{
							context.file_ = *(xgc_lpcstr*)packet;
							packet += sizeof( xgc_lpcstr );
							length -= sizeof( xgc_lpcstr );
							
							context.func_ = *(xgc_lpcstr*)packet;
							packet += sizeof( xgc_lpcstr );
							length -= sizeof( xgc_lpcstr );

							context.line_ = *(xgc_ulong*)packet;
							packet += sizeof( xgc_ulong );
							length -= sizeof( xgc_ulong );

							xgc_long tags_len = *(xgc_long*)packet;
							packet += sizeof( xgc_ulong );
							length -= sizeof( xgc_ulong );

							context.tags_ = (xgc_lpcstr)packet;
							packet += tags_len;
							length -= tags_len;

							xgc_long logs_len = *(xgc_long*)packet;
							packet += sizeof( xgc_ulong );
							length -= sizeof( xgc_ulong );

							context.logs_ = (xgc_lpcstr)packet;
							packet += logs_len;
							length -= logs_len;

							// 分派给其他日志适配器
							int len = -1;
							std::lock_guard< std::mutex > lock( guard );
							for( auto adapter : adapters )
							{
								if( adapter->write( context ) > 0 )
									// 适配器有自己的格式化需求
									continue;

								if( len < 0 )
									// 格式化日志
									len = formater.parse_log( context );

								if( len > 0 )
									// 直写到适配器
									adapter->write( formater.log(), len );
							}
						}
						break;
						case 0xff:
						{
							work_thread_exit = 1;
						}
						break;
					}

					total -= bytes;
					memmove( buffer, buffer + bytes, total );
				}

				free( buffer );
			}
		private:
			virtual xgc_void join()
			{
				if( work_thread_exit == 0 )
				{
					xgc_uint32 len = sizeof( xgc_uint32 ) + 1;
					struct chunk chunks[] ={
						{ &len, sizeof( len ) },
						{ "\xff", 1 },
					};

					pipe_write( chunks, 3 );

					work_thread.join();
				}
			}

			xgc_long pipe_write( xgc_lpcvoid data, xgc_size size )
			{
				xgc_long total_write = 0;
				while( total_write < size )
				{
					int write_bytes = ::_write( fd[1], (xgc_lpstr)data + total_write, (int)( size - total_write ) );
					XGC_ASSERT_BREAK( -1 != write_bytes );
					total_write += write_bytes;
				}

				return total_write;
			}

			xgc_long pipe_write( struct chunk chunks[], int count )
			{
				xgc_long total_write = 0;

				std::lock_guard< decltype( write_grard_ ) > _lock( write_grard_ );

				for( int i = 0; i < count; ++i )
				{
					auto data = chunks[i].data;
					auto size = chunks[i].size;

					int write_bytes = pipe_write( data, size );
					XGC_ASSERT_BREAK( size == write_bytes );
					total_write += write_bytes;
				}

				return total_write;
			}

			virtual xgc_long write( logger_context& ctx ) override
			{
				// 准备传输的日志上下文
				auto line = ctx.line_;
				auto file = ctx.file_;
				auto func = ctx.func_;
				auto tags = ctx.tags_;
				auto tags_len = (xgc_long)strlen( tags ) + 1;
				auto logs = ctx.logs_;
				auto logs_len = (xgc_long)strlen( logs ) + 1;

				xgc_long len = sizeof( xgc_uint32 ) + 1
					+ sizeof( file )
					+ sizeof( func )
					+ sizeof( line )
					+ sizeof( tags_len )
					+ tags_len
					+ sizeof( logs_len )
					+ logs_len;

				struct chunk chunks[] ={
					{ &len, sizeof( len ) },
					{ "\x2", 1 },
					{ &file, sizeof( file ) },
					{ &func, sizeof( func ) },
					{ &line, sizeof( line ) },
					{ &tags_len, sizeof( tags_len ) },
					{ tags, (xgc_size)tags_len },
					{ &logs_len, sizeof( logs_len ) },
					{ logs, (xgc_size)logs_len },
				};

				return pipe_write( chunks, 9 );
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) override
			{
				// 包的尺寸
				xgc_uint32 len = xgc_uint32( sizeof(size) + 1 + size );

				struct chunk chunks[] = {
					{ &len, sizeof( len ) },
					{ "\x1", 1 },
					{ data, size }
				};

				return pipe_write( chunks, 3 );
			}
		};

		class sock_adapter : public logger_adapter
		{
		private:
			/// pipe file describtion
			int fd[2];

			/// pipe cache size
			xgc_uint32 sock_buffer_size;

			/// udp broadcast port
			xgc_uint32 udp_port;

			/// work thread exit flag
			std::atomic< xgc_ulong > work_thread_exit;

			/// work thread
			std::thread		work_thread;

			/// adapters lock guard.
			std::mutex		guard;

			/// write lock
			std::mutex		write_grard_;

			/// 发送缓冲结构
			struct chunk { xgc_lpcvoid data; xgc_size size; };

		public:
			///
			/// \brief 构造 
			/// \date 1/8/2018
			/// \author albert.xu
			///
			sock_adapter()
			{

			}

			///
			/// \brief 析构
			/// \date 1/8/2018
			/// \author albert.xu
			///
			~sock_adapter()
			{

			}

			// 初始化网络
			xgc_bool init()
			{
				// 开启工作线程
				work_thread = std::thread( &sock_adapter::thread, this );
				return true;
			}

		private:
			xgc_void thread()
			{
				// 写数据线程
			}

			virtual xgc_void join()
			{
				if( work_thread_exit == 0 )
				{
					xgc_uint32 len = sizeof( xgc_uint32 ) + 1;
					struct chunk chunks[] ={
						{ &len, sizeof( len ) },
						{ "\xff", 1 },
					};

					sock_write( chunks, 3 );

					work_thread.join();
				}
			}

			xgc_long sock_write( xgc_lpcvoid data, xgc_size size )
			{
				xgc_long total_write = 0;
				while( total_write < size )
				{
					int write_bytes = ::_write( fd[1], (xgc_lpstr)data + total_write, (int)( size - total_write ) );
					XGC_ASSERT_BREAK( -1 != write_bytes );
					total_write += write_bytes;
				}

				return total_write;
			}

			xgc_long sock_write( struct chunk chunks[], int count )
			{
				xgc_long total_write = 0;

				std::lock_guard< decltype( write_grard_ ) > _lock( write_grard_ );

				for( int i = 0; i < count; ++i )
				{
					auto data = chunks[i].data;
					auto size = chunks[i].size;

					int write_bytes = sock_write( data, size );
					XGC_ASSERT_BREAK( size == write_bytes );
					total_write += write_bytes;
				}

				return total_write;
			}

			virtual xgc_long write( logger_context& ctx ) override
			{
				// 准备传输的日志上下文
				auto line = ctx.line_;
				auto file = ctx.file_;
				auto func = ctx.func_;
				auto tags = ctx.tags_;
				auto tags_len = (xgc_long)strlen( tags ) + 1;
				auto logs = ctx.logs_;
				auto logs_len = (xgc_long)strlen( logs ) + 1;

				xgc_long len = sizeof( xgc_uint32 ) + 1
					+ sizeof( file )
					+ sizeof( func )
					+ sizeof( line )
					+ sizeof( tags_len )
					+ tags_len
					+ sizeof( logs_len )
					+ logs_len;

				struct chunk chunks[] ={
					{ &len, sizeof( len ) },
					{ "\x2", 1 },
					{ &file, sizeof( file ) },
					{ &func, sizeof( func ) },
					{ &line, sizeof( line ) },
					{ &tags_len, sizeof( tags_len ) },
					{ tags, (xgc_size)tags_len },
					{ &logs_len, sizeof( logs_len ) },
					{ logs, (xgc_size)logs_len },
				};

				return sock_write( chunks, 9 );
			}

			xgc_long write( xgc_lpcvoid data, xgc_size size ) override
			{
				// 包的尺寸
				xgc_uint32 len = xgc_uint32( sizeof( size ) + 1 + size );

				struct chunk chunks[] ={
					{ &len, sizeof( len ) },
					{ "\x1", 1 },
					{ data, size }
				};

				return sock_write( chunks, 3 );
			}

		};

		class COMMON_API logger
		{
		private:
			xgc::unordered_map< xgc::string, logger_impl* > loggers;

			xgc::unordered_map< xgc::string, logger_adapter* > adapters;

		public:
			logger_impl& get( xgc_lpcstr name )
			{
				auto it = loggers.find( name );
				if( it != loggers.end() )
					return *(it->second);

				return *(logger_impl*)xgc_nullptr;
			}

			logger_impl& get_or_create( xgc_lpcstr name )
			{
				auto it = loggers.find( name );
				if( it != loggers.end() )
					return *(it->second);

				auto log = XGC_NEW logger_impl( name );
				auto ib = loggers.insert( std::make_pair( name, log ) );
				XGC_ASSERT( ib.second );

				return *(ib.first->second);
			}

			xgc_void clear()
			{
				for( auto it : loggers )
				{
					SAFE_DELETE( it.second );
				}

				loggers.clear();

				for( auto it : adapters )
					it.second->join();

				for( auto it : adapters )
					SAFE_DELETE( it.second );

				adapters.clear();
			}

			///
			/// create logger adapter and store in adapters container
			///
			logger_adapter* create_logger_impl( ini_reader &ini, xgc_lpcstr section_name );
		};

		static logger logger_instance;

		///
		/// create logger adapter and store in adapters container
		///
		logger_adapter* logger::create_logger_impl( ini_reader &ini, xgc_lpcstr section_name )
		{
			auto it = adapters.find( section_name );
			if( it != adapters.end() )
				return it->second;

			// make output adapter
			auto logger_output_conf = ini.get_section(section_name);
			if( xgc_nullptr == logger_output_conf )
				return xgc_nullptr;

			// get device
			auto device = ini.get_item_value( logger_output_conf, "Device", xgc_nullptr );
			if( xgc_nullptr == device )
				return xgc_nullptr;

			auto format = ini.get_item_value( logger_output_conf, "Format", xgc_nullptr );

			logger_adapter *adapter = xgc_nullptr;

			if( strcasecmp( device, "stdout" ) == 0 )
			{
				adapter = XGC_NEW stdout_adapter();
			}
			else if( strcasecmp( device, "stderr" ) == 0 )
			{
				adapter = XGC_NEW stderr_adapter();
			}
			else if( strcasecmp( device, "trace" ) == 0 )
			{
				adapter = XGC_NEW trace_adapter();
			}
			else if( strcasecmp( device, "shared" ) == 0 )
			{
				auto name = ini.get_item_value(logger_output_conf, "Name", xgc_nullptr );
				XGC_ASSERT_RETURN(name, false, "shared memeory conf, name field not found." );
				auto path = ini.get_item_value(logger_output_conf, "Path", xgc_nullptr );
				XGC_ASSERT_RETURN(path, false, "shared memeory conf, path field not found." );
				auto file = ini.get_item_value(logger_output_conf, "File", xgc_nullptr );
				XGC_ASSERT_RETURN(path, false, "shared memeory conf, file field not found." );

				auto size = ini.get_item_value(logger_output_conf, "Size", 1024*1024*4 );

				// normal path
				xgc_char absolute_path[XGC_MAX_PATH] = { 0 };
				auto ret = get_absolute_path( absolute_path, "%s", path );
				XGC_ASSERT_RETURN(path, false, "shared memeory conf, path error.");
				XGC_ASSERT_RETURN( 0 == make_dirs( absolute_path ), false );

				adapter = XGC_NEW shared_adapter( name, size, absolute_path );
			}
			else if( strcasecmp( device, "file" ) == 0 )
			{
				auto file = ini.get_item_value(logger_output_conf, "File", xgc_nullptr );
				XGC_ASSERT_RETURN(file, false, "file conf, file field not found." );

				auto path = ini.get_item_value(logger_output_conf, "Path", xgc_nullptr );
				XGC_ASSERT_RETURN(path, false, "file conf, path field not found." );

				auto split_size = ini.get_item_value(logger_output_conf, "SplitSize", 1024*1024*1024 );
				auto split_date = ini.get_item_value(logger_output_conf, "SplitDate", true );

				// normal path
				xgc_char absolute[XGC_MAX_PATH] = { 0 };
				path = get_absolute_path( absolute, "%s/", path );
				XGC_ASSERT_RETURN(path, false, "file conf, path error.");

				XGC_ASSERT_RETURN( 0 == make_dirs( path ), false );

				adapter = XGC_NEW file_adapter( file, path, split_size, split_date );
			}
			else if( strcasecmp( device, "pipe" ) == 0 )
			{
				auto output = ini.get_item_value(logger_output_conf, "Output", xgc_nullptr );
				XGC_ASSERT_RETURN( output, false, "pipe conf, output field not found." );

				auto length = ini.get_item_value(logger_output_conf, "Size", 1048576 );

				adapter = XGC_NEW pipe_adapter( length );
				XGC_ASSERT_RETURN( adapter, false, "pipe conf, create adapter failed." );
				((pipe_adapter*)adapter)->init();

				auto outputs = string_split< xgc::vector >( output, " ," );
				for( auto &name : outputs )
				{
					auto redirect = logger_instance.create_logger_impl( ini, name.c_str() );
					XGC_ASSERT_CONTINUE( redirect, "pipe conf, create redirect failed." );
					((pipe_adapter*)adapter)->add_adapter( redirect );
				}
			}
			else if( strcasecmp( device, "socket" ) == 0 )
			{

			}

			if( adapter )
			{
				adapter->parse_format( format );

				auto ib = adapters.insert( std::make_pair( section_name, adapter ) );
				XGC_ASSERT( ib.second );
				if( false == ib.second )
					SAFE_DELETE( adapter );
			}

			return adapter;
		}

		xgc_bool init_logger( xgc_lpcstr path )
		{
			ini_reader ini;
			if( false == ini.load( path ) )
				return false;

			return init_logger( ini );
		}

		xgc_bool init_logger( ini_reader &ini )
		{
			auto logger = ini.get_section("Logger");
			XGC_ASSERT_RETURN( logger, false );

			auto logger_count = ini.get_item_count(logger);
			for( xgc_size i = 0; i < logger_count; ++i )
			{
				auto logger_name = ini.get_item_name(logger, i);
				auto logger_open = ini.get_item_value(logger, i, "disable");
				if( strcasecmp(logger_open, "enable") != 0 )
					continue;

				auto logger_conf = ini.get_section(logger_name);
				XGC_ASSERT_CONTINUE( logger_conf, "logger conf %s not found", logger_name );

				auto logger_output = ini.get_item_value(logger_conf, "Output", "stderr");
				XGC_ASSERT_CONTINUE( logger_output, "Output field not found." );

				// 日志格式化串
				auto logger_format = ini.get_item_value(logger_conf, "Format", xgc_nullptr );
				// 日志过滤包含模式关键字
				auto logger_filter_include = ini.get_item_value( logger_conf, "FilterInclude", xgc_nullptr );
				// 日志过滤排除模式关键字
				auto logger_filter_exclude = ini.get_item_value( logger_conf, "FilterExclude", xgc_nullptr );
				// 日志过滤模式
				auto logger_filter_mode = ini.get_item_value( logger_conf, "FilterMode", 0 );

				// 创建日志
				auto &impl = new_logger(logger_name);

				// parse logger output format
				impl.parse_format( logger_format );

				if( logger_filter_include )
				{
					auto keywords = string_split( logger_filter_include, ", " );
					for( auto &keyword : keywords )
					{
						impl.add_include( std::move( keyword ) );
					}
				}

				if( logger_filter_exclude )
				{
					auto keywords = string_split( logger_filter_exclude, ", " );
					for( auto &keyword : keywords )
					{
						impl.add_exclude( std::move( keyword ) );
					}
				}

				impl.set_filter_mode( logger_filter_mode );

				// check output adapter is exist;
				auto outputs = string_split(logger_output, " ,");
				for( auto &name : outputs )
				{
					auto adapter = logger_instance.create_logger_impl( ini, name.c_str() );
					XGC_ASSERT_CONTINUE( adapter );
					impl.add_adapter( adapter );
				}
			}
			return true;
		}

		xgc_void fini_logger()
		{
			logger_instance.clear();
		}

		xgc_void write_file( xgc_lpcstr file, xgc_lpcstr format, ... )
		{
			FILE *fp = xgc_nullptr;
			if( (fp = fopen( file, "a" )) == xgc_nullptr )
				return;

			if( xgc_nullptr == fp )
				return;

			va_list args;
			va_start( args, format );
			vfprintf_s( fp, format, args );
			va_end( args );

			fclose( fp );
		}

		logger_impl& get_logger( xgc_lpcstr name )
		{
			return logger_instance.get(name);
		}

		logger_impl& new_logger( xgc_lpcstr name )
		{
			return logger_instance.get_or_create(name);
		}

		logger_impl::logger_impl( xgc_lpcstr name )
			: filter_mode( 0 )
		{
			// 设置日志名
			strcpy_s( logger_name, name );
		}

		logger_impl::~logger_impl()
		{
			adapters.clear();
			filter_include.clear();
			filter_exclude.clear();
		}


		///
		/// \brief 是否包含过滤字
		/// \date 1/5/2018
		/// \author albert.xu
		///
		xgc_bool logger_impl::filter( xgc_lpcstr tags )
		{
			switch( filter_mode )
			{
				case 1: // include mode
				{
					auto tag_list = string_split( tags, ", " );
					for( auto &tag : tag_list )
						if( filter_include.find( tag ) != filter_include.end() )
							return false;
				}
				break;
				case 2: // exclude mode
				{
					auto tag_list = string_split( tags, ", " );
					for( auto &tag : tag_list )
						if( filter_exclude.find( tag ) == filter_exclude.end() )
							return false;
				}
				break;
			}

			return true;
		}

		///
		/// \brief 写日志
		///
		/// [01/16/2017 albert.xu]
		///
		xgc_void logger_impl::write( logger_context context, xgc_lpcstr fmt, ... )
		{
			// 尽量使用宏定义的方式来使用该函数
			if( xgc_nullptr == this )
				return;

			char logs[4096];

			va_list args;
			va_start( args, fmt );
			int cpy = vsprintf_s( logs, fmt, args );
			if( cpy == -1 )
				return;

			if( cpy >= sizeof( logs ) )
				logs[sizeof( logs ) - 1] = 0;

			va_end( args );

			context.logs_ = logs;
			context.logs_len_ = cpy;

			xgc_long len = -1;
			for( auto adapter : adapters )
			{
				if( adapter->write( context ) > 0 )
					// 适配器有自己的格式化需求
					continue;

				if( len < 0 )
					// 格式化日志
					len = formater.parse_log( context );

				if( len > 0 )
					// 直写到适配器
					adapter->write( formater.log(), len );
			}
		}

		///
		/// logger_formater
		///

		///
		/// \brief 构造日志格式化对象
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		logger_formater::logger_formater()
			: message_( (xgc_lpstr)malloc( 1024 ) )
		{
		}

		///
		/// \brief 析构日志格式化对象
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		logger_formater::~logger_formater()
		{
			free( message_ );
		}

		///
		/// \brief 解析格式化字符串
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		xgc_void logger_formater::parse_format( xgc_lpcstr fmt )
		{
			auto ptr = fmt;
			auto beg = fmt;

			format.clear();
			xgc_bool in_match = false;
			xgc_long barckets = 0;

			while( ptr && *ptr )
			{
				if( *ptr == '$' && *( ptr + 1 ) == '(' )
				{
					if( ptr > beg )
					{
						format.emplace_back( std::bind( &logger_formater::span, _1, _3, _4, string( beg, ptr - beg ) ) );
					}

					if( strncmp( ptr, "$(date)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::datetime, _1, _3, _4, "%Y-%m-%d" ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(time)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::datetime, _1, _3, _4, "%H:%M:%S" ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(datetime)", 11 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::datetime, _1, _3, _4, "%Y-%m-%d %H:%M:%S" ) );
						beg = ptr += 11;
					}
					else if( strncmp( ptr, "$(tags)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::tags, _1, _2, _3, _4 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(file)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::file, _1, _2, _3, _4 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(path)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::path, _1, _2, _3, _4 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(func)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::func, _1, _2, _3, _4 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(line)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::line, _1, _2, _3, _4 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(endl)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::endl, _1, _3, _4 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(message)", 10 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::message, _1, _2, _3, _4 ) );
						beg = ptr += 10;
					}
				}
				else
				{
					++ptr;
				}
			}

			if( ptr > beg )
			{
				format.emplace_back( std::bind( &logger_formater::span, _1, _3, _4, string( beg, ptr - beg ) ) );
			}
		}

		///
		/// \brief 解析格式化字符串
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		xgc_ulong logger_formater::parse_message( const logger_context &ctx, xgc_char** message )
		{
			if( message == xgc_nullptr )
				return -1;

			auto buf = *message;
			auto len = *message ? memsize( *message ) : 0;

			// 复制的字节数
			int cpy = 0;

			// 初始化缓冲写指针
			xgc_ulong pos = 0;

			// 循环写入
			for( auto &span : format )
			{
				do
				{
					cpy = (int)span( this, std::cref(ctx), buf + pos, len - pos );
					if( cpy >= len - pos - 1 )
					{
						// 日志缓冲不足的，则扩大缓冲
						buf = (xgc_lpstr)realloc( *message, len + 256 );
						if( buf == xgc_nullptr )
						{
							if( *message != message_ )
							{
								free( *message );
								*message = xgc_nullptr;
							}

							return -1;
						}

						len += 1024;
						*message = buf;

						continue;
					}
				} while( false );

				pos += cpy;
			}

			return pos;
		}

		XGC_INLINE xgc_size logger_formater::datetime( xgc_char * buf, xgc_size len, xgc_lpcstr fmt )
		{
			return datetime::now( buf, len, fmt );
		}

		///
		/// \brief 解析格式化字符串
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		//xgc_long logger_formater::parse_message( const logger_context &ctx, xgc_char* message, xgc_size size )
		//{
		//	int pos = 0;
		//	int cpy = 0;

		//	// 循环写入
		//	for( auto &span : format )
		//	{
		//		cpy = (int)span( ctx, message + pos, size - pos );
		//		if( cpy < 0 )
		//			return -1;

		//		pos += cpy;
		//	}

		//	return pos;
		//}
		logger_context::logger_context()
			: line_( 0 )
			, file_( xgc_nullptr )
			, func_( xgc_nullptr )
			, tags_( xgc_nullptr )
			, logs_( xgc_nullptr )
		{
		}

		logger_context::logger_context( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags )
			: line_( line )
			, file_( file )
			, func_( func )
			, tags_( tags )
			, logs_( xgc_nullptr )
		{
		}

		logger_context::logger_context( const logger_context & ctx )
			: line_( ctx.line_ )
			, file_( ctx.file_ )
			, func_( ctx.func_ )
			, tags_( ctx.tags_ )
			, logs_( ctx.logs_ )
		{
		}

		logger_context::~logger_context()
		{

		}
	}
}
