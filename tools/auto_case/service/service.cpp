///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file Service.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// windows service 封装
///
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <io.h>
#include "Service.h"

xgc_char gServiceLog[_MAX_PATH] = { 0 };
#define SVC_LOG( FMT, ... ) logger::write_file( gServiceLog, FMT, __VA_ARGS__ )
static SERVICE_STATUS_HANDLE gServiceHandle = NULL;
/// @var 服务是否暂停
static xgc_bool g_bPaused = false;
/// @var 服务是否停止
static xgc_bool g_bStoped = false;
/// @var 是否服务启动
static xgc_bool g_bService = false;
////////////////////////////////////////////////////////////////////////////////////////
xgc_bool IsInstalled( xgc_lpcstr lpServiceName )
{
	xgc_bool bResult = FALSE;

	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hSCM )
	{
		SC_HANDLE hService = ::OpenServiceA( hSCM, lpServiceName, SERVICE_QUERY_CONFIG );
		if( hService )
		{
			bResult = TRUE;
			::CloseServiceHandle( hService );
		}

		::CloseServiceHandle( hSCM );
	}

	return bResult;
}

///
/// 安装服务
/// [11/28/2014] create by albert.xu
///
int InstallService( xgc_lpcstr lpConfigFile, xgc_lpcstr lpServiceName, xgc_lpcstr lpServiceDisp, xgc_lpcstr lpServiceDesc )
{
	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( !hSCM )
	{
		return -1;
	}

	char szFilePath[_MAX_PATH];
	::GetModuleFileNameA( NULL, szFilePath, sizeof( szFilePath ) );

	strcat_s( szFilePath, " -service " );
	strcat_s( szFilePath, lpConfigFile );

	SC_HANDLE hService = ::CreateServiceA( hSCM,
		lpServiceName,
		lpServiceDisp,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		szFilePath,
		NULL, NULL, NULL, NULL, NULL );

	if( !hService )
	{
		::CloseServiceHandle( hSCM );
		printf( "%s", "服务已经安装或者有同名服务。\n" );
		return -1;
	}

	SERVICE_DESCRIPTIONA Description;
	xgc_char szDescription[1024];
	strcpy_s( szDescription, lpServiceDesc );
	Description.lpDescription = szDescription;
	ChangeServiceConfig2A( hService, SERVICE_CONFIG_DESCRIPTION, &Description );

	char szKey[256];
	HKEY hKey = NULL;
	strcpy_s( szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" );
	strcat_s( szKey, lpServiceName );
	if( ::RegCreateKeyA( HKEY_LOCAL_MACHINE, szKey, &hKey ) != ERROR_SUCCESS )
	{
		::CloseServiceHandle( hService );
		::CloseServiceHandle( hSCM );
		return -1;
	}

	::RegSetValueExA( hKey,
		"EventMessageFile",
		0,
		REG_EXPAND_SZ,
		( CONST BYTE* )szFilePath,
		(DWORD)strlen( szFilePath ) + 1 );

	DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
	::RegSetValueExA( hKey,
		"TypesSupported",
		0,
		REG_DWORD,
		( CONST BYTE* )&dwData,
		sizeof( DWORD ) );
	::RegCloseKey( hKey );

	::CloseServiceHandle( hService );
	::CloseServiceHandle( hSCM );

	return 0;
}

///
/// 卸载服务
/// [11/28/2014] create by albert.xu
///
int UnInstallService( xgc_lpcstr lpServiceName )
{
	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( !hSCM )
		return FALSE;

	int nReturn = FALSE;
	SC_HANDLE hService = ::OpenServiceA( hSCM, lpServiceName, DELETE );
	if( hService )
	{
		nReturn = ::DeleteService( hService ) ? 0 : -1;

		::CloseServiceHandle( hService );
		SVC_LOG( "删除服务%s成功.\n", lpServiceName );
	}
	else
	{
		SVC_LOG( "删除服务%s失败.\n", lpServiceName );
	}

	::CloseServiceHandle( hSCM );
	return nReturn;
}

///
/// 报告服务状态
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceStatus( xgc_uint32 nState, xgc_uint32 nExitCode, xgc_uint32 nWaitHint )
{
	static volatile unsigned long long dwCheckPoint = 0;

	SERVICE_STATUS status;
	status.dwServiceType   = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState  = nState;
	status.dwWin32ExitCode = nExitCode;
	status.dwWaitHint      = nWaitHint;

	if( nState == SERVICE_START_PENDING )
		status.dwControlsAccepted = 0;
	else
		status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;

	if( nState == SERVICE_RUNNING || nState == SERVICE_STOPPED )
		status.dwCheckPoint = 0;
	else
		status.dwCheckPoint = ( DWORD )InterlockedIncrement( &dwCheckPoint );

	BOOL bRet = SetServiceStatus( gServiceHandle, &status );
	if( FALSE == bRet )
	{
		HRESULT hreult = (HRESULT) GetLastError();
		void* pMsgBuf;
		::FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			hreult,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			(LPSTR) &pMsgBuf,
			0,
			NULL
			);

		SVC_LOG( "SetServiceStatus invoke error. \"%s\"", (LPSTR)pMsgBuf );
		LocalFree( pMsgBuf );
	}
}

///
/// 报告服务器事件
/// @param nEventType EVENTLOG_
/// @param nErrorCode ServerService.h defined.
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceEvent( xgc_uint16 nEventType, xgc_uint32 nErrorCode, xgc_lpcstr lpInfomation )
{
	HANDLE hEventSource = INVALID_HANDLE_VALUE;
	xgc_lpcstr lpStrings[2] = { xgc_nullptr, xgc_nullptr };
	xgc_char szMsg[256] = { 0 };

	hEventSource = RegisterEventSourceA( NULL, GetModuleName() );
	if( NULL != hEventSource )
	{
		sprintf_s( szMsg, "%s", lpInfomation );
		lpStrings[0] = GetModuleName();
		lpStrings[1] = szMsg;

		ReportEventA( hEventSource,
			nEventType,
			0,
			nErrorCode,
			NULL,
			2,
			0,
			lpStrings,
			NULL );

		DeregisterEventSource( hEventSource );
	}
}

VOID WINAPI ServiceHandler( DWORD dwOpcode )
{
	switch( dwOpcode )
	{
		case SERVICE_CONTROL_STOP: // 1
		g_bStoped = true;
		ReportServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 5 * 60 * 1000 );
		break;

		case SERVICE_CONTROL_PAUSE: // 2
		g_bPaused = true;
		ReportServiceStatus( SERVICE_PAUSED, NO_ERROR, 0 );
		break;

		case SERVICE_CONTROL_CONTINUE: // 3
		g_bPaused = false;
		ReportServiceStatus( SERVICE_RUNNING, NO_ERROR, 0 );
		break;

		case SERVICE_CONTROL_INTERROGATE: // 4
		break;

		case SERVICE_CONTROL_SHUTDOWN: // 5
		g_bStoped = true;
		ReportServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 5 * 60 * 1000 );
		break;

		default:
		break;
	}
}

static int		gnArgc = 1;
static char **	gpArgv = xgc_nullptr;

///
/// 留给服务器实现的入口函数
/// [11/28/2014] create by albert.xu
///
extern int ServiceMain( int argc, char *argv[] );

///
/// 开启服务
/// [11/28/2014] create by albert.xu
///
VOID WINAPI ServiceRun( DWORD dwArgc, LPSTR lpArgv[] )
{

	gServiceHandle = RegisterServiceCtrlHandlerA( "", ServiceHandler );

	if( NULL == gServiceHandle )
	{
		ReportServiceEvent( EVENTLOG_INFORMATION_TYPE, SVC_ERROR, __FUNCTION__ );
	}
	else
	{
		ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 30000 );
		ServiceMain( gnArgc, gpArgv );
	}
}

///
/// 服务器是否已停止
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServiceStoped()
{
	return g_bStoped;
}

///
/// 服务器是否已暂停
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServicePaused()
{
	return g_bPaused;
}

///
/// 是否通过服务启动
/// [12/5/2014] create by albert.xu
///
xgc_bool IsService()
{
	return g_bService;
}

///
/// InvalidParameterHandler
/// 异常参数调用时的处理函数，由此函数则不调用dr.waston！
/// \author xufeng04
/// \date [6/6/2014]
///
void InvalidParameterHandler( const wchar_t* expr
	, const wchar_t* func
	, const wchar_t* file
	, unsigned int   line
	, uintptr_t      resv )
{
	if( func && file && expr )
	{
		SYS_ERROR(
			"Invalid parameter detected in: Function:%S File:%S Line:%d Expression:%S"
			, func
			, file
			, line
			, expr );
	}

	DumpStackFrame();
	XGC_DEBUG_MESSAGE( "非法的参数调用，请检查调用项并排除BUG。" );
	// _invoke_watson( expr, func, file, line, resv );
	throw std::runtime_error( "invalid parameter detected" );
}

///
/// 虚调用处理函数
/// \author xufeng04
/// \date [6/6/2014]
///
void __cdecl PureCallHandler()
{
	DumpStackFrame();
	XGC_DEBUG_MESSAGE( "PURECALL" );
	SYS_ERROR( "PURECALL" );
	throw std::exception( "pure call" );
}

///
/// 主函数入口
/// [11/28/2014] create by albert.xu
///
int main( int argc, char *argv[] )
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	_set_invalid_parameter_handler( InvalidParameterHandler );
	_set_purecall_handler( PureCallHandler );

	setlocale( LC_ALL, "chs" );

	GetNormalPath( gServiceLog, sizeof( gServiceLog ), "%s", "service.log" );

	if( argc > 1 )
	{
		if( _stricmp( argv[1], "-install" ) == 0 )
		{
			SVC_LOG( "%s", "install service.\n" );
			xgc_lpcstr lpServiceName = xgc_nullptr;
			xgc_lpcstr lpServiceDisp = xgc_nullptr;
			xgc_lpcstr lpServiceDesc = xgc_nullptr;

			xgc_lpcstr lpConfigFile = "Config.ini";
			if( argc > 2 )
				lpConfigFile = argv[2];

			char szConfPath[_MAX_PATH];
			GetNormalPath( szConfPath, sizeof( szConfPath ), "%s", lpConfigFile );
			ini_reader ini;
			if( false == ini.load( szConfPath ) )
			{
				printf( "read config file %s failed. please check and try again.", szConfPath );
				return -1;
			}

			lpServiceName = ini.get_item_value( "ServiceCfg", "ServiceName", xgc_nullptr );
			if( xgc_nullptr == lpServiceName )
			{
				printf( "section 'ServiceCfg' key 'ServiceName' is empty. it must configuration" );
				return -1;
			}

			xgc_char szServiceDisp[256];
			xgc_char szServiceDesc[256];

			lpServiceDisp = ini.get_item_value( "ServiceCfg", "DisplayName", lpServiceName );
			lpServiceDesc = ini.get_item_value( "ServiceCfg", "Description", "这个人很懒注册服务的时候都不写服务描述。" );

			SVC_LOG( "install params name=%s, display name=%s, description=%s, config=%s\n", 
				lpServiceName,
				lpServiceDisp,
				lpServiceDesc,
				lpConfigFile );

			return InstallService( lpConfigFile, lpServiceName, szServiceDisp, szServiceDesc );
		}
		else if( _stricmp( argv[1], "-uninstall" ) == 0 )
		{
			xgc_lpcstr lpServiceName = xgc_nullptr;

			if( argc > 2 )
				lpServiceName = argv[2];
			else
				lpServiceName = GetModuleName();

			SVC_LOG( "uninstall params name=%s\n", lpServiceName );

			return UnInstallService( lpServiceName );
		}
		else if( _stricmp( argv[1], "-service" ) == 0 )
		{
			gnArgc = argc - 1;
			gpArgv = argv + 1;

			xgc_char szDateTime[64] = { 0 };
			SVC_LOG( "service start at %s config file=%s\n",
				datetime::now().to_string( szDateTime, sizeof(szDateTime) ),
				argv[2] );

			g_bService = true;
			SERVICE_TABLE_ENTRYA st[] =
			{
				{ "Service", ServiceRun },
				{ NULL, NULL }
			};

			if( !::StartServiceCtrlDispatcherA( st ) )
			{
				ReportServiceEvent( EVENTLOG_INFORMATION_TYPE, SVC_ERROR, __FUNCTION__ );
			}

			SVC_LOG( "service stop at %s\n", datetime::now().to_string( szDateTime, sizeof( szDateTime ) ) );
			return 0;
		}
		else if( _stricmp( argv[1], "-debug" ) == 0 )
		{
			char path[_MAX_PATH] = { 0 };
			GetNormalPath( path, sizeof( path ), "config*.ini" );

			char choice = 0;
			int n = 0;
			_finddata_t stat[8];

			do
			{
				memset( stat, 0, sizeof( stat ) );

				intptr_t fd = _findfirst( path, &stat[0] );
				if( fd == -1 )
				{
					puts( "search path error!" );
					return 0;
				}

				puts( "debug mode" );
				puts( "choice your config file:" );
				n = 0;
				do
				{
					printf( "%d. %s\n", n, stat[n].name );
				} while( n < _countof( stat ) && _findnext( fd, &stat[++n] ) == 0 );


				printf( "q. exit\n" );
				printf( "press menu number to continue ..." );
				_findclose( fd );
				if( ( choice = _getch() ) == 'q' )
					return 1;

			} while( false == isdigit( choice ) || ( choice - '0' >= n ) );

			argv[1] = stat[choice - '0'].name;
			return ServiceMain( argc, argv );
		}
	}

	return ServiceMain( argc, argv );
}
