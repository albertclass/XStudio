#include "logger.h"
#include "shmbuf.h"
#include "xsystem.h"
#include "markupstl.h"
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
// CExeLogger
///////////////////////////////////////////////////////////////////////////////
namespace XGC
{
	namespace Logger
	{
		const int ShmLogLineWidth = 256;
		const int ShmLogLineCount = 1024;
		const int LoggerBufSize   = 1024 * 4;

		xgc_bool		m_bLogToFile		= true;
		xgc_bool		m_bLogToDebugger	= true;

		ShmBuffer*		gspLoggerShm = xgc_nullptr;

		LoggerCallback	fnCallback	= xgc_nullptr;
		xgc_lpvoid		gpContext	= xgc_nullptr;

		xgc_bool		LogSwitch[LOGLVL_MAX] =
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

		xgc_lpcstr	LogPrefix[LOGLVL_MAX] =
		{
			"[SYS_ERR]",	//LOGLVL_SYS_ERROR
			"[SYS_WRN]",	//LOGLVL_SYS_WARNING,
			"[SYS_INF]",	//LOGLVL_SYS_INFO,
			"[USR_ERR]",	//LOGLVL_USR_ERROR,
			"[USR_WRN]",	//LOGLVL_USR_WARNING,
			"[USR_INF]",	//LOGLVL_USR_INFO,
			"[DBG_ERR]",	//LOGLVL_DBG_ERROR, 
			"[DBG_WRN]",	//LOGLVL_DBG_WARNING, 
			"[DBG_INF]",	//LOGLVL_DBG_INFO,    
			"[SRT_ERR]",	//LOGLVL_SCRIPT_ERROR, 
			"[SRT_WRN]",	//LOGLVL_SCRIPT_WARNING, 
			"[SRT_INF]",	//LOGLVL_SCRIPT_INFO,    
		};

		xgc_char gConfigPath[_MAX_PATH] = "";

		const char* ELEM_CONFIG		    = "config";
		const char* ELEM_LOG			= "log";
		const char* ELEM_LEVELS		    = "levels";
		const char* ELEM_LEVEL		    = "level";
		const char* ATTR_ID			    = "id";
		const char* ATTR_LOG			= "log";
		const char* ELEM_TARGETS		= "targets";
		const char* ELEM_TARGET		    = "target";

		const char* VAL_SYS_ERROR	    = "SYS_ERROR";
		const char* VAL_SYS_WARNING	    = "SYS_WARNING";
		const char* VAL_SYS_INFO		= "SYS_INFO";
		const char* VAL_USR_ERROR	    = "USR_ERROR";
		const char* VAL_USR_WARNING	    = "USR_WARNING";
		const char* VAL_USR_INFO		= "USR_INFO";
		const char* VAL_DBG_ERROR	    = "DBG_ERROR";
		const char* VAL_DBG_WARNING	    = "DBG_WARNING";
		const char* VAL_DBG_INFO		= "DBG_INFO";

		const char* VAL_CRITICAL		= "CRITICAL";
		const char* VAL_WARN			= "WARN";
		const char* VAL_INFO			= "INFO";
		const char* VAL_DBG			    = "DBG";
		const char* VAL_FILE			= "FILE";
		const char* VAL_DEBUGGER		= "DEBUGGER";

		void WriteLog( xgc_lpcstr pszFunction, xgc_int32 nLine, EExeLogLvl nLevel, xgc_lpcstr format, ... );

		xgc_bool loadcfg()
		{
			SYS_INFO( "Module %s Begin Load Log Config File %s", GetModuleName(), gConfigPath );

			CMarkupSTL oXml;
			if( false == oXml.Load( gConfigPath ) )
			{
				SYS_INFO( "Load Log Config File %s failed: %s", gConfigPath, oXml.GetError( ).c_str( ) );
				return false;
			}

			if( false == oXml.FindElem( ELEM_CONFIG ) )
			{
				SYS_INFO( "Can not find element %s", ELEM_CONFIG );
				return false;
			}
			if( false == oXml.IntoElem() )
			{
				SYS_INFO( "Fail to into element %s", ELEM_CONFIG );
				return false;
			}

			if( false == oXml.FindElem( ELEM_LOG ) )
			{
				SYS_INFO( "Can not Find element %s", ELEM_LOG );
				return false;
			}
			if( false == oXml.IntoElem() )
			{
				SYS_INFO( "Fail to into element %s", ELEM_LOG );
				return false;
			}

			if( false == oXml.FindElem( ELEM_LEVELS ) )
			{
				SYS_INFO( "Can not Find element %s", ELEM_LEVELS );
				return false;
			}
			if( false == oXml.IntoElem() )
			{
				SYS_INFO( "Fail to into element %s", ELEM_LEVELS );
				return false;
			}

			std::string strID;
			std::string strLog;
			EExeLogLvl nLevel;
			xgc_bool bLog;

			while( oXml.FindElem( ELEM_LEVEL ) )
			{
				strID = oXml.GetAttrib( ATTR_ID );
				if( strID.empty() )
				{
					SYS_INFO( "elem %s, attr %s is empty", ELEM_LEVEL, ATTR_ID );
					continue;
				}

				strLog = oXml.GetAttrib( ELEM_LOG );
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

			oXml.OutOfElem();

			oXml.ResetChildPos();
			if( false == oXml.FindElem( ELEM_TARGETS ) )
			{
				SYS_INFO( "Can not Find element %s", ELEM_TARGETS );
				return false;
			}
			if( false == oXml.IntoElem() )
			{
				SYS_INFO( "Fail to into element %s", ELEM_TARGETS );
				return false;
			}
			while( oXml.FindElem( ELEM_TARGET ) )
			{
				strID = oXml.GetAttrib( ATTR_ID );
				if( strID.empty() )
				{
					SYS_INFO( "elem %s, attr %s is empty", ELEM_TARGET, ATTR_ID );
					continue;
				}

				strLog = oXml.GetAttrib( ELEM_LOG );
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

		xgc_bool InitExeLogger( xgc_lpcstr pszConfig, LoggerCallback pfnCallback, xgc_lpvoid pContext )
		{
			strcpy_s( gConfigPath, pszConfig );

			fnCallback = pfnCallback;
			gpContext = pContext;

			return loadcfg();
		}

		//-----------------------------------//
		// [12/16/2013 albert.xu]
		// 重载日志配置
		//-----------------------------------//
		xgc_bool Reload()
		{
			return loadcfg();
		}

		xgc_string GenerateShmFileName( xgc_lpcstr lpPath, xgc_lpcstr lpExt )
		{
			char szFilePath[_MAX_PATH] = { 0 };
			time_t now = time( xgc_nullptr );
			clock_t c = clock();
			struct tm *tt = localtime( &now );

			GetNormalPath( szFilePath, sizeof( szFilePath )
				, "%s\\shm_%4d%02d%02d_%02d%02d%02d_%03d.%s"
				, lpPath
				, tt->tm_year, tt->tm_mon + 1, tt->tm_mday
				, tt->tm_hour, tt->tm_min, tt->tm_sec, c % 1000
				, lpExt );

			return szFilePath;
		}

		xgc_bool InitShmLogger( xgc_lpcstr lpPath, xgc_lpcstr lpShmName )
		{
			gspLoggerShm = CreateShmBuffer( lpShmName, ShmLogLineCount*ShmLogLineWidth, lpPath, true );
			if( NULL == gspLoggerShm )
			{
				fprintf( stderr, "create shm logger failed!" );
				return false;
			}
			else if( false == gspLoggerShm->IsCreated() )
			{
				gspLoggerShm->ForceWriteToFile( GenerateShmFileName( lpPath, "log" ).c_str() );
			}

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
			DWORD tid = GetCurrentThreadId();

			time_t now = time( xgc_nullptr );
			clock_t c = clock();
			struct tm *tt = localtime( &now );

			int cpy = _snprintf( szSyncLog, sizeof(szSyncLog), "[%4d-%02d-%02d %02d:%02d:%02d:%04d][tid:%d]%s\n"
				, tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday
				, tt->tm_hour, tt->tm_min, tt->tm_sec, c % 1000
				, tid, pStrLogMsg );

			if( cpy > 0 )
			{
				szSyncLog[ShmLogLineWidth - 1] = 0;
				if( gspLoggerShm )
					gspLoggerShm->Write( xBuffer( szSyncLog, cpy ), true );

				return true;
			}

			return false;
		}

		xgc_bool _LogToDebugger( char* pStrLogMsg )
		{
			if( m_bLogToDebugger )
			{
				strcat( pStrLogMsg, "\n" );
#ifdef WIN32
				OutputDebugStringA( pStrLogMsg );
#else
				// printf(szMsg);
#endif
			}
			return true;
		}

		void WriteLog( xgc_lpcstr pszFunction, xgc_int32 nLine, EExeLogLvl nLevel, xgc_lpcstr format, ... )
		{
			if( nLevel >= LOGLVL_MAX ) return;
			if( LogSwitch[nLevel] == false ) return;

			char szMsg[LoggerBufSize];
			int cpy1 = _snprintf( szMsg, sizeof( szMsg ), "%s %s(%d): ", LogPrefix[nLevel], pszFunction, nLine );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
			{
				return;
			}

			va_list args;
			va_start( args, format );
			int cpy2 = _vsnprintf( szMsg + cpy1, sizeof( szMsg ) - cpy1, format, args );
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
				_snprintf( szMsg, cpy1, "%s(%d): %s#", pszFunction, nLine, LogPrefix[nLevel] );
				_LogToDebugger( szMsg );
			}
		}

		void WriteLog( EExeLogLvl nLevel, xgc_lpcstr format, ... )
		{
			if( nLevel >= LOGLVL_MAX ) return;
			if( LogSwitch[nLevel] == false ) return;

			char szMsg[LoggerBufSize];
			int cpy1 = _snprintf( szMsg, sizeof( szMsg ), "%s : ", LogPrefix[nLevel] );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
				return;

			va_list args;
			va_start( args, format );
			int cpy2 = _vsnprintf( szMsg + cpy1, sizeof( szMsg ) - cpy1, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
				return;

			_LogToFile( szMsg );
			_LogToShm( szMsg );

			if( IsDebuggerPresent() )
			{
				_snprintf( szMsg, cpy1, ": %s#", LogPrefix[nLevel] );
				_LogToDebugger( szMsg );
			}
		}

		xgc_void WriteShm( xgc_lpcstr pszFunction, xgc_int32 nLine, xgc_lpcstr format, ... )
		{
			char szMsg[LoggerBufSize];
			int cpy1 = _snprintf( szMsg, sizeof( szMsg ), "%s %s(%d): ", "[SHM_LOG]", pszFunction, nLine );
			if( cpy1 < 0 )
				return;

			if( cpy1 == sizeof( szMsg ) )
				return;

			va_list args;
			va_start( args, format );
			int cpy2 = _vsnprintf( szMsg + cpy1, sizeof( szMsg ) - cpy1, format, args );
			va_end( args );

			if( cpy2 < 0 )
				return;

			if( cpy2 == sizeof( szMsg ) - cpy1 )
				return;

			_LogToShm( szMsg );
			va_end( args );
		}

		xgc_void FiniExeLogger()
		{
			SAFE_DELETE( gspLoggerShm );
		}

		xgc_void FileLog( xgc_lpcstr file, xgc_lpcstr format, ... )
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
