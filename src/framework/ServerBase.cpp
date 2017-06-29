// ServerBase.cpp : Defines the exported functions for the DLL application.
//

#include "ServerDefines.h"
#include "ServerBase.h"
#include "ServerDatabase.h"
#include "ServerLogger.h"
#include "ServerRefresh.h"
#include "ServerAsyncEvt.h"
#include "ServerSequence.h"

const char* __version__ = "1.0.0.0";
const char* __version_svn__ = "37687";
const char* __version_url__ = "github.com/albertclass/XStudio";
const char* __version_build__ = "37686";

///
/// 初始化数据库连接
/// [1/15/2014 albert.xu]
///
extern xgc_bool InitServerDatabase( ini_reader &ini );

///
/// 清理数据库连接模块
/// [1/15/2014 albert.xu]
///
extern xgc_void FiniServerDatabase();

// 服务器配置
/// @var 服务器名
static xgc_char szServerName[32];
/// @var 服务器配置文件路径
static xgc_char szConfigPath[XGC_MAX_PATH];
/// @var 是否需要初始化Sequence
static xgc_bool bUseSequence;

int __cdecl PrintMemReport( xgc_lpcstr fmt, ... )
{
	char szMsg[1024];
	va_list args;
	va_start( args, fmt );
	int cpy = vsprintf_s( szMsg, sizeof( szMsg ), fmt, args );
	va_end( args );

	SYS_INFO( "%s", szMsg );
	return cpy;
}

xgc_bool InitServer( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )(xgc::common::ini_reader &, xgc_lpvoid), xgc_lpvoid lpParam )
{
	FUNCTION_BEGIN;
	ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 5 * 60 * 1000 );

	xgc_lpcstr pInitNode = MemMark( "Initialize" );

	xgc_char szPath[XGC_MAX_PATH] = { 0 };

	ini_reader ini;
	if( lpConfigPath )
	{
		MemMark( "config", pInitNode );

		if( xgc_nullptr == get_absolute_path( szPath, sizeof( szPath ), "%s", lpConfigPath ) )
			return false;

		if( false == ini.load( szPath ) )
			return false;

		// 服务器配置
		try
		{
			xgc_lpcstr lpValue = xgc_nullptr;
			lpValue = ini.get_item_value( "ServerCfg", "ServerName", xgc_nullptr );
			if( xgc_nullptr == lpValue )
				throw std::runtime_error( "ServerCfg.ServerName read error." );

			strcpy_s( szServerName, lpValue );

			lpValue = ini.get_item_value( "ServerCfg", "ConfigPath", xgc_nullptr );
			if( xgc_nullptr == lpValue )
				throw std::runtime_error( "ServerCfg.ConfigPath read error. need set config path!" );

			if( xgc_nullptr == get_absolute_path( szConfigPath, sizeof( szConfigPath ), "%s", lpValue ) )
				throw std::runtime_error( "ServerCfg.ConfigPath path string too long." );

			bUseSequence = ini.get_item_value( "ServerCfg", "UseSequence", false );
		}
		catch( std::runtime_error e )
		{
			SYS_ERROR( "初始化配置文件失败 ConfigPath - %s, Error - %s", lpConfigPath, e.what() );
			return false;
		}
	}

	MemMark( "logger", pInitNode );
	if( false == InitializeLogger( ini ) )
		return false;

	SYS_INFO( "---------------------------------------------------" );
	SYS_INFO( "服务器正在启动..." );
	SYS_INFO( "服务器版本编号:%s", __version_svn__ );
	SYS_INFO( "服务器版本路径:%s", __version_url__ );
	SYS_INFO( "服务器编译日期:%s", __version_build__ );
	SYS_INFO( "日志系统初始化成功。服务器[%s]正在启动……", GetServerName() );
	SYS_INFO( "服务器配置文件[%s]", lpConfigPath );
	SYS_INFO( "服务器配置路径[%s]", szConfigPath );
	SYS_INFO( "服务器进程ID[0x%0x:%u],线程ID[0x%x:%u]", get_process_id(), get_process_id(), get_thread_id(), get_thread_id() );

	if( false == InitException() )
		return false;

	MemMark( "database", pInitNode );
	if( false == InitServerDatabase( ini ) )
		return false;

	SYS_INFO( "数据库连接初始化成功！" );

	MemMark( "refresh", pInitNode );
	if( false == InitServerRefresh( ini ) )
		return false;

	SYS_INFO( "刷新系统初始化成功！" );

	MemMark( "asyncevent", pInitNode );
	if( false == InitServerAsyncEvt() )
		return false;

	SYS_INFO( "异步逻辑处理初始化成功！" );

	MemMark( "debugcommand", pInitNode );
	if( false == InitDebugCmd( ini ) )
		return false;

	SYS_INFO( "调试指令系统初始化成功！" );

	MemMark( "eventlogger", pInitNode );
	if( false == InitializeEventLog( ini ) )
		return false;

	SYS_INFO( "事件日志系统初始化成功！" );

	if( bUseSequence )
	{
		MemMark( "sequence", pInitNode );
		if( false == ServerSequence::InitServerSequence() )
			return false;

		SYS_INFO( "Sequence 初始化成功！" );
	}

	ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 5 * 60 * 1000 );
	MemMark( "configuration", pInitNode );
	if( false == InitConfiguration( ini, lpParam ) )
	{
		USR_ERROR( "InitConfiguration failed." );
		return false;
	}
	SYS_INFO( "配置数据初始化成功！" );

	MemMark( "network", pInitNode );
	if( false == InitializeNetwork( szPath ) )
		return false;

	SYS_INFO( "网络初始化成功！" );

	MemMarkReport( xgc_nullptr, PrintMemReport );
	MemMarkClear();

	return true;
	FUNCTION_END;
	return false;
}

///
/// 运行服务器
/// [11/29/2014] create by albert.xu
///
xgc_void RunServer( xgc_bool( *OnServerStep )( xgc_bool, xgc_lpvoid ), xgc_lpvoid lpParam )
{
	// 报告运行状态
	ReportServiceStatus( SERVICE_RUNNING, NO_ERROR, 0 );

	// 开启超时监控
	getInvokeWatcherMgr().Start();
	SYS_INFO( "服务器初始化成功" );

	xgc_bool bBusy = false;
	datetime tLast = datetime::now();

	while( false == IsServerStoped() )
	{
		if( IsServerPaused() )
		{
			Sleep( 1 );
			continue;
		}

		bBusy = false;

		// 处理网络数据
		bBusy = ProcessNetwork() || bBusy;

		// 处理数据库操作
		bBusy = AsyncDBExecuteResp( 50 ) > 0 || bBusy;

		if( OnServerStep( bBusy, lpParam ) )
			break;

		StepServerRefresh();

		// 处理需要的异步逻辑操作
		StepServerEvent();

		// 是否空闲，空闲的时候要主动挂起服务器
		if( bBusy == false )
			Sleep( 1 );

		if( datetime::now() - tLast >= timespan::from_minutes( 5 ) )
		{
			xgc_uint64 nMem = 0;
			xgc_uint64 nVMem = 0;
			get_process_memory_usage( &nMem, &nVMem );
			SYS_INFO( "Mem: %I64u  VMem : %I64u", nMem, nVMem );

			tLast = datetime::now();
		}
	}

	SYS_INFO( "关闭超时检测..." );
	getInvokeWatcherMgr().Stop();
}

xgc_void FiniServer( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam )
{
	FUNCTION_BEGIN;
	ReportServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 5 * 60 * 1000 );
	FinializeNetwork();
	SYS_INFO( "网络库清理完成..." );

	FinializeEventLog();
	SYS_INFO( "Evt清理完成..." );

	FiniConfiguration( lpParam );
	SYS_INFO( "配置项清理完成..." );

	FiniDebugCmd();
	SYS_INFO( "调试指令系统清理完成..." );

	FiniServerRefresh();
	SYS_INFO( "刷新系统清理完成..." );

	FiniServerAsyncEvt();
	SYS_INFO( "异步逻辑处理清理完成..." );

	FiniServerDatabase();
	SYS_INFO( "数据库清理完成..." );

	FiniException();
	FinializeLogger();

	MemMarkClear();
	ReportServiceStatus( SERVICE_STOPPED, NO_ERROR, 0 );
	FUNCTION_END;
}

///
/// 获取服务器名
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerName()
{
	return szServerName;
}

xgc_lpcstr GetConfPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... )
{
	int cpy1 = sprintf_s( szPath, nSize, "%s", szConfigPath );
	XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );

	va_list args;
	va_start( args, lpRelativePath );
	int cpy2 = vsprintf_s( szPath + cpy1, nSize - cpy1, lpRelativePath, args );
	va_end( args );

	if( cpy2 < 0 )
		return xgc_nullptr;

	if( cpy2 == sizeof(szPath) -cpy1 )
	{
		return xgc_nullptr;
	}

	return _fullpath( szPath, szPath, nSize );
}

xgc_string GetConfPath( xgc_lpcstr lpRelativePath, ... )
{
	xgc_char szPath[_MAX_PATH] = { 0 };

	auto nSize = sizeof( szPath );

	int cpy1 = sprintf_s( szPath, nSize, "%s", szConfigPath );
	XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );

	va_list args;
	va_start( args, lpRelativePath );
	int cpy2 = vsprintf_s( szPath + cpy1, nSize - cpy1, lpRelativePath, args );
	va_end( args );

	if( cpy2 < 0 )
		return xgc_nullptr;

	if( cpy2 == sizeof(szPath) -cpy1 )
	{
		return xgc_nullptr;
	}

	_fullpath( szPath, szPath, nSize );

	return szPath;
}

xgc_string LuaGetConfPath( xgc_lpcstr lpRelativePath )
{
	xgc_char szPath[_MAX_PATH] = { 0 };

	auto nSize = sizeof( szPath );

	int cpy1 = sprintf_s( szPath, nSize, "%s", szConfigPath );
	XGC_ASSERT_RETURN( cpy1 >= 0, xgc_nullptr );

	int cpy2 = sprintf_s( szPath + cpy1, nSize - cpy1, "%s", lpRelativePath );

	if( cpy2 < 0 )
		return xgc_nullptr;

	if( cpy2 == sizeof( szPath ) - cpy1 )
	{
		return xgc_nullptr;
	}

	_fullpath( szPath, szPath, nSize );

	return szPath;
}

xgc_uint64 GetSequenceID()
{
	FUNCTION_BEGIN;
	return ServerSequence::GetSID();
	FUNCTION_END;
	return 0;
}
