#include "stdafx.h"
#include "ServerEventLog.h"
#include "MessageHandler.h"
#include "ServerLogger.h"
#include <shellapi.h>
#include <process.h>

#include "protoevent.h"
#pragma comment( lib, "shell32.lib" )

static xgc_bool		gEventLogLoop     = true;
static xgc_handle	gEventCheckThread = 0;
static ShmBuffer *	gEventShmBuffer   = xgc_nullptr;

xgc_bool isEvtServerScmStart()
{
	SC_HANDLE hScm = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hScm == NULL )
	{
		return false;
	}

	SC_HANDLE hEvt = OpenService( hScm, "EvtServer", SERVICE_ALL_ACCESS | DELETE );
	if( hEvt == NULL )
	{
		return false;
	}

	SERVICE_STATUS status;
	QueryServiceStatus( hEvt, &status );
	if( status.dwCurrentState != SERVICE_RUNNING && status.dwCurrentState != SERVICE_START_PENDING )
	{
		return false;
	}
	return true;
}

xgc_bool ScmStartEvtServer()
{
	SC_HANDLE hScm = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hScm == NULL )
	{
		return false;
	}
	SC_HANDLE hEvt = OpenService( hScm, "EvtServer", SERVICE_ALL_ACCESS | DELETE );
	if( hEvt == NULL )
	{
		return false;
	}
	SERVICE_STATUS status;
	QueryServiceStatus( hEvt, &status );
	if( status.dwCurrentState == SERVICE_RUNNING || status.dwCurrentState == SERVICE_START_PENDING )
	{
		return true;
	}
	else
	{
		return StartService( hEvt, 0, NULL ) == TRUE;
	}
	return false;
}

xgc_bool StartEvtServer( xgc_lpcstr szEventName, xgc_lpcstr szEventPath )
{
	STARTUPINFO stStartupInfo;

	memset( &stStartupInfo, 0, sizeof( stStartupInfo ) );
	stStartupInfo.lpDesktop = _T( "XcbDesk" );
	stStartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
	stStartupInfo.wShowWindow = SW_SHOWNORMAL;
	stStartupInfo.dwX = 0;
	stStartupInfo.dwY = 0;

	//DWORD dwCreationFlags = CREATE_NEW_CONSOLE;                

	SECURITY_ATTRIBUTES sa = { 0 };
	SECURITY_DESCRIPTOR sd = { 0 };
	InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
	SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );

	sa.nLength = sizeof( sa );
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = TRUE;

	HWND hEvtServerWnd = NULL;
	HINSTANCE ret = ShellExecuteA( hEvtServerWnd
		, "open"
		, szEventName
		, NULL
		, szEventPath
		, SW_SHOWNORMAL );

	if( (int) ret <= 32 )
	{
		DWORD errNo = GetLastError();
		USR_WARNING( "Loading %s failed. errCode = %d", szEventPath, errNo );
		return false;
	}
	return true;
}

///
/// 启动EventServer
/// [12/12/2014] create by albert.xu
///
static unsigned int __stdcall StartupEventLog( xgc_lpvoid params )
{
	xgc_char szEventPath[_MAX_PATH];
	xgc_char szEventName[_MAX_PATH] = "EvtServer.exe";

	GetNormalPath( szEventPath, sizeof( szEventPath ), "..\\EvtServer" );

	while( gEventLogLoop )
	{
		Sleep( 5 * 1000 );

		HWND hConsoleWnd = FindWindowA( NULL, szEventName );
		if( NULL != hConsoleWnd )
		{
			// 已经通过进程直接启动了
			// DBG_INFO( "EvtServer [%s] process exist", szEventName );
			continue;
		}
		else
		{
			// 没有窗口进程，检查是否有服务启动
			if( isEvtServerScmStart() )
			{
				continue;
			}

			if( IsServerService() )
			{
				if( !ScmStartEvtServer() )
				{
					StartEvtServer( szEventName, szEventPath );
					continue;
				}
			}
			else
			{
				StartEvtServer( szEventName, szEventPath );
				continue;
			}
		}
	}

	return 0;
}

xgc_bool InitializeEventLog( IniFile &ini )
{
	FUNCTION_BEGIN;
	if( ini.IsExistSection( "EventLog" ) )
	{
		xgc_lpcstr pName = ini.GetItemValue( "EventLog", "LogName", xgc_nullptr );
		xgc_uint32 nSize = ini.GetItemValue( "EventLog", "LogSize", 1024 * 1024 );

		if( xgc_nullptr == pName )
		{
			SYS_ERROR( "未能正确配置EventLog.LogName" );
			return false;
		}

		gEventShmBuffer = XGC::CreateShmBuffer( pName, nSize, GetLogPath() );

		if( xgc_nullptr == gEventShmBuffer )
		{
			return false;
		}

		gEventCheckThread = (xgc_handle) _beginthreadex( xgc_nullptr, 0, StartupEventLog, xgc_nullptr, 0, xgc_nullptr );
	}

	return true;
	FUNCTION_END;

	return false;
}

xgc_void FinializeEventLog()
{
	FUNCTION_BEGIN;
	gEventLogLoop = false;
	WaitForSingleObject( (HANDLE) gEventCheckThread, INFINITE );
	CloseHandle( (HANDLE) gEventCheckThread );

	if( gEventShmBuffer )
	{
		gEventShmBuffer->Destroy();
		SAFE_DELETE( gEventShmBuffer );
	}
	FUNCTION_END;
}

xgc_bool LogEvent( xgc_lpvoid lpData )
{
	FUNCTION_BEGIN;
	return gEventShmBuffer->Write( 
		xBuffer( lpData, 
			( (EvtHeader*) lpData )->length + sizeof( EvtHeader ) ) );

	FUNCTION_END;
	return false;
}

xgc_bool LogKeyFlow( xgc_lpcstr lpUserName, xgc_uint32 nType )
{
	FUNCTION_BEGIN;
	SYSTEMTIME now;
	GetSystemTime( &now );

	KeyFlowEvent evt;
	strcpy_s( evt.szPTID, lpUserName );
	evt.byHour = (UINT8) ( now.wHour + 8 ) % 24;
	evt.byMin = (UINT8) now.wMinute;
	evt.bySec = (UINT8) now.wSecond;
	evt.wMSec = now.wMilliseconds;
	evt.wType = nType;

	LogEvent( BuildEventLog( MsgKeyFlowEvent, evt ) );

	FUNCTION_END;
	return false;
}
