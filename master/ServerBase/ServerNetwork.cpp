#include "stdafx.h"
#include "ServerNetwork.h"
#include "PipeSession.h"
#include "GameSession.h"
#include "GateSession.h"
#include "ServerBase.h"
#include "sdserverid.h"
#include "ServerMonitor.h"

using namespace SGDP;
ISDNet			*gpNetModule	= xgc_nullptr;
ISDPipeModule	*gpPipeModule	= xgc_nullptr;
ISDListener		*gpListener		= xgc_nullptr;
ISDGate			*gpGate			= xgc_nullptr;

DWORD			 gnListenIndex = -1;

xgc_bool         gbMonitorIsOpen = false;

volatile xgc_lpvoid gpFirstNetBuffer = xgc_nullptr;
__declspec( thread ) xgc_lpvoid gpNetBuffer = xgc_nullptr;

class CGGPacketParser : public ISDPacketParser
{
public:
	virtual xgc_int32 SDAPI ParsePacket( xgc_lpcstr data, xgc_uint32 length )
	{
		if( length <= TOTAL_HEADER_LEN )
		{
			//BodyΪ��Ҳ����, ��ͷ���������İ�
			return 0;
		}

		const PkgHeader *pHeader = reinterpret_cast<const PkgHeader*>( data );
		if( pHeader->uLength > length - PKG_HEADER_LEN )
		{
			//��������
			return 0;
		}

		if( pHeader->uLength > MAX_RAW_DATA_LEN || pHeader->uLength <= MSG_HEADER_LEN )
		{
			//�����Ȳ���
			return -1;
		}

		return pHeader->uLength;
	}
};

static CGGPacketParser gGatePacketParser;

xgc_bool InitializeNetwork( IniFile &ini )
{
	FUNCTION_BEGIN;

	xgc_uint16 nListenMode = 0;
	
	// ��������
	xgc_uint16 nListenPort = ini.GetItemValue( "Network", "ListenPort", 0 );
	if( nListenPort )
	{
		xgc_lpcstr lpListenMode = ini.GetItemValue( "Network", "ListenMode", xgc_nullptr );
		if( xgc_nullptr == lpListenMode )
		{
			SYS_ERROR( "������ Network.ListenMode û����ȷ���á�" );
			return false;
		}

		if( stricmp( lpListenMode, "Normal" ) == 0 )
			nListenMode = 0;
		else if( stricmp( lpListenMode, "GateServer" ) == 0 )
			nListenMode = 1;
		else if( stricmp( lpListenMode, "GameServer" ) == 0 )
			nListenMode = 2;
		else
		{
			SYS_ERROR( "������ Network.ListenMode û����ȷ���á�(δ֪�ļ���ģʽ��" );
			return false;
		}
	}

	xgc_uint32 nSendBuff = ini.GetItemValue( "Network", "RecvBuffer", 16777216U );
	xgc_uint32 nRecvBuff = ini.GetItemValue( "Network", "RecvBuffer", 16777216U );

	xgc_char szOptionPath[_MAX_PATH];
	xgc_char szIPListPath[_MAX_PATH];

	xgc_lpcstr lpOptionPath = ini.GetItemValue( "Network", "OptionCfg", "option.xml" );
	if( xgc_nullptr == lpOptionPath )
	{
		SYS_ERROR( "������ Network.OptionCfg û����ȷ���á�" );
		return false;
	}
	GetNormalPath( szOptionPath, sizeof( szOptionPath ), "%s", lpOptionPath );
	if( _access( szOptionPath, 0 ) == -1 )
	{
		SYS_ERROR( "������ Network.OptionCfg ��ָ����ļ�[%s]�����ڡ�", szOptionPath );
		return false;
	}

	xgc_lpcstr lpIPListPath = ini.GetItemValue( "Network", "IPListCfg", "iplist.xml" );
	if( xgc_nullptr == lpIPListPath )
	{
		SYS_ERROR( "������ Network.IPListCfg û����ȷ���á�" );
		return false;
	}
	GetNormalPath( szIPListPath, sizeof( szIPListPath ), "%s", lpIPListPath );
	if( _access( szIPListPath, 0 ) == -1 )
	{
		SYS_ERROR( "������ Network.szIPListPath ��ָ����ļ�[%s]�����ڡ�", szIPListPath );
		return false;
	}

	SYS_INFO( "��ʼ��ʼ������ģ��..." );
	xgc_lpcstr lpParentNode = "network";
	MemMark( "module", lpParentNode );
	gpNetModule = SDNetGetModule( &SDNET_VERSION );
	if( xgc_nullptr == gpNetModule )
	{
		SYS_ERROR( "�����ʼ�� - ����ģ����ʧ�ܡ�" );
		return false;
	}

	MemMark( "pipe", lpParentNode );
	gpPipeModule = SDPipeGetModule( &SDPIPE_VERSION );
	if( xgc_nullptr == gpPipeModule )
	{
		SYS_ERROR( "�����ʼ�� - �ܵ�ģ�鴴��ʧ��" );
		SAFE_RELEASE( gpNetModule );
		return false;
	}

	switch( nListenMode )
	{
		case 0: // Normal
		{
			MemMark( "listen", lpParentNode );
			gpListener = gpNetModule->CreateListener( NETIO_COMPLETIONPORT );
			if( xgc_nullptr == gpListener )
			{
				SYS_ERROR( "�����ʼ�� - �������󴴽�ʧ�ܡ�" );
				SAFE_RELEASE( gpPipeModule );
				SAFE_RELEASE( gpNetModule );
				return false;
			}

			MemMark( "setting", lpParentNode );

			gpListener->SetBufferSize( nRecvBuff, nSendBuff );
			gpListener->SetPacketParser( &gGatePacketParser );
			gpListener->SetSessionFactory( &getGameSessionFactory() );

			MemMark( "start", lpParentNode );
			if( !gpListener->Start( "0.0.0.0", nListenPort ) )
			{
				SYS_ERROR( "�����ʼ�� - �����˿�%dʧ��", nListenPort );
				SAFE_RELEASE( gpListener );
				SAFE_RELEASE( gpPipeModule );
				SAFE_RELEASE( gpNetModule );
				return false;
			}
		}
		break;
		case 1: // GateServer
		{
			gpGate = SDCreateGate( &SGDP::SDGATE_VERSION, gpNetModule );
			if( NULL == gpGate )
			{
				SYS_WARNING( "--Create Gate failed." );
				return false;
			}

			gpGate->SetClientPacketParser( &gGatePacketParser );
			gpGate->SetClientSessionFactory( xgc_nullptr );
			gpGate->SetClientBufferSize( nRecvBuff, nSendBuff );
			
			gnListenIndex = gpGate->AddListen( "0.0.0.0", nListenPort );
		}
		break;
		case 2: // GameServer
		{
			MemMark( "listen", lpParentNode );
			gpListener = gpNetModule->CreateListener( NETIO_COMPLETIONPORT_GATE );
			if( xgc_nullptr == gpListener )
			{
				SYS_ERROR( "�����ʼ�� - �������󴴽�ʧ�ܡ�" );
				SAFE_RELEASE( gpPipeModule );
				SAFE_RELEASE( gpNetModule );
				return false;
			}

			MemMark( "setting", lpParentNode );

			gpListener->SetBufferSize( nRecvBuff, nSendBuff );
			gpListener->SetPacketParser( &gGatePacketParser );

			SListenerOptSessionFactory psListenerOptSessionFactory;
			psListenerOptSessionFactory.pSessionFactory = &getGateSessionFactory();
			gpListener->SetOpt( LISTENER_OPT_SESSION_FACTORY, &psListenerOptSessionFactory );
			gpListener->SetSessionFactory( &getGameSessionFactory() );

			MemMark( "start", lpParentNode );
			if( !gpListener->Start( "0.0.0.0", nListenPort ) )
			{
				SYS_ERROR( "�����ʼ�� - �����˿�%dʧ��", nListenPort );
				SAFE_RELEASE( gpListener );
				SAFE_RELEASE( gpPipeModule );
				SAFE_RELEASE( gpNetModule );
				return false;
			}
		}
		break;
	}

	MemMark( "pipe init", lpParentNode );
	if( false == gpPipeModule->Init( szOptionPath, szIPListPath, &getPipeReporter(), gpNetModule ) )
	{
		SYS_ERROR( "�����ʼ�� - �ܵ�ģ���ʼ��ʧ�� �ܵ������ļ�%s,%s", szOptionPath, szIPListPath );
		SAFE_RELEASE( gpListener );
		SAFE_RELEASE( gpPipeModule );
		SAFE_RELEASE( gpNetModule );

		return false;
	}
	SYS_INFO( "��ʼ������ģ�����" );

	return true;
	FUNCTION_END;

	return false;
}

xgc_void FinializeNetwork()
{
	if( gpListener )
	{
		gpListener->Stop();
	}

	if( gpGate )
	{
		gpGate->DelListen( gnListenIndex );
	}

	// �˴�Run��Ϊ�˴���GateSession�Ͽ�����Ϣ��
	if( gpNetModule )
	{
		gpNetModule->Run();
	}

	SAFE_RELEASE( gpListener );
	SAFE_RELEASE( gpGate );
	SAFE_RELEASE( gpPipeModule );
	SAFE_RELEASE( gpNetModule );
}

xgc_bool ProcessNetwork()
{
	FUNCTION_BEGIN;
	xgc_bool busy = false;
	busy = gpNetModule->Run() || busy;
	busy = gpPipeModule->Run() || busy;
	return busy;
	FUNCTION_END;

	return true;
}

///
/// Monitor�¼��������
/// [1/4/2015] create by jianglei.kinly
///
xgc_void MT_EvtHandler( CPipeSession* pNetSession, xgc_uint16 nEvent, xgc_uint32 nCode )
{
	// ���ﴦ��reconnect�߼�
	XGC_ASSERT_RETURN( pNetSession, xgc_void( 0 ) );
	if( nEvent == SESSION_EVENT_CONNECTED )
	{
		MonitorConnect();
	}
}

///
/// Monitor��Ϣ�������
/// [1/4/2015] create by jianglei.kinly
///
xgc_void MT_MsgHandler( CPipeSession* pNetSession, xgc_uint16 nMsgID, xgc_lpcstr lpData, xgc_size nSize, xgc_uint32 nData )
{
	FUNCTION_BEGIN;
	FUNCTION_END;
}

xgc_bool InitializeMTNetwork( IniFile& ini )
{
	FUNCTION_BEGIN;
	///< Monitor������IP
	xgc_lpcstr lpMonitorIsDisabled = xgc_nullptr;

	if( !ini.IsExistSection( "Monitor" ) )
	{
		gbMonitorIsOpen = false;
		SYS_INFO( "Monitor ����û�г�ʼ����" );
		return true;
	}

	lpMonitorIsDisabled = ini.GetItemValue( "Monitor", "IsDisabled", "false" );
	if( stricmp( lpMonitorIsDisabled, "true" ) == 0 )
	{
		gbMonitorIsOpen = false;
		SYS_INFO( "Monitor ����û�г�ʼ����" );
		return true;
	}
	gbMonitorIsOpen = true;

	RegistPipeHandler( ServerType::SD_MONITOR_SERVER, MT_MsgHandler, MT_EvtHandler );

	SYS_INFO( "Monitor �����ʼ���ɹ���" );

	return true;
	FUNCTION_END;
	return false;
}

xgc_bool GetMonitorIsOpen()
{
	return gbMonitorIsOpen;
}