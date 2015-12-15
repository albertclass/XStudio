// ServerBase.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ServerBase.h"

#include "ServerLogger.h"
#include "ServerConsole.h"
#include "ServerNetwork.h"
#include "ServerMonitor.h"
#include "ServerRefresh.h"
#include "protomtserver.h"
#include "ServerAsyncEvt.h"
#include "ServerSequence.h"

#include <process.h>

extern const char* __version__;
extern const char* __version_svn__;
extern const char* __version_url__;
extern const char* __version_build__;

///
/// 初始化数据库连接
/// [1/15/2014 albert.xu]
///
extern xgc_bool InitServerDatabase( IniFile &ini );

///
/// 清理数据库连接模块
/// [1/15/2014 albert.xu]
///
extern xgc_void FiniServerDatabase();

// 服务器配置
/// @var 服务器名
static xgc_char szServerName[32];
/// @var 服务器编号
static xgc_char szServerCode[32];
/// @var 服务器编号
static xgc_byte stServerCode[4];
/// @var 服务器配置文件路径
static xgc_char szConfigPath[_MAX_PATH];
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

xgc_bool InitServer( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( IniFile &ini, xgc_lpvoid ), xgc_lpvoid lpParam )
{
	FUNCTION_BEGIN;
	ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 5 * 60 * 1000 );

	xgc_lpcstr pInitNode = MemMark( "Initialize" );

	IniFile ini;
	if( lpConfigPath )
	{
		MemMark( "config", pInitNode );

		xgc_char szPath[_MAX_PATH] = { 0 };
		if( xgc_nullptr == GetNormalPath( szPath, sizeof( szPath ), "%s", lpConfigPath ) )
			return false;

		if( false == ini.Load( szPath ) )
			return false;

		// 服务器配置
		try
		{
			xgc_lpcstr lpValue = xgc_nullptr;
			lpValue = ini.GetItemValue( "ServerCfg", "ServerName", xgc_nullptr );
			if( xgc_nullptr == lpValue )
				throw std::runtime_error( "ServerCfg.ServerName read error." );

			strcpy_s( szServerName, lpValue );

			lpValue = ini.GetItemValue( "ServerCfg", "ServerCode", xgc_nullptr );
			if( xgc_nullptr == lpValue )
				throw std::runtime_error( "ServerCfg.ServerCode read error." );

			strcpy_s( szServerCode, lpValue );

			lpValue = ini.GetItemValue( "ServerCfg", "ConfigPath", xgc_nullptr );
			if( xgc_nullptr == lpValue )
				throw std::runtime_error( "ServerCfg.ConfigPath read error. need set config path!" );

			if( xgc_nullptr == GetNormalPath( szConfigPath, sizeof( szConfigPath ), "%s", lpValue ) )
				throw std::runtime_error( "ServerCfg.ConfigPath path string too long." );

			lpValue = ini.GetItemValue( "ServerCfg", "UseSequence", "false" );
			if( stricmp( lpValue, "true" ) != 0 )
			{
				bUseSequence = false;
				SYS_INFO( "不需要Sequence！" );
			}
			else
			{
				bUseSequence = true;
				SYS_INFO( "需要Sequence！" );
			}
		}
		catch( std::runtime_error e )
		{
			SYS_ERROR( "初始化配置文件失败 ConfigPath - %s, Error - %s", lpConfigPath, e.what() );
			return false;
		}
	}

	if( false == IsServerService() )
	{
		MemMark( "console", pInitNode );
		InitializeConsole( ini );
	}

	MemMark( "logger", pInitNode );
	if( false == InitializeLogger( ini ) )
		return false;

	SYS_INFO( "---------------------------------------------------" );
	SYS_INFO( "服务器正在启动..." );
	SYS_INFO( "服务器版本编号:%s", __version_svn__ );
	SYS_INFO( "服务器版本路径:%s", __version_url__ );
	SYS_INFO( "服务器编译日期:%s", __version_build__ );
	SYS_INFO( "日志系统初始化成功。服务器[%s ( %s )]正在启动……", GetServerName(), GetServerCode() );
	SYS_INFO( "服务器配置文件[%s]", lpConfigPath );
	SYS_INFO( "服务器配置路径[%s]", szConfigPath );
	SYS_INFO( "服务器进程ID[0x%0x:%u],线程ID[0x%x:%u]", getpid(), getpid(), _threadid, _threadid );

	if( false == InitException() )
		return false;

	MemMark( "MonitorBase", pInitNode );
	InitializeMonitor( szServerName, szServerCode );
	SYS_INFO( "MonitorBase 初始化成功" );

	MemMark( "database", pInitNode );
	if( false == InitServerDatabase( ini ) )
		return false;

	SYS_INFO( "数据库连接初始化成功！" );

	if( false == InitServerRefresh( ini ) )
		return false;

	SYS_INFO( "刷新系统初始化成功！" );

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

	// 初始化Monitor网络
	MemMark( "monitor", pInitNode );
	if( false == InitializeMTNetwork( ini ) )
		return false;
	SYS_INFO( "Monitor网络 初始化成功！" );

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
	if( false == InitializeNetwork( ini ) )
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
xgc_void RunServer( xgc_bool( *OnServerStep )( xgc_bool bBusy, xgc_lpvoid ), xgc_lpvoid lpParam )
{
	// 报告运行状态
	ReportServiceStatus( SERVICE_RUNNING, NO_ERROR, 0 );

	// 开启超时监控
	getInvokeWatcherMgr().Start();
	SYS_INFO( "服务器初始化成功" );

	xgc_bool bBusy = false;
	datetime nTime = datetime::current_time();

	while( false == IsServerStoped() )
	{
		if( IsServerPaused() )
		{
			Sleep( 1 );
			continue;
		}

		bBusy = false;
		if( false == IsServerService() )
		{
			if( -1 == ConsoleUpdate() )
				// exit server
				break;
		}

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

		if( datetime::current_time() - nTime >= timespan::from_minutes( 5 ) )
		{
			xgc_uint64 nMem = 0;
			xgc_uint64 nVMem = 0;
			GetMemoryStatus( &nMem, &nVMem );
			SYS_INFO( "Mem: %I64u  VMem : %I64u", nMem, nVMem );

			nTime = datetime::current_time();
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

	if( false == IsServerService() )
	{
		FinializeConsole();
		SYS_INFO( "控制台正常退出...\n///////////////////// - THE END - //////////////////////\n\n\n" );
	}

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

///
/// 获取服务器编号
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerCode()
{
	return szServerCode;
}

///
/// 拼凑服务器ID
/// [11/27/2014] create by albert.xu
///
xgc_uint32 GetServerCode( xgc_byte *byIdx, xgc_byte *byTyp, xgc_byte *byGrp, xgc_byte *byRgn )
{
	xgc_uint32 nPipeID = ServerString2Code( GetServerCode() );
	xgc_lpstr  pPipeID = (xgc_lpstr) &nPipeID;

	if( byRgn && *byRgn != 0 ) pPipeID[0] = *byRgn;
	if( byGrp && *byGrp != 0 ) pPipeID[1] = *byGrp;
	if( byTyp && *byTyp != 0 ) pPipeID[2] = *byTyp;
	if( byIdx && *byIdx != 0 ) pPipeID[3] = *byIdx;

	if( byRgn && *byRgn == 0 ) *byRgn = pPipeID[0];
	if( byGrp && *byGrp == 0 ) *byGrp = pPipeID[1];
	if( byTyp && *byTyp == 0 ) *byTyp = pPipeID[2];
	if( byIdx && *byIdx == 0 ) *byIdx = pPipeID[3];

	return nPipeID;
}

///
/// 转换服务器ID到字符串
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr ServerCode2String( xgc_uint32 nServerCode, xgc_lpstr lpBuffer, xgc_size nSize )
{
	if( nSize < 1 )
		return xgc_nullptr;

	xgc_byte *pCode = (xgc_byte*) &nServerCode;

	int cpy = _snprintf( lpBuffer, nSize, "%d-%d-%d-%d", pCode[0], pCode[1], pCode[2], pCode[3] );
	if( cpy <= 0 )
	{
		lpBuffer[0] = 0;
		return xgc_nullptr;
	}

	return lpBuffer;
}

///
/// 转换服务器ID为数值
/// [11/27/2014] create by albert.xu
///
xgc_uint32 ServerString2Code( xgc_lpcstr pServerCode )
{
	xgc_lpcstr p = pServerCode;
	xgc_uint32 s = 0;
	xgc_byte *c = (xgc_byte*) &s;
	for( int i = 0; i < sizeof( s ) && *p; ++i, ++p, ++c )
	{
		xgc_long v = strtoul( p, (char**) &p, 10 );
		XGC_ASSERT_MESSAGE( v != 0 && v < 256U, "服务器ID设置错误%s", pServerCode );
		*c = (xgc_byte) v;
	}

	return s;
}

///
/// 转换服务器ID为数值
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeType( xgc_uint32 nPipeID )
{
	return ( (xgc_lpcstr) &nPipeID )[2];
}

///
/// 转换服务器ID为数值
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeIndex( xgc_uint32 nPipeID )
{
	return ( (xgc_lpcstr) &nPipeID )[3];
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

xgc_void WriteToMonitor( xgc_lpcstr lpFmt, ... )
{
	FUNCTION_BEGIN;
	va_list args;
	va_start( args, lpFmt );

	char szMsg[1024] = { 0 };

	INT32 n1 = _snprintf( szMsg, sizeof( szMsg ), szServerCode );
	INT32 n2 = _vsnprintf( szMsg + n1, sizeof(szMsg) -n1 - 2, lpFmt, args );
	szMsg[n2 + n1] = 0;

	va_end( args );

	MT_CONSOLE_OTHER_PKG_NTF stNtf = { 0 };
	strcpy_s( stNtf.szmsg, szMsg );

	auto pSession = GetPipeSession( ServerType::SD_MONITOR_SERVER );
	if( pSession )
		SendPackage( pSession, BuildPackage( MT_CONSOLE_OTHER_ID_NTF, stNtf ) );
	FUNCTION_END;
}

xgc_uint64 GetSequenceID()
{
	FUNCTION_BEGIN;
	return ServerSequence::GetSID();
	FUNCTION_END;
	return 0;
}