#include "logger.h"
#include "xbuffer.h"
#include "xsystem.h"
#include "pugixml.hpp"
#include "datetime.h"

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

		xgc_char gConfigPath[_MAX_PATH] = "";

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

			char szFilePath[_MAX_PATH] = { 0 };
			char szDateTime[64] = { 0 };

			datetime::now( szDateTime );

			get_normal_path( szFilePath, sizeof( szFilePath )
						   , "%s\\shm_%s_%03d.%s"
						   , lpPath
						   , szDateTime
						   , clock() % 1000
						   , "log" );

			SHM_LOG( "%s log create", szFilePath );
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
			int cpy1 = _snprintf_s( szMsg, sizeof( szMsg ), "%s %s(%d): ", LogPrefix[nLevel], pszFunction, nLine );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
			{
				return;
			}

			va_list args;
			va_start( args, format );
			int cpy2 = _vsnprintf_s( szMsg + cpy1, sizeof( szMsg ) - cpy1, _TRUNCATE, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
			{
				return;
			}

			_LogToFile( szMsg );
			_LogToShm( szMsg );

			if( IsDebuggerPresent() )
			{
				_snprintf_s( szMsg, cpy1, "%s(%d): %s#", pszFunction, nLine, LogPrefix[nLevel] );
				_LogToDebugger( szMsg );
			}
		}

		void write( EExeLogLvl nLevel, xgc_lpcstr format, ... )
		{
			if( nLevel >= LOGLVL_MAX ) return;
			if( LogSwitch[nLevel] == false ) return;

			char szMsg[LoggerBufSize];
			int cpy1 = _snprintf_s( szMsg, sizeof( szMsg ), "%s : ", LogPrefix[nLevel] );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
				return;

			va_list args;
			va_start( args, format );
			int cpy2 = _vsnprintf_s( szMsg + cpy1, sizeof( szMsg ) - cpy1, _TRUNCATE, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
				return;

			_LogToFile( szMsg );
			_LogToShm( szMsg );

			if( IsDebuggerPresent() )
			{
				_snprintf_s( szMsg, cpy1, ": %s#", LogPrefix[nLevel] );
				_LogToDebugger( szMsg );
			}
		}

		xgc_void write_shared( xgc_lpcstr pszFunction, xgc_int32 nLine, xgc_lpcstr format, ... )
		{
			char szMsg[LoggerBufSize];
			int cpy1 = _snprintf_s( szMsg, sizeof( szMsg ), "%s %s(%d): ", "[SHM_LOG]", pszFunction, nLine );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
				return;

			va_list args;
			va_start( args, format );
			int cpy2 = _vsnprintf_s( szMsg + cpy1, sizeof( szMsg ) - cpy1, sizeof( szMsg ) - cpy1, format, args );
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
			if( 0 != fopen_s( &fp, file, "a+" ) )
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
}
