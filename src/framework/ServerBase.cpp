// ServerBase.cpp : Defines the exported functions for the DLL application.
//
#include "ServerDefines.h"
#include "ServerBase.h"

const char* __version__ = "1.0.0.0";
const char* __version_svn__ = "37687";
const char* __version_url__ = "github.com/albertclass/XStudio";
const char* __version_build__ = "37686";

///
/// ��ʼ�����ݿ�����
/// [1/15/2014 albert.xu]
///
extern xgc_bool InitServerDatabase( ini_reader &ini );

///
/// �������ݿ�����ģ��
/// [1/15/2014 albert.xu]
///
extern xgc_void FiniServerDatabase();

// ����������
/// @var ��������
static xgc_char szServerName[32];
/// @var �����������ļ�·��
static xgc_char szConfigPath[XGC_MAX_PATH];
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

xgc_bool ServerInit( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )(xgc::common::ini_reader &, xgc_lpvoid), xgc_lpvoid lpParam )
{
	FUNCTION_BEGIN;
	ReportServiceStatus( SERVICE_STATUS_START_PENDING, SERVICE_ERROR_NONE, 5 * 60 * 1000 );
	XGC_ASSERT_RETURN( lpConfigPath, false );

	xgc_lpcstr pInitNode = MemMark( "Initialize" );

	xgc_char szPath[XGC_MAX_PATH] = { 0 };

	ini_reader ini;

	MemMark( "config", pInitNode );

	if( xgc_nullptr == get_absolute_path( szPath, sizeof( szPath ), "%s", lpConfigPath ) )
		return false;

	if( false == ini.load( szPath ) )
		return false;

	// ����������
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
		SYS_ERROR( "��ʼ�������ļ�ʧ�� ConfigPath - %s, Error - %s", lpConfigPath, e.what() );
		return false;
	}

	MemMark( "logger", pInitNode );
	if( false == InitializeLogger( ini ) )
		return false;

	SYS_INFO( "---------------------------------------------------" );
	SYS_INFO( "��������������..." );
	SYS_INFO( "�������汾���:%s", __version_svn__ );
	SYS_INFO( "�������汾·��:%s", __version_url__ );
	SYS_INFO( "��������������:%s", __version_build__ );
	SYS_INFO( "��־ϵͳ��ʼ���ɹ���������[%s]������������", ServerName() );
	SYS_INFO( "�����������ļ�[%s]", lpConfigPath );
	SYS_INFO( "����������·��[%s]", szConfigPath );
	SYS_INFO( "����������ID[0x%0x:%u],�߳�ID[0x%x:%u]", get_process_id(), get_process_id(), get_thread_id(), get_thread_id() );

	if( false == InitException() )
		return false;

	MemMark( "database", pInitNode );
	if( false == InitServerDatabase( ini ) )
		return false;

	SYS_INFO( "���ݿ����ӳ�ʼ���ɹ���" );

	MemMark( "refresh", pInitNode );
	if( false == InitServerRefresh( ini ) )
		return false;

	SYS_INFO( "ˢ��ϵͳ��ʼ���ɹ���" );

	MemMark( "asyncevent", pInitNode );
	if( false == InitServerEvent() )
		return false;

	SYS_INFO( "�첽�߼������ʼ���ɹ���" );

	MemMark( "debugcommand", pInitNode );
	if( false == InitDebugCmd( ini, xgc_nullptr ) )
		return false;

	SYS_INFO( "����ָ��ϵͳ��ʼ���ɹ���" );

	if( bUseSequence )
	{
		MemMark( "sequence", pInitNode );
		if( false == ServerSequence::InitServerSequence() )
			return false;

		SYS_INFO( "Sequence ��ʼ���ɹ���" );
	}

	ReportServiceStatus( SERVICE_STATUS_START_PENDING, SERVICE_ERROR_NONE, 5 * 60 * 1000 );
	MemMark( "configuration", pInitNode );
	if( false == InitConfiguration( ini, lpParam ) )
	{
		USR_ERROR( "InitConfiguration failed." );
		return false;
	}
	SYS_INFO( "�������ݳ�ʼ���ɹ���" );

	MemMark( "network", pInitNode );
	xgc_lpcstr network = ini.get_item_value( "Network", "config", "network.xml" );
	if( false == InitializeNetwork( network ) )
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
xgc_void ServerLoop( xgc_bool( *OnServerStep )( xgc_bool, xgc_lpvoid ), xgc_lpvoid lpParam )
{
	// ������ʱ���
	getInvokeWatcherMgr().Start();
	SYS_INFO( "��������ʼ���ɹ�" );

	xgc_bool bBusy = false;
	datetime tLast = datetime::now();

	while( false == IsServerStatus( SERVICE_STATUS_STOP_PENDING ) )
	{
		if( IsServerPaused() )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds(1) );
			continue;
		}

		// ��������״̬
		ReportServiceStatus( SERVICE_STATUS_RUNNING, SERVICE_ERROR_NONE, 0 );

		bBusy = false;

		// ������������
		bBusy = ProcessNetwork() || bBusy;

		// �������ݿ����
		bBusy = AsyncDBExecuteResp( 50 ) > 0 || bBusy;

		// ����������߼�
		if( OnServerStep( bBusy, lpParam ) )
			break;

		// ����ˢ���¼�
		StepServerRefresh();

		// ������Ҫ���첽�߼�����
		ExecEvent();

		// �Ƿ���У����е�ʱ��Ҫ�������������
		if( bBusy == false )
			std::this_thread::sleep_for( std::chrono::milliseconds(1) );

		// ÿ����ӱ���һ���ڴ�״̬
		if( datetime::now() - tLast >= timespan::from_minutes( 5 ) )
		{
			xgc_uint64 nMem = 0;
			xgc_uint64 nVMem = 0;
			get_process_memory_usage( &nMem, &nVMem );
			SYS_INFO( "Mem: %llu  VMem : %llu", nMem, nVMem );

			tLast = datetime::now();
		}
	}

	SYS_INFO( "�رճ�ʱ���..." );
	getInvokeWatcherMgr().Stop();
}

xgc_void ServerFini( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam )
{
	FUNCTION_BEGIN;
	ReportServiceStatus( SERVICE_STATUS_STOP_PENDING, SERVICE_ERROR_NONE, 5 * 60 * 1000 );
	FinializeNetwork();
	SYS_INFO( "������������..." );

	FiniConfiguration( lpParam );
	SYS_INFO( "�������������..." );

	FiniDebugCmd();
	SYS_INFO( "����ָ��ϵͳ�������..." );

	FiniServerRefresh();
	SYS_INFO( "ˢ��ϵͳ�������..." );

	FiniServerEvent();
	SYS_INFO( "�첽�߼������������..." );

	FiniServerDatabase();
	SYS_INFO( "���ݿ��������..." );

	FiniException();
	FinializeLogger();

	MemMarkClear();
	ReportServiceStatus( SERVICE_STATUS_STOPPED, SERVICE_ERROR_NONE, 0 );
	FUNCTION_END;
}

xgc_void ServerConfigFile( xgc_lpstr szPath, xgc_size nSize )
{
	strcpy_s( szPath, nSize, szConfigPath );
}

xgc_void ServerConfigPath( xgc_lpstr szPath, xgc_size nSize )
{
	path_dirs( szPath, nSize, szConfigPath );
}
///
/// ��ȡ��������
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr ServerName()
{
	return szServerName;
}
