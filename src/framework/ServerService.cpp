#include "ServerDefines.h"
#include "ServerService.h"

/// @var 服务是否暂停
static xgc_uint32 g_nServiceStatus = SERVICE_STATUS_STOPPED;

/// 是否服务启动
static xgc_bool g_bService;

/// 
static xgc_char gServiceLog[XGC_MAX_PATH] = { 0 };
#define SVC_LOG( FMT, ... ) xgc::common::write_file( gServiceLog, FMT, __VA_ARGS__ )

#ifdef _WINDOWS

static SERVICE_STATUS_HANDLE gServiceHandle = NULL;
////////////////////////////////////////////////////////////////////////////////////////
xgc_bool IsInstalled( xgc_lpcstr lpServiceName )
{
	xgc_bool bResult = FALSE;

	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hSCM )
	{
		SC_HANDLE hService = ::OpenService( hSCM, lpServiceName, SERVICE_QUERY_CONFIG );
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
	SVC_LOG( "install params name=%s, display name=%s, description=%s, config=%s\n", 
			 lpServiceName,
			 lpServiceDisp,
			 lpServiceDesc,
			 lpConfigFile );

	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( !hSCM )
	{
		return -1;
	}

	char szFilePath[_MAX_PATH];
	::GetModuleFileName( NULL, szFilePath, sizeof( szFilePath ) );

	strcat_s( szFilePath, " -service " );
	strcat_s( szFilePath, lpConfigFile );

	SC_HANDLE hService = ::CreateService( hSCM,
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

	SC_ACTION _action[1];
	_action[0].Type = SC_ACTION_RESTART;
	_action[0].Delay = 10000;
	SERVICE_FAILURE_ACTIONS sfa;
	ZeroMemory(&sfa, sizeof(SERVICE_FAILURE_ACTIONS));
	sfa.lpsaActions = _action;
	sfa.cActions = 1;
	sfa.dwResetPeriod = INFINITE;
	ChangeServiceConfig2A(
		hService,                               // handle to service
		SERVICE_CONFIG_FAILURE_ACTIONS,         // information level
		&sfa);                                  // new data

	char szKey[256];
	HKEY hKey = NULL;
	strcpy_s( szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" );
	strcat_s( szKey, lpServiceName );
	if( ::RegCreateKey( HKEY_LOCAL_MACHINE, szKey, &hKey ) != ERROR_SUCCESS )
	{
		::CloseServiceHandle( hService );
		::CloseServiceHandle( hSCM );
		return -1;
	}

	::RegSetValueEx( hKey,
		"EventMessageFile",
		0,
		REG_EXPAND_SZ,
		( CONST BYTE* )szFilePath,
		(DWORD)strlen( szFilePath ) + 1 );

	DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
	::RegSetValueEx( hKey,
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
	SVC_LOG( "uninstall params name=%s\n", lpServiceName );

	SC_HANDLE hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( !hSCM )
		return FALSE;

	int nReturn = FALSE;
	SC_HANDLE hService = ::OpenService( hSCM, lpServiceName, DELETE );
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

	g_nServiceStatus = nState;
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

	hEventSource = RegisterEventSourceA( NULL, get_module_name() );
	if( NULL != hEventSource )
	{
		sprintf_s( szMsg, "%s", lpInfomation );
		lpStrings[0] = get_module_name();
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
		ReportServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 5 * 60 * 1000 );
		break;

		case SERVICE_CONTROL_PAUSE: // 2
		ReportServiceStatus( SERVICE_PAUSED, NO_ERROR, 0 );
		break;

		case SERVICE_CONTROL_CONTINUE: // 3
		ReportServiceStatus( SERVICE_RUNNING, NO_ERROR, 0 );
		break;

		case SERVICE_CONTROL_INTERROGATE: // 4
		break;

		case SERVICE_CONTROL_SHUTDOWN: // 5
		ReportServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 5 * 60 * 1000 );
		break;
	}
}

static int		gnArgc = 0;
static char **	gpArgv = xgc_nullptr;

///
/// 开启服务
/// [11/28/2014] create by albert.xu
///
static VOID WINAPI ServiceEntry( DWORD dwArgc, LPSTR lpArgv[] )
{
	gServiceHandle = RegisterServiceCtrlHandlerA( "", ServiceHandler );

	if( NULL == gServiceHandle )
	{
		ReportServiceEvent( EVENTLOG_INFORMATION_TYPE, SVC_ERROR, __FUNCTION__ );
		return;
	}

	ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 30000 );
	ServerMain( gnArgc, gpArgv );
}

xgc_void ServiceRun( int argc, char ** argv )
{
	gnArgc = argc - 1;
	gpArgv = argv + 1;

	auto pServiceLog = get_absolute_path( gServiceLog, "service.log" );
	if( nullptr == pServiceLog )
		return;

	xgc_char szDateTime[64] = { 0 };
	SVC_LOG( "service start at %s config file=%s\n",
			 datetime::now().to_string( szDateTime, sizeof(szDateTime) ),
			 argv[2] );

	g_bService = true;
	SERVICE_TABLE_ENTRY st[] =
	{
		{ "Service", ServiceEntry },
		{ NULL, NULL }
	};

	if( !::StartServiceCtrlDispatcher( st ) )
	{
		ReportServiceEvent( EVENTLOG_INFORMATION_TYPE, SVC_ERROR, __FUNCTION__ );
	}

	SVC_LOG( "service stop at %s\n", datetime::now().to_string( szDateTime, sizeof( szDateTime ) ) );
}

#endif

#ifdef _LINUX
#include <sys/stat.h>
///
/// 服务是否已安装
/// [11/28/2014] create by albert.xu
///
xgc_bool IsInstalled( xgc_lpcstr lpServiceName )
{
	return true;
}

///
/// 安装服务
/// [11/28/2014] create by albert.xu
///
int InstallService( xgc_lpcstr lpConfigFile, xgc_lpcstr lpServiceName, xgc_lpcstr lpServiceDisp, xgc_lpcstr lpServiceDesc )
{
	return 0;
}

///
/// 卸载服务
/// [11/28/2014] create by albert.xu
///
int UnInstallService( xgc_lpcstr lpServiceName )
{
	return 0;
}

///
/// 报告服务状态
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceStatus( xgc_uint32 nState, xgc_uint32 nExitCode, xgc_uint32 nWaitHint )
{
	g_nServiceStatus = nState;
}

///
/// 报告服务器事件
/// @param nEventType EVENTLOG_
/// @param nErrorCode ServerService.h defined.
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceEvent( xgc_uint16 nEventType, xgc_uint32 nErrorCode, xgc_lpcstr lpInfomation )
{
}


// signal handler  
void sigterm_handler(int arg)  
{  
	ReportServiceStatus( SERVICE_STATUS_STOP_PENDING, SERVICE_ERROR_NONE, 5 * 60 * 1000 );
}

xgc_void ServiceRun( int argc, char ** argv )
{
	pid_t pid;  
	// char *buf = "This is a Daemon, wcdj\n";

	/* 屏蔽一些有关控制终端操作的信号 
	* 防止在守护进程没有正常运转起来时，因控制终端受到干扰退出或挂起 
	* */  
	signal(SIGINT,  SIG_IGN);// 终端中断  
	signal(SIGHUP,  SIG_IGN);// 连接挂断  
	signal(SIGQUIT, SIG_IGN);// 终端退出  
	signal(SIGPIPE, SIG_IGN);// 向无读进程的管道写数据  
	signal(SIGTTOU, SIG_IGN);// 后台程序尝试写操作  
	signal(SIGTTIN, SIG_IGN);// 后台程序尝试读操作  
	signal(SIGTERM, SIG_IGN);// 终止  

	// test  
	//sleep(20);// try cmd: ./test &; kill -s SIGTERM PID  

	// [1] fork child process and exit father process  
	pid = fork();
	if(pid < 0)  
	{  
		perror("fork error!");  
		exit(1);  
	}
	else if(pid > 0)
	{
		exit(0);
	}

	// [2] create a new session  
	setsid();

	// [3] set current path  
	char szPath[1024];
	if(getcwd(szPath, sizeof(szPath)) == NULL)
	{  
		perror("getcwd");
		exit(1);
	}
	else
	{
		chdir(szPath);
		printf("set current path succ [%s]\n", szPath);
	}

	// [4] umask 0
	umask(0);

	/* [5] Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// [6] set termianl signal  
	signal(SIGTERM, sigterm_handler);

	ServerMain( argc, argv );
}
#endif

///
/// 检查服务器状态
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerStatus( xgc_uint32 nStatus )
{
	return g_nServiceStatus == nStatus;
}

///
/// 服务器是否已停止
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerStoped()
{
	return g_nServiceStatus == SERVICE_STATUS_STOPPED;
}

///
/// 服务器是否已暂停
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerPaused()
{
	return g_nServiceStatus == SERVICE_STATUS_PAUSED;
}

///
/// 是否通过服务启动
/// [12/5/2014] create by albert.xu
///
xgc_bool IsServerService()
{
	return g_bService;
}