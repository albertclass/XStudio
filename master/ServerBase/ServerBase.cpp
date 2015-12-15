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
/// ��ʼ�����ݿ�����
/// [1/15/2014 albert.xu]
///
extern xgc_bool InitServerDatabase( IniFile &ini );

///
/// �������ݿ�����ģ��
/// [1/15/2014 albert.xu]
///
extern xgc_void FiniServerDatabase();

// ����������
/// @var ��������
static xgc_char szServerName[32];
/// @var ���������
static xgc_char szServerCode[32];
/// @var ���������
static xgc_byte stServerCode[4];
/// @var �����������ļ�·��
static xgc_char szConfigPath[_MAX_PATH];
/// @var �Ƿ���Ҫ��ʼ��Sequence
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

		// ����������
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
				SYS_INFO( "����ҪSequence��" );
			}
			else
			{
				bUseSequence = true;
				SYS_INFO( "��ҪSequence��" );
			}
		}
		catch( std::runtime_error e )
		{
			SYS_ERROR( "��ʼ�������ļ�ʧ�� ConfigPath - %s, Error - %s", lpConfigPath, e.what() );
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
	SYS_INFO( "��������������..." );
	SYS_INFO( "�������汾���:%s", __version_svn__ );
	SYS_INFO( "�������汾·��:%s", __version_url__ );
	SYS_INFO( "��������������:%s", __version_build__ );
	SYS_INFO( "��־ϵͳ��ʼ���ɹ���������[%s ( %s )]������������", GetServerName(), GetServerCode() );
	SYS_INFO( "�����������ļ�[%s]", lpConfigPath );
	SYS_INFO( "����������·��[%s]", szConfigPath );
	SYS_INFO( "����������ID[0x%0x:%u],�߳�ID[0x%x:%u]", getpid(), getpid(), _threadid, _threadid );

	if( false == InitException() )
		return false;

	MemMark( "MonitorBase", pInitNode );
	InitializeMonitor( szServerName, szServerCode );
	SYS_INFO( "MonitorBase ��ʼ���ɹ�" );

	MemMark( "database", pInitNode );
	if( false == InitServerDatabase( ini ) )
		return false;

	SYS_INFO( "���ݿ����ӳ�ʼ���ɹ���" );

	if( false == InitServerRefresh( ini ) )
		return false;

	SYS_INFO( "ˢ��ϵͳ��ʼ���ɹ���" );

	if( false == InitServerAsyncEvt() )
		return false;
	SYS_INFO( "�첽�߼������ʼ���ɹ���" );

	MemMark( "debugcommand", pInitNode );
	if( false == InitDebugCmd( ini ) )
		return false;

	SYS_INFO( "����ָ��ϵͳ��ʼ���ɹ���" );

	MemMark( "eventlogger", pInitNode );
	if( false == InitializeEventLog( ini ) )
		return false;

	SYS_INFO( "�¼���־ϵͳ��ʼ���ɹ���" );

	// ��ʼ��Monitor����
	MemMark( "monitor", pInitNode );
	if( false == InitializeMTNetwork( ini ) )
		return false;
	SYS_INFO( "Monitor���� ��ʼ���ɹ���" );

	if( bUseSequence )
	{
		MemMark( "sequence", pInitNode );
		if( false == ServerSequence::InitServerSequence() )
			return false;

		SYS_INFO( "Sequence ��ʼ���ɹ���" );
	}

	ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 5 * 60 * 1000 );
	MemMark( "configuration", pInitNode );
	if( false == InitConfiguration( ini, lpParam ) )
	{
		USR_ERROR( "InitConfiguration failed." );
		return false;
	}
	SYS_INFO( "�������ݳ�ʼ���ɹ���" );

	MemMark( "network", pInitNode );
	if( false == InitializeNetwork( ini ) )
		return false;

	SYS_INFO( "�����ʼ���ɹ���" );

	MemMarkReport( xgc_nullptr, PrintMemReport );
	MemMarkClear();

	return true;
	FUNCTION_END;
	return false;
}

///
/// ���з�����
/// [11/29/2014] create by albert.xu
///
xgc_void RunServer( xgc_bool( *OnServerStep )( xgc_bool bBusy, xgc_lpvoid ), xgc_lpvoid lpParam )
{
	// ��������״̬
	ReportServiceStatus( SERVICE_RUNNING, NO_ERROR, 0 );

	// ������ʱ���
	getInvokeWatcherMgr().Start();
	SYS_INFO( "��������ʼ���ɹ�" );

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

		// ������������
		bBusy = ProcessNetwork() || bBusy;

		// �������ݿ����
		bBusy = AsyncDBExecuteResp( 50 ) > 0 || bBusy;

		if( OnServerStep( bBusy, lpParam ) )
			break;

		StepServerRefresh();

		// ������Ҫ���첽�߼�����
		StepServerEvent();

		// �Ƿ���У����е�ʱ��Ҫ�������������
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

	SYS_INFO( "�رճ�ʱ���..." );
	getInvokeWatcherMgr().Stop();
}

xgc_void FiniServer( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam )
{
	FUNCTION_BEGIN;
	ReportServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 5 * 60 * 1000 );
	FinializeNetwork();
	SYS_INFO( "������������..." );

	FinializeEventLog();
	SYS_INFO( "Evt�������..." );

	FiniConfiguration( lpParam );
	SYS_INFO( "�������������..." );

	FiniDebugCmd();
	SYS_INFO( "����ָ��ϵͳ�������..." );

	FiniServerRefresh();
	SYS_INFO( "ˢ��ϵͳ�������..." );

	FiniServerAsyncEvt();
	SYS_INFO( "�첽�߼������������..." );

	FiniServerDatabase();
	SYS_INFO( "���ݿ��������..." );

	FiniException();
	FinializeLogger();

	if( false == IsServerService() )
	{
		FinializeConsole();
		SYS_INFO( "����̨�����˳�...\n///////////////////// - THE END - //////////////////////\n\n\n" );
	}

	MemMarkClear();
	ReportServiceStatus( SERVICE_STOPPED, NO_ERROR, 0 );
	FUNCTION_END;
}

///
/// ��ȡ��������
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerName()
{
	return szServerName;
}

///
/// ��ȡ���������
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerCode()
{
	return szServerCode;
}

///
/// ƴ�շ�����ID
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
/// ת��������ID���ַ���
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
/// ת��������IDΪ��ֵ
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
		XGC_ASSERT_MESSAGE( v != 0 && v < 256U, "������ID���ô���%s", pServerCode );
		*c = (xgc_byte) v;
	}

	return s;
}

///
/// ת��������IDΪ��ֵ
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeType( xgc_uint32 nPipeID )
{
	return ( (xgc_lpcstr) &nPipeID )[2];
}

///
/// ת��������IDΪ��ֵ
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