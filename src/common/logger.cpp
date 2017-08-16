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
			virtual xgc_void join()
			{
				fprintf( stdout, "stdout adapter join" );
			}

			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				XGC_ASSERT( 1 == fwrite( data, size, 1, stdout ) );
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class stderr_adapter : public logger_adapter
		{
			virtual xgc_void join()
			{
				fprintf( stderr, "stderr adapter join" );
			}

			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				XGC_ASSERT( 1 == fwrite( data, size, 1, stderr ) );
			}
		};

		//////////////////////////////////////////////////////////////////////////
		class trace_adapter : public logger_adapter
		{
			virtual xgc_void join()
			{
				#ifdef _WINDOWS
				OutputDebugStringA( "trace adapter join" );
				#endif
			}

			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				#ifdef _WINDOWS
				OutputDebugStringA( (char*)data );
				#endif
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
			virtual xgc_void join()
			{
			}

			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				shared.write_overflow( data, size );
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

			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
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
					xgc_lpcstr ext = "";
					xgc_lpstr dot = strchr( filename, '.' );
					if( dot )
					{
						*dot = '\0';
						ext = dot + 1;
					}

					// normal path
					char absolute[XGC_MAX_PATH] = { 0 };
					if( split_size )
					{
						get_absolute_path( absolute, "%s/%s%s_%d.%s", path, filename, date, split_size_sn, ext );
					}
					else
					{
						get_absolute_path( absolute, "%s/%s%s.%s", path, filename, date, ext );
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
					xgc_lpcstr ext = "";
					xgc_lpstr dot = strchr( filename, '.' );
					if( dot )
					{
						*dot = '\0';
						ext = dot + 1;
					}

					// normal path 
					char absolute[XGC_MAX_PATH] = { 0 };
					get_absolute_path( absolute, "%s/%s%s_%d.%s", path, filename, date, ++split_size_sn, ext );

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
				if( size != fwrite( data, 1, size, file_p ) )
				{
					XGC_ASSERT( false );
					write_file( "error.log", "write file error total = %ul.", split_size_total );
					return;
				}

				split_size_total += size;
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

			// 日志输出上下文
			struct context
			{
				/// 命令标示
				xgc_byte command;
				/// 文件名
				xgc_lpcstr file;
				/// 函数名
				xgc_lpcstr func;
				/// 标签
				xgc_lpcstr tags;
				/// 行号
				xgc_ulong line;
				/// 缓冲区
				xgc_ulong size;
				
			} context_;


			/// 发送缓冲结构
			struct chunk { xgc_lpvoid data; xgc_size size; };
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

			xgc_void thread()
			{
				// 读缓冲
				xgc_byte* buffer = (xgc_byte*)malloc(pipe_cache_size);

				context* _ctx = xgc_nullptr;

				auto read_bytes = [this]( xgc_lpvoid buffer, int bytes )->xgc_long {
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

				auto dispatch_adapter = [this]( logger_formater::context &ctx, xgc_lpcstr fmt, ... ){
					va_list args;
					va_start( args, fmt );

					ctx.fmt = fmt;
					va_copy( ctx.args, args );

					auto text = va_arg( args, void* );
					auto size = va_arg( args, xgc_ulong );

					int len = -1;
					std::lock_guard< std::mutex > lock( guard );
					for( auto adapter : adapters )
					{
						if( -1 == adapter->write( ctx ) )
						{
							adapter->write( text, size );
						}
					}

					va_end(args);
				};

				while( work_thread_exit == 0 )
				{
					xgc_long bytes = 0;
					xgc_long total = 0;

					bytes = read_bytes( buffer + total, sizeof( context ) );
					if( bytes < 0 )
						break;

					total += bytes;

					context* _ctx = (context*)buffer;

					// check it is exit command
					switch( _ctx->command )
					{
						case 1: // normal write.
						{
							if( total + _ctx->size >= pipe_cache_size )
							{
								auto siz = XGC_ALIGN_MEM( total + _ctx->size, 128 );
								auto ptr = malloc( siz + 4 ); // 留给字符串结束符
								XGC_ASSERT_BREAK( ptr );

								pipe_cache_size = siz;

								buffer = (xgc_byte*)ptr;
								// 内存重新分配后要将_ctx重置
								_ctx = (context*)buffer;
							}

							bytes = read_bytes( buffer + total, _ctx->size );
							if( bytes < 0 )
								break;

							xgc_lpstr message = ( xgc_lpstr )buffer + total;
							total += bytes;

							buffer[total] = 0;

							logger_formater::context ctx;
							ctx.file = _ctx->file;
							ctx.func = _ctx->file;
							ctx.line = _ctx->line;
							ctx.tags = _ctx->tags;

							dispatch_adapter( ctx, "%s", message, _ctx->size );
						}
						break;
						case 0xff:
						{
							work_thread_exit = 1;
						}
						break;
					}
				}

				free( buffer );
			}
		private:
			virtual xgc_void join()
			{
				if( work_thread_exit == 0 )
				{
					context_.command = 0xff;
					write( xgc_nullptr, 0 );
					work_thread.join();
				}
			}

			xgc_size real_write( xgc_lpvoid data, xgc_size size )
			{
				xgc_size total_write = 0;
				while( total_write < size )
				{
					int write_bytes = ::_write( fd[1], (xgc_lpstr)data + total_write, (int)( size - total_write ) );
					XGC_ASSERT_BREAK( -1 != write_bytes );
					total_write += write_bytes;
				}

				return total_write;
			}

			xgc_size real_write( struct chunk chunks[], int count )
			{
				xgc_size total_write = 0;

				std::lock_guard< decltype( write_grard_ ) > _lock( write_grard_ );

				for( int i = 0; i < count; ++i )
				{
					auto data = chunks[i].data;
					auto size = chunks[i].size;

					int write_bytes = (int)real_write( data, size );
					XGC_ASSERT_BREAK( -1 != write_bytes );
					total_write += write_bytes;
				}

				return total_write;
			}

			virtual xgc_long write( const logger_formater::context& ctx )
			{
				// 准备传输的日志上下文
				context_.command = 1;
				context_.file = ctx.file;
				context_.line = ctx.line;
				context_.func = ctx.func;
				context_.tags = ctx.tags;

				if( formater.empty() )
					return -1;

				xgc_long len = formater.parse_message( ctx );
				if( len > 0 )
					write( formater.log(), len );

				return len;
			}

			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				context_.size = (xgc_ulong)size;
				struct chunk chunks[] = {
					{ &context_, sizeof( context_ ) },
					{ data, size }
				};

				real_write( chunks, 2 );
			}
		};

		class COMMON_API logger
		{
		private:
			xgc_unordered_map< xgc_string, logger_impl* > loggers;

			xgc_unordered_map< xgc_string, logger_adapter* > adapters;

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

				auto outputs = string_split< xgc_vector >( output, " ," );
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
				// 日志过滤关键字
				auto logger_filter = ini.get_item_value(logger_conf, "Filter", xgc_nullptr );

				auto &impl = new_logger(logger_name);

				// parse logger output format
				impl.parse_format( logger_format );

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
		/// \brief 写日志
		///
		/// [01/16/2017 albert.xu]
		///
		xgc_void logger_impl::write( xgc_lpcstr file, xgc_lpcstr func, xgc_ulong line, xgc_lpcstr tags, xgc_lpcstr fmt, ... )
		{
			// 尽量使用宏定义的方式来使用该函数
			if( xgc_nullptr == this )
				return;

			va_list args;
			va_start( args, fmt );

			logger_formater::context ctx;

			ctx.file = file;
			ctx.func = func;
			ctx.tags = tags;
			ctx.line = line;
			ctx.fmt = fmt;
			va_copy( ctx.args, args );

			xgc_size len = -1;
			for( auto adapter : adapters )
			{
				if( -1 == adapter->write( ctx ) )
				{
					if( -1 == len )
					{
						len = formater.parse_message( ctx );
					}

					if( -1 != len )
					{
						adapter->write( formater.log(), len );
					}
				}
			}

			va_end( ctx.args );
			va_end( args );
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
						format.emplace_back( std::bind( &logger_formater::span, _2, _3, xgc_string( beg, ptr - beg ) ) );
					}

					if( strncmp( ptr, "$(date)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::date, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(time)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::time, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(datetime)", 13 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::datetime, _2, _3 ) );
						beg = ptr += 13;
					}
					else if( strncmp( ptr, "$(tags)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::tags, _1, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(file)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::file, _1, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(path)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::path, _1, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(func)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::func, _1, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(line)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::line, _1, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(endl)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::endl, _2, _3 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(message)", 10 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_formater::message, _1, _2, _3 ) );
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
				format.emplace_back( std::bind( &logger_formater::span, _2, _3, xgc_string( beg, ptr - beg ) ) );
			}
		}

		///
		/// \brief 解析格式化字符串
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		xgc_long logger_formater::parse_message( const context &ctx )
		{
			return parse_message( ctx, &message_ );
		}

		///
		/// \brief 解析格式化字符串
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		xgc_long logger_formater::parse_message( const context &ctx, xgc_char** message )
		{
			if( message == xgc_nullptr )
				return -1;

			auto buf = *message;
			auto len = *message ? memsize( *message ) : 0;

			// 复制的字节数
			int cpy = 0;

			// 初始化缓冲写指针
			xgc_long pos = 0;

			// 循环写入
			for( auto &span : format )
			{
				do
				{
					cpy = (int)span( std::ref( ctx ), buf + pos, len - pos );
					if( cpy >= len - pos - 1 )
					{
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

		///
		/// \brief 解析格式化字符串
		/// \author Albert.xu
		/// \date 2017/07/25
		///
		xgc_long logger_formater::parse_message( const context &ctx, xgc_char* message, xgc_size size )
		{
			int pos = 0;
			int cpy = 0;

			// 循环写入
			for( auto &span : format )
			{
				cpy = (int)span( std::ref( ctx ), message + pos, size - pos );
				if( cpy < 0 )
					return -1;

				pos += cpy;
			}

			return pos;
		}

		inline xgc_size logger_formater::date( xgc_char * buf, xgc_size len )
		{
			return datetime::now( buf, len, "%Y-%m-%d" );
		}

		inline xgc_size logger_formater::time( xgc_char * buf, xgc_size len )
		{
			return datetime::now( buf, len, "%H:%M:%S" );
		}

		inline xgc_size logger_formater::datetime( xgc_char * buf, xgc_size len )
		{
			return datetime::now( buf, len, "%Y-%m-%d %H:%M:%S" );
		}

		inline xgc_size logger_formater::path( const context &ctx, xgc_char * buf, xgc_size len )
		{
			return sprintf_s( buf, len, "%s", ctx.file );
		}

		inline xgc_size logger_formater::file( const context &ctx, xgc_char * buf, xgc_size len )
		{
			auto last_slash = ctx.file;
			auto next_slash = ctx.file;
			while( next_slash = strpbrk( last_slash + 1, SLASH_ALL ) )
				last_slash = next_slash;

			return sprintf_s( buf, len, "%s", last_slash + 1 );
		}

		inline xgc_size logger_formater::func( const context &ctx, xgc_char * buf, xgc_size len )
		{
			return sprintf_s( buf, len, "%s", ctx.func );
		}

		inline xgc_size logger_formater::tags( const context &ctx, xgc_char * buf, xgc_size len )
		{
			return sprintf_s( buf, len, "%s", ctx.tags );
		}

		inline xgc_size logger_formater::line( const context &ctx, xgc_char * buf, xgc_size len )
		{
			return sprintf_s( buf, len, "(%lu)", ctx.line );
		}

		inline xgc_size logger_formater::span( xgc_char * buf, xgc_size len, const xgc_string & span )
		{
			return sprintf_s( buf, len, "%s", span.c_str() );
		}

		inline xgc_size logger_formater::endl( xgc_char * buf, xgc_size len )
		{
			return sprintf_s( buf, len, "\n" );
		}

		inline xgc_size logger_formater::message( const context &ctx, xgc_char * buf, xgc_size len )
		{
			va_list args;
			va_copy( args, const_cast< context& >( ctx ).args );
			return vsprintf_s( buf, len, ctx.fmt, args );
			va_end( args );
		}
	}
}
