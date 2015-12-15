#include "stdafx.h"
#include <direct.h>

#include "ServerBase.h"
#include "ServerLogger.h"
#include "ServerConsole.h"

using namespace SGDP;
static ISDFileLogger* gpFileLogger = xgc_nullptr; // 日志指针
static xgc_char szLoggerPath[_MAX_PATH];

xgc_void FileLoggerCallback( xgc_lpvoid context, xgc_lpcstr text, xgc_size size )
{
	if( gpFileLogger )
	{
		gpFileLogger->LogText( text );
	}

	if( false == IsServerService() )
	{
		ConsoleLog( text );
	}
}

xgc_bool InitializeLogger( IniFile &ini )
{
	FUNCTION_BEGIN;
	// 日志配置
	xgc_lpcstr lpLogPath = ini.GetItemValue( "Log", "Path", "Log" );
	xgc_lpcstr lpLogName = ini.GetItemValue( "Log", "Name", GetServerName() );

	xgc_lpcstr lpLogConf = ini.GetItemValue( "Log", "Conf", "LogConfig.xml" );

	gpFileLogger = SDCreateFileLogger( &SDLOGGER_VERSION );
	XGC_ASSERT_RETURN( gpFileLogger, false );

	// 创建日志目录
	GetNormalPath( szLoggerPath, sizeof( szLoggerPath ), "%s\\", lpLogPath );

	if( _access( szLoggerPath, 0 ) == -1 && _mkdir( szLoggerPath ) == -1 )
	{
		SYS_ERROR( "Log目录无效或创建失败 Path - %s", szLoggerPath );
		return false;
	}

	xgc_char szExceptionLog[_MAX_PATH];
	GetLogPath( szExceptionLog, sizeof( szExceptionLog ), "%s", "exception.log" );
	SetExceptionLog( szExceptionLog );

	GetLogPath( szExceptionLog, sizeof( szExceptionLog ), "%s", "exception.more.log" );
	SetExceptionExt( szExceptionLog );

	GetLogPath( szExceptionLog, sizeof( szExceptionLog ), "%s", "exception\\" );
	SetExceptionDmp( szExceptionLog );

	GetLogPath( szExceptionLog, sizeof( szExceptionLog ), "%s", "debuger.log" );
	SetDebugerLog( szExceptionLog );

	if( gpFileLogger->Init( szLoggerPath, lpLogName, LOG_MODE_DAY_DIVIDE ) == false )
	{
		SYS_ERROR( "初始化文件日志模块失败。日志路径%s", szLoggerPath );
		return false;
	}

	SYS_INFO( "初始化文件日志模块成功。日志路径%s", szLoggerPath );

	// 设置网络日志回调接口
	if( false == SDNetSetLogger( gpFileLogger, LOGLV_DEBUG | LOGLV_INFO | LOGLV_WARN | LOGLV_CRITICAL ) )
	{
		SYS_ERROR( "设置网络日志回调接口失败。" );
		return false;
	}

	// 设置管道日志回调接口
	if( false == SDPipeSetLogger( gpFileLogger, LOGLV_DEBUG | LOGLV_INFO | LOGLV_WARN | LOGLV_CRITICAL ) )
	{
		SYS_ERROR( "设置管道日志回调接口失败。" );
		return false;
	}

	// 初始化底层日志模块
	xgc_char szLogConfPath[_MAX_PATH] = { 0 };
	GetNormalPath( szLogConfPath, sizeof( szLogConfPath ), "%s", lpLogConf );
	if( false == XGC::Logger::InitExeLogger( szLogConfPath, FileLoggerCallback, xgc_nullptr ) )
	{
		SYS_ERROR( "初始化日志模块失败..." );
		return false;
	}

	// 初始化共享内存日志模块
	xgc_char szShmName[_MAX_FNAME];
	xgc_size cpy = sprintf_s( szShmName, sizeof( szShmName ), "%s_Shm", lpLogName );
	XGC_ASSERT_RETURN( cpy > 0, false );

	if( false == XGC::Logger::InitShmLogger( szLoggerPath, szShmName ) )
	{
		SYS_ERROR( "初始化共享内存模块失败..." );
		return false;
	}

	FUNCTION_END;
	return true;
}

xgc_void FinializeLogger()
{
	XGC::Logger::FiniExeLogger();

	SAFE_RELEASE( gpFileLogger );
}

xgc_void WriteServerLog( xgc_lpcstr text )
{
	FUNCTION_BEGIN;
	if( gpFileLogger )
	{
		gpFileLogger->LogText( text );
	}
	FUNCTION_END;
}

xgc_lpcstr GetLogPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... )
{
	int cpy1 = sprintf_s( szPath, nSize, "%s", szLoggerPath );
	XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );

	va_list args;
	va_start( args, lpRelativePath );
	int cpy2 = vsprintf_s( szPath + cpy1, nSize - cpy1, lpRelativePath, args );
	va_end( args );

	if( cpy2 < 0 )
		return xgc_nullptr;

	if( cpy2 == sizeof( szPath ) - cpy1 )
	{
		return xgc_nullptr;
	}

	return _fullpath( szPath, szPath, nSize );
}

xgc_lpcstr GetLogPath()
{
	return szLoggerPath;
}
