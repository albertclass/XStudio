#include "ServerDefines.h"
#include "ServerLogger.h"

///
/// 获取服务器名
/// [11/27/2014] create by albert.xu
///
extern xgc_lpcstr ServerName();

///
/// \brief 初始化日志模块
/// \date 1/13/2014 
/// \author albert.xu
///
xgc_bool InitializeLogger( ini_reader &ini )
{
	FUNCTION_BEGIN;
	// 日志配置
	xgc_lpcstr lpLogPath = ini.get_item_value( "Log", "Path", "Log" );
	xgc_lpcstr lpLogName = ini.get_item_value( "Log", "Name", ServerName() );

	// 创建日志目录
	xgc_char szLoggerPath[XGC_MAX_PATH];
	get_absolute_path( szLoggerPath, sizeof( szLoggerPath ), "%s\\", lpLogPath );

	if( _access( szLoggerPath, 0 ) == -1 && make_dirs( szLoggerPath ) == -1 )
	{
		SYS_ERROR( "Log目录无效或创建失败 Path - %s", szLoggerPath );
		return false;
	}

	xgc_char szExceptionLog[XGC_MAX_PATH];
	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "%s\\exception.log", lpLogPath );
	SetExceptionLog( szExceptionLog );

	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "%s\\exception.more.log", lpLogPath );
	SetExceptionExt( szExceptionLog );

	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "%s\\exception.dmp", lpLogPath );
	SetExceptionDmp( szExceptionLog );

	get_absolute_path( szExceptionLog, sizeof( szExceptionLog ), "%s\\debuger.log", lpLogPath );
	SetDebugerLog( szExceptionLog );

	if( init_logger( ini ) == false )
	{
		SYS_ERROR( "初始化文件日志模块失败。日志路径%s", szLoggerPath );
		return false;
	}

	SYS_INFO( "初始化文件日志模块成功。日志路径%s", szLoggerPath );

	return true;
	FUNCTION_END;
	return false;
}

///
/// \brief 清理化日志模块
/// \date 1/13/2014 
/// \author albert.xu
///
xgc_void FinializeLogger()
{
	fini_logger();
}
