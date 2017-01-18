#include "logger.h"
#include "ini_reader.h"
#include "xbuffer.h"
#include "xsystem.h"
#include "pugixml.hpp"
#include "datetime.h"

#ifdef _WINDOWS
#	include <io.h>
#	include <fcntl.h>
#endif

#ifdef _LINUX
#endif

#include <thread>
#include <mutex>
#include <atomic>

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
	namespace logger
	{
		const int ShmLogLineWidth = 256;
		const int ShmLogLineCount = 1024;
		const int LoggerBufSize	  = 1024 * 4;

		xgc_bool m_bLogToFile = true;
		xgc_bool m_bLogToDebugger = true;

		shared_memory_buffer gspLoggerShm;

		LoggerCallback fnCallback = xgc_nullptr;
		xgc_lpvoid gpContext = xgc_nullptr;

		xgc_bool LogSwitch[LOGLVL_MAX] =
		{
			true,   //LOGLVL_SYS_ERROR
			true,   //LOGLVL_SYS_WARNING,
			true,	//LOGLVL_SYS_INFO,
			true,   //LOGLVL_USR_ERROR,
			true,   //LOGLVL_USR_WARNING,
			false,  //LOGLVL_USR_INFO,
			true,	//LOGLVL_DBG_ERROR, 
			false,  //LOGLVL_DBG_WARNING, 
			false,  //LOGLVL_DBG_INFO,
			true,	//LOGLVL_SCRIPT_ERROR, 
			true,   //LOGLVL_SCRIPT_WARNING, 
			true,   //LOGLVL_SCRIPT_INFO,
		};

		xgc_lpcstr LogPrefix[LOGLVL_MAX] =
		{
			"[SYS_ERR]",	//LOGLVL_SYS_ERROR
			"[SYS_WRN]",	//LOGLVL_SYS_WARNING
			"[SYS_INF]",	//LOGLVL_SYS_INFO
			"[USR_ERR]",	//LOGLVL_USR_ERROR
			"[USR_WRN]",	//LOGLVL_USR_WARNING
			"[USR_INF]",	//LOGLVL_USR_INFO
			"[DBG_ERR]",	//LOGLVL_DBG_ERROR
			"[DBG_WRN]",	//LOGLVL_DBG_WARNING
			"[DBG_INF]",	//LOGLVL_DBG_INFO
			"[SRT_ERR]",	//LOGLVL_SCRIPT_ERROR
			"[SRT_WRN]",	//LOGLVL_SCRIPT_WARNING
			"[SRT_INF]",	//LOGLVL_SCRIPT_INFO
		};

		xgc_char gConfigPath[XGC_MAX_PATH] = "";

		const char* ELEM_CONFIG = "config";
		const char* ELEM_LOG = "log";
		const char* ELEM_LEVELS = "levels";
		const char* ELEM_LEVEL = "level";
		const char* ATTR_ID = "id";
		const char* ATTR_LOG = "log";
		const char* ELEM_TARGETS = "targets";
		const char* ELEM_TARGET = "target";

		const char* VAL_SYS_ERROR = "SYS_ERROR";
		const char* VAL_SYS_WARNING = "SYS_WARNING";
		const char* VAL_SYS_INFO = "SYS_INFO";
		const char* VAL_USR_ERROR = "USR_ERROR";
		const char* VAL_USR_WARNING = "USR_WARNING";
		const char* VAL_USR_INFO = "USR_INFO";
		const char* VAL_DBG_ERROR = "DBG_ERROR";
		const char* VAL_DBG_WARNING = "DBG_WARNING";
		const char* VAL_DBG_INFO = "DBG_INFO";

		const char* VAL_CRITICAL = "CRITICAL";
		const char* VAL_WARN = "WARN";
		const char* VAL_INFO = "INFO";
		const char* VAL_DBG = "DBG";
		const char* VAL_FILE = "FILE";
		const char* VAL_DEBUGGER = "DEBUGGER";

		void write_log( xgc_lpcstr pszFunction, xgc_int32 nLine, EExeLogLvl nLevel, xgc_lpcstr format, ... );

		xgc_bool loadcfg()
		{
			SYS_INFO( "Module %s Begin Load Log Config File %s", get_module_name(), gConfigPath );

			pugi::xml_document doc;
			if( !doc.load_file( gConfigPath ) )
			{
				SYS_INFO( "Load Log Config File %s ", gConfigPath );
				return false;
			}

			if( !doc.child( ELEM_CONFIG ) )
			{
				SYS_INFO( "Can not find element %s", ELEM_CONFIG );
				return false;
			}

			if( !doc.child( ELEM_CONFIG ).child( ELEM_LOG ) )
			{
				SYS_INFO( "Can not Find element %s", ELEM_LOG );
				return false;
			}

			if( !doc.child( ELEM_CONFIG ).child( ELEM_LOG ).child( ELEM_LEVELS ) )
			{
				SYS_INFO( "Can not Find element %s", ELEM_LEVELS );
				return false;
			}

			std::string strID;
			std::string strLog;
			EExeLogLvl nLevel;
			xgc_bool bLog;

			auto root = doc.child( ELEM_CONFIG ).child( ELEM_LOG );
			for( auto node = root.child( ELEM_LEVELS ).child( ELEM_LEVEL ); node; node = node.next_sibling( ELEM_LEVEL ) )
			{
				strID = node.attribute( ATTR_ID ).as_string();
				if( strID.empty() )
				{
					SYS_INFO( "elem %s, attr %s is empty", ELEM_LEVEL, ATTR_ID );
					continue;
				}

				strLog = node.attribute( ELEM_LOG ).as_string();
				if( strLog.empty() )
				{
					SYS_INFO( "elem %s, attr %s is empty", ELEM_LEVEL, ATTR_LOG );
					continue;
				}

				if( strID == VAL_SYS_ERROR )
				{
					nLevel = LOGLVL_SYS_ERROR;
				}
				else if( strID == VAL_SYS_WARNING )
				{
					nLevel = LOGLVL_SYS_WARNING;
				}
				else if( strID == VAL_SYS_INFO )
				{
					nLevel = LOGLVL_SYS_INFO;
				}
				else if( strID == VAL_USR_ERROR )
				{
					nLevel = LOGLVL_USR_ERROR;
				}
				else if( strID == VAL_USR_WARNING )
				{
					nLevel = LOGLVL_USR_WARNING;
				}
				else if( strID == VAL_USR_INFO )
				{
					nLevel = LOGLVL_USR_INFO;
				}

				else if( strID == VAL_DBG_ERROR )
				{
					nLevel = LOGLVL_DBG_ERROR;
				}
				else if( strID == VAL_DBG_WARNING )
				{
					nLevel = LOGLVL_DBG_WARNING;
				}

				else if( strID == VAL_DBG_INFO )
				{
					nLevel = LOGLVL_DBG_INFO;
				}
				else
				{
					SYS_INFO( "elem %s, attr %s is unknown value %s", ELEM_LEVEL, ATTR_ID, strID.c_str() );
					continue;
				}

				if( strLog == "Y" || strLog == "y" )
					bLog = true;
				else
					bLog = false;

				LogSwitch[nLevel] = bLog;

				SYS_INFO( "Log Level %s: %s", strID.c_str(), strLog.c_str() );
			}

			if( !root.child( ELEM_TARGETS ) )
			{
				SYS_INFO( "Can not Find element %s", ELEM_TARGETS );
				return false;
			}

			for( auto node = root.child( ELEM_TARGETS ).child( ELEM_TARGET ); node; node = node.next_sibling( ELEM_TARGET ) )
			{
				strID = node.attribute( ATTR_ID ).as_string();
				if( strID.empty() )
				{
					SYS_INFO( "elem %s, attr %s is empty", ELEM_TARGET, ATTR_ID );
					continue;
				}

				strLog = node.attribute( ELEM_LOG ).as_string();
				if( strLog.empty() )
				{
					SYS_INFO( "elem %s, attr %s is empty", ELEM_TARGET, ATTR_LOG );
					continue;
				}

				if( strLog == "Y" || strLog == "y" )
					bLog = true;
				else
					bLog = false;

				if( strID == VAL_FILE )
					m_bLogToFile = bLog;
				else if( strID == VAL_DEBUGGER )
					m_bLogToDebugger = bLog;

				SYS_INFO( "Log Target %s:%s", strID.c_str(), strLog.c_str() );
			}

			SYS_INFO( "Load Log Config File Success!" );

			return true;
		}

		xgc_bool init_logger( xgc_lpcstr pszConfig, LoggerCallback pfnCallback, xgc_lpvoid pContext )
		{
			strcpy_s( gConfigPath, pszConfig );

			fnCallback = pfnCallback;
			gpContext = pContext;

			return loadcfg();
		}

		xgc_bool init_logger_shared( xgc_lpcstr lpPath, xgc_lpcstr lpShmName )
		{
			auto ret = gspLoggerShm.create( lpShmName, ShmLogLineCount*ShmLogLineWidth, lpPath );

			if( -1 == ret )
			{
				fprintf( stderr, "create shm logger failed!" );
				return false;
			}
			else if( 0 == ret )
			{
				
			}

			char sz_filepath[XGC_MAX_PATH] = { 0 };
			char sz_dateTime[64] = { 0 };

			datetime::now( sz_dateTime );

			get_normal_path( sz_filepath
						   , "%s\\shm_%s_%03d.%s"
						   , lpPath
						   , sz_dateTime
						   , clock() % 1000
						   , "log" );

			SHM_LOG( "%s log create", sz_filepath );
			return true;
		}

		xgc_bool _LogToFile( xgc_lpcstr pStrLogMsg )
		{
			if( m_bLogToFile && fnCallback )
			{
				fnCallback( gpContext, pStrLogMsg, -1 );
			}
			return true;
		}

		xgc_bool _LogToShm( xgc_lpcstr pStrLogMsg )
		{
			if( gspLoggerShm.is_create() )
			{
				xgc_char szSyncLog[ShmLogLineWidth] = { 0 };
				xgc_long tid = get_thread_id();

				char szDateTime[64];
				datetime::now( szDateTime );
				clock_t c = clock();
				int cpy = sprintf_s( szSyncLog, "[%s:%04d][tid:%d]%s\n"
									, szDateTime
									, c % 1000
									, tid, pStrLogMsg );

				if( cpy > 0 )
				{
					szSyncLog[ShmLogLineWidth - 1] = 0;
					ring_buffer< reference_buffer, union_buffer_recorder > shm( gspLoggerShm );
					shm.write_some( szSyncLog, cpy );

					return true;
				}
			}
			
			return false;
		}

		template< size_t size >
		xgc_bool _LogToDebugger( xgc_char ( &buf )[size] )
		{
			if( m_bLogToDebugger )
			{
				strcat_s( buf, size, "\n" );
				#ifdef WIN32
				OutputDebugStringA( buf );
				#else
				// printf(szMsg);
				#endif
			}
			return true;
		}

		void write( xgc_lpcstr pszFunction, xgc_int32 nLine, EExeLogLvl nLevel, xgc_lpcstr format, ... )
		{
			if( nLevel >= LOGLVL_MAX ) return;
			if( LogSwitch[nLevel] == false ) return;

			char szMsg[LoggerBufSize];
			int cpy1 = snprintf_s( szMsg, sizeof( szMsg ), "%s %s(%d): ", LogPrefix[nLevel], pszFunction, nLine );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
			{
				return;
			}

			va_list args;
			va_start( args, format );
			int cpy2 = vsnprintf_s( szMsg + cpy1, sizeof( szMsg ) - cpy1, _TRUNCATE, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
			{
				return;
			}

			_LogToFile( szMsg );
			_LogToShm( szMsg );

			#if defined(_WINDOWS)
			if( IsDebuggerPresent() )
			{
				_snprintf_s( szMsg, cpy1, "%s(%d): %s#", pszFunction, nLine, LogPrefix[nLevel] );
				_LogToDebugger( szMsg );
			}
			#endif
		}

		void write( EExeLogLvl nLevel, xgc_lpcstr format, ... )
		{
			if( nLevel >= LOGLVL_MAX ) return;
			if( LogSwitch[nLevel] == false ) return;

			char szMsg[LoggerBufSize];
			int cpy1 = snprintf_s( szMsg, sizeof( szMsg ), "%s : ", LogPrefix[nLevel] );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
				return;

			va_list args;
			va_start( args, format );
			int cpy2 = vsnprintf_s( szMsg + cpy1, sizeof( szMsg ) - cpy1, _TRUNCATE, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
				return;

			_LogToFile( szMsg );
			_LogToShm( szMsg );

			#if defined(_WINDOWS)
			if( IsDebuggerPresent() )
			{
				_snprintf_s( szMsg, cpy1, ": %s#", LogPrefix[nLevel] );
				_LogToDebugger( szMsg );
			}
			#endif
		}

		xgc_void write_shared( xgc_lpcstr pszFunction, xgc_int32 nLine, xgc_lpcstr format, ... )
		{
			char szMsg[LoggerBufSize];
			int cpy1 = snprintf_s( szMsg, sizeof( szMsg ), "%s %s(%d): ", "[SHM_LOG]", pszFunction, nLine );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
				return;

			va_list args;
			va_start( args, format );
			int cpy2 = vsnprintf_s( szMsg + cpy1, sizeof( szMsg ) - cpy1, sizeof( szMsg ) - cpy1, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
				return;

			_LogToShm( szMsg );
			va_end( args );
		}

		xgc_void fini_logger()
		{
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
	}

	namespace common
	{
		static logger logger_instance;

		class stdout_adapter : public logger_adapter
		{
			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				XGC_ASSERT( 1 == fwrite( data, size, 1, stdout ) );
			}
		};

		class stderr_adapter : public logger_adapter
		{
			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				XGC_ASSERT( 1 == fwrite( data, size, 1, stderr ) );
			}
		};

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
			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				shared.write_overflow( data, size );
			}
		};

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
					char *ext = "";
					char *dot = strchr( filename, '.' );
					if( dot )
					{
						*dot = 0;
						ext = dot + 1;
					}

					// normal path
					char absolute[XGC_MAX_PATH] = { 0 };
					if( split_size )
					{
						get_normal_path( absolute, "%s/%s%s(%d).%s", path, filename, date, split_size_sn, ext );
					}
					else
					{
						get_normal_path( absolute, "%s/%s%s.%s", path, filename, date, ext );
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
					char *ext = "";
					char *dot = strchr( filename, '.' );
					if( dot )
					{
						*dot = 0;
						ext = dot + 1;
					}

					// normal path 
					char absolute[XGC_MAX_PATH] = { 0 };
					get_normal_path( absolute, "%s/%s%s(%d).%s", path, filename, date, ++split_size_sn, ext );

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
				if( 1 != fwrite( data, size, 1, file_p ) )
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

			/// pipe buffer size
			xgc_size buffer_size;

			/// pipe mount adapters
			std::vector<logger_adapter *> adapters;

			/// work thread exit flag
			std::atomic< xgc_ulong > work_thread_exit;

			/// work thread
			std::thread		work_thread;

			/// adapters lock guard.
			std::mutex		guard;
		public:
			pipe_adapter( xgc_size _buffer_size )
				: fd{ -1, -1 }
				, buffer_size( _buffer_size )
				, work_thread_exit( 0 )
			{
			}

			~pipe_adapter()
			{
				work_thread_exit = clock();
				write( "##############\n", 15 );
				work_thread.join();

				_close( fd[0] );
				_close( fd[1] );

				for( auto adapter : adapters )
					SAFE_DELETE( adapter );

				adapters.clear();
			}

			xgc_bool init()
			{
				#ifdef _WINDOWS
				if( -1 == _pipe( fd, 1024 * 1024, O_BINARY ) )
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
				void* buffer = malloc(buffer_size);

				while( work_thread_exit == 0 )
				{
					int readbytes = _read( fd[0], buffer, (xgc_uint32) buffer_size );
					if( readbytes > 0 )
					{
						std::unique_lock< std::mutex > lock( guard );
						for( auto adapter : adapters )
						{
							adapter->write( buffer, readbytes );
						}
					}
				}

				work_thread_exit = clock();
				// delay exit read maybe wait a moment
				while( clock() - work_thread_exit < (buffer_size >> 10) )
				{
					// check is end of pipe
					if( !_eof(fd[0]) )
					{
						// block read pipe date
						int readbytes = _read( fd[0], buffer, (xgc_uint32) buffer_size );
						if( readbytes > 0 )
						{
							std::unique_lock< std::mutex > lock( guard );
							for( auto adapter : adapters )
							{
								adapter->write( buffer, readbytes );
							}
						}

						// refresh delay clock
						work_thread_exit = clock();
					}
					else
					{
						std::this_thread::sleep_for( std::chrono::milliseconds(1) );
					}
				}


				free( buffer );
			}
		private:
			virtual xgc_void write( xgc_lpvoid data, xgc_size size )
			{
				while( size )
				{
					int write_bytes = _write( fd[1], data, (xgc_uint32)size );
					XGC_ASSERT_BREAK( -1 != write_bytes );
					size -= write_bytes;
				}
			}
		};

		static logger_adapter* create_logger_impl( ini_reader &ini, xgc_lpcstr section_name )
		{
			// make output adapter
			auto logger_output_conf = ini.get_section(section_name);
			// get device
			auto device = ini.get_item_value( logger_output_conf, "device", xgc_nullptr );
			if( strcasecmp( device, "stdout" ) == 0 )
			{
				return XGC_NEW stdout_adapter();
			}
			else if( strcasecmp( device, "stderr" ) == 0 )
			{
				return XGC_NEW stderr_adapter();
			}
			else if( strcasecmp( device, "shared" ) == 0 )
			{
				auto name = ini.get_item_value(logger_output_conf, "name", xgc_nullptr );
				XGC_ASSERT_RETURN(name, false, "shared memeory conf, name field not found." );
				auto path = ini.get_item_value(logger_output_conf, "path", xgc_nullptr );
				XGC_ASSERT_RETURN(path, false, "shared memeory conf, path field not found." );

				auto size = ini.get_item_value(logger_output_conf, "size", 1024*1024*4 );

				// normal path
				xgc_char absolute[XGC_MAX_PATH] = { 0 };
				path = get_normal_path( absolute, "%s/", path );
				XGC_ASSERT_RETURN(path, false, "shared memeory conf, path error.");

				XGC_ASSERT_RETURN( 0 == makepath( path ), false );
				return XGC_NEW shared_adapter( name, size, path );
			}
			else if( strcasecmp( device, "file" ) == 0 )
			{
				auto file = ini.get_item_value(logger_output_conf, "file", xgc_nullptr );
				XGC_ASSERT_RETURN(file, false, "file conf, file field not found." );

				auto path = ini.get_item_value(logger_output_conf, "path", xgc_nullptr );
				XGC_ASSERT_RETURN(path, false, "file conf, path field not found." );

				auto split_size = ini.get_item_value(logger_output_conf, "split_size", 1024*1024*1024 );
				auto split_date = ini.get_item_value(logger_output_conf, "split_date", true );

				// normal path
				xgc_char absolute[XGC_MAX_PATH] = { 0 };
				path = get_normal_path( absolute, "%s/", path );
				XGC_ASSERT_RETURN(path, false, "file conf, path error.");

				XGC_ASSERT_RETURN( 0 == makepath( path ), false );

				return XGC_NEW file_adapter( file, path, split_size, split_date );
			}
			else if( strcasecmp( device, "pipe" ) == 0 )
			{
				auto output = ini.get_item_value(logger_output_conf, "output", xgc_nullptr );
				XGC_ASSERT_RETURN( output, false, "pipe conf, output field not found." );

				auto length = ini.get_item_value(logger_output_conf, "size", 1048576 );

				auto adapter = XGC_NEW pipe_adapter( length );
				XGC_ASSERT_RETURN( adapter, false, "pipe conf, create adapter failed." );
				adapter->init();

				auto outputs = string_split( output, " ," );
				for( auto &name : outputs )
				{
					auto redirect = create_logger_impl( ini, name.c_str() );
					XGC_ASSERT_CONTINUE( redirect, "pipe conf, create redirect failed." );
					adapter->add_adapter( redirect );
				}
				return adapter;
			}
			else if( strcasecmp( device, "socket" ) == 0 )
			{

			}

			return xgc_nullptr;
		}

		xgc_bool init_logger( xgc_lpcstr path )
		{
			ini_reader ini;
			if( false == ini.load( path ) )
				return false;

			auto logger = ini.get_section("logger");
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

				auto logger_output = ini.get_item_value(logger_conf, "output", "stderr");
				XGC_ASSERT_CONTINUE( logger_output, "output field not found." );
				auto logger_format = ini.get_item_value(logger_conf, "format", "$(date) $(time):[$(tags)]:$(message)");
				XGC_ASSERT_CONTINUE( logger_format, "format field not found." );
				auto logger_filter = ini.get_item_value(logger_conf, "filter", xgc_nullptr);

				auto &impl = new_logger(logger_name);
				// check output is exist;
				auto outputs = string_split(logger_output, " ,");
				for( auto &name : outputs )
				{
					auto adapter = create_logger_impl( ini, name.c_str() );
					XGC_ASSERT_CONTINUE( adapter );
					impl.add_adapter( adapter );
				}

				impl.parse_format(logger_format);
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

		xgc_void logger_impl::parse_format( xgc_lpcstr fmt )
		{
			auto ptr = fmt;
			auto beg = fmt;

			format.clear();
			xgc_bool in_match = false;
			xgc_long barckets = 0;

			while( ptr && *ptr )
			{
				if( *ptr == '$' && *(ptr + 1) == '(' )
				{
					if( ptr > beg )
					{
						format.emplace_back( std::bind( &logger_impl::span, this, std::placeholders::_1, std::placeholders::_2, xgc_string( beg, ptr - beg ) ) );
					}

					if( strncmp( ptr, "$(date)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::date, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(time)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::time, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(datetime)", 13 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::datetime, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 13;
					}
					else if( strncmp( ptr, "$(tags)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::tags, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(file)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::file, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(func)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::func, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(line)", 7 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::line, this, std::placeholders::_1, std::placeholders::_2 ) );
						beg = ptr += 7;
					}
					else if( strncmp( ptr, "$(message)", 10 ) == 0 )
					{
						format.emplace_back( std::bind( &logger_impl::message, this, std::placeholders::_1, std::placeholders::_2 ) );
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
				format.emplace_back( std::bind( &logger_impl::span, this, std::placeholders::_1, std::placeholders::_2, xgc_string( beg, ptr - beg ) ) );
			}
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

			xgc_char log[2048] = { 0 };
			xgc_size len = 0;

			va_list args;
			va_start( args, fmt );
			context.file = file;
			context.func = func;
			context.tags = tags;
			context.line = line;
			context.fmt = fmt;
			context.args = args;

			for( auto &span : format )
			{
				auto cpy = span( log + len, sizeof( log ) - len );
				if( cpy < 0 )
					cpy = sprintf_s( log + len, sizeof( log ) - len, "<format error>" );

				len += cpy;
			}

			for( auto adapter : adapters )
			{
				adapter->write( log, len );
			}
			va_end( args );
		}

	}
}
