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
			//Body为空也不行, 连头都不完整的包
			return 0;
		}

		const PkgHeader *pHeader = reinterpret_cast<const PkgHeader*>( data );
		if( pHeader->uLength > length - PKG_HEADER_LEN )
		{
			//不完整包
			return 0;
		}

		if( pHeader->uLength > MAX_RAW_DATA_LEN || pHeader->uLength <= MSG_HEADER_LEN )
		{
			//包长度不对
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
	
	// 网络配置
	xgc_uint16 nListenPort = ini.GetItemValue( "Network", "ListenPort", 0 );
	if( nListenPort )
	{
		xgc_lpcstr lpListenMode = ini.GetItemValue( "Network", "ListenMode", xgc_nullptr );
		if( xgc_nullptr == lpListenMode )
		{
			SYS_ERROR( "配置项 Network.ListenMode 没有正确配置。" );
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
			SYS_ERROR( "配置项 Network.ListenMode 没有正确配置。(未知的监听模式）" );
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
		SYS_ERROR( "配置项 Network.OptionCfg 没有正确配置。" );
		return false;
	}
	GetNormalPath( szOptionPath, sizeof( szOptionPath ), "%s", lpOptionPath );
	if( _access( szOptionPath, 0 ) == -1 )
	{
		SYS_ERROR( "配置项 Network.OptionCfg 中指向的文件[%s]不存在。", szOptionPath );
		return false;
	}

	xgc_lpcstr lpIPListPath = ini.GetItemValue( "Network", "IPListCfg", "iplist.xml" );
	if( xgc_nullptr == lpIPListPath )
	{
		SYS_ERROR( "配置项 Network.IPListCfg 没有正确配置。" );
		return false;
	}
	GetNormalPath( szIPListPath, sizeof( szIPListPath ), "%s", lpIPListPath );
	if( _access( szIPListPath, 0 ) == -1 )
	{
		SYS_ERROR( "配置项 Network.szIPListPath 中指向的文件[%s]不存在。", szIPListPath );
		return false;
	}

	SYS_INFO( "开始初始化网络模块..." );
	xgc_lpcstr lpParentNode = "network";
	MemMark( "module", lpParentNode );
	gpNetModule = SDNetGetModule( &SDNET_VERSION );
	if( xgc_nullptr == gpNetModule )
	{
		SYS_ERROR( "网络初始化 - 网络模创建失败。" );
		return false;
	}

	MemMark( "pipe", lpParentNode );
	gpPipeModule = SDPipeGetModule( &SDPIPE_VERSION );
	if( xgc_nullptr == gpPipeModule )
	{
		SYS_ERROR( "网络初始化 - 管道模块创建失败" );
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
				SYS_ERROR( "网络初始化 - 监听对象创建失败。" );
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
				SYS_ERROR( "网络初始化 - 监听端口%d失败", nListenPort );
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
				SYS_ERROR( "网络初始化 - 监听对象创建失败。" );
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
				SYS_ERROR( "网络初始化 - 监听端口%d失败", nListenPort );
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
		SYS_ERROR( "网络初始化 - 管道模块初始化失败 管道配置文件%s,%s", szOptionPath, szIPListPath );
		SAFE_RELEASE( gpListener );
		SAFE_RELEASE( gpPipeModule );
		SAFE_RELEASE( gpNetModule );

		return false;
	}
	SYS_INFO( "初始化网络模块完成" );

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

	// 此处Run是为了处理GateSession断开的消息。
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
/// Monitor事件处理入口
/// [1/4/2015] create by jianglei.kinly
///
xgc_void MT_EvtHandler( CPipeSession* pNetSession, xgc_uint16 nEvent, xgc_uint32 nCode )
{
	// 这里处理reconnect逻辑
	XGC_ASSERT_RETURN( pNetSession, xgc_void( 0 ) );
	if( nEvent == SESSION_EVENT_CONNECTED )
	{
		MonitorConnect();
	}
}

///
/// Monitor消息处理入口
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
	///< Monitor服务器IP
	xgc_lpcstr lpMonitorIsDisabled = xgc_nullptr;

	if( !ini.IsExistSection( "Monitor" ) )
	{
		gbMonitorIsOpen = false;
		SYS_INFO( "Monitor 网络没有初始化！" );
		return true;
	}

	lpMonitorIsDisabled = ini.GetItemValue( "Monitor", "IsDisabled", "false" );
	if( stricmp( lpMonitorIsDisabled, "true" ) == 0 )
	{
		gbMonitorIsOpen = false;
		SYS_INFO( "Monitor 网络没有初始化！" );
		return true;
	}
	gbMonitorIsOpen = true;

	RegistPipeHandler( ServerType::SD_MONITOR_SERVER, MT_MsgHandler, MT_EvtHandler );

	SYS_INFO( "Monitor 网络初始化成功！" );

	return true;
	FUNCTION_END;
	return false;
}

xgc_bool GetMonitorIsOpen()
{
	return gbMonitorIsOpen;
}