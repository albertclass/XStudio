#include "config.h"
#include "net_module.h"
#include "base_session.h"
#include "cli_session.h"
#include "pipe_session.h"
#include "relay_session.h"
#include "pipe_manager.h"

using namespace xgc::net;

namespace net_module
{
	ClientMsgParser  pfn_ClientMsgParser;
	ClientMsgHandler pfn_ClientMsgHandler;
	ClientEvtHandler pfn_ClientEvtHandler;

	xgc_time64 tick()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()
			).count();
	}
}

///
/// \brief 创建网络句柄
///
/// \author albert.xu
/// \date 2017/03/01 11:32
///
template< class SessionType >
INetworkSession * CreateSession()
{
	return XGC_NEW SessionType();
}

/// 所有的监听端口
static xgc::vector< xgc_lpvoid > g_listener;

xgc_bool make_connect( pugi::xml_node &node, net_module::enListenMode mode )
{
	auto attr_inet = node.attribute( "id" );
	auto attr_addr = node.attribute( "address" );
	auto attr_port = node.attribute( "port" );
	auto attr_timeout = node.attribute( "timeout" );

	if( attr_addr.empty() )
	{
		SYS_ERR( "读取配置文件失败 - 未找到 /network/connect@address" );
		return false;
	}

	if( attr_port.empty() )
	{
		SYS_ERR( "读取配置文件失败 - 未找到 /network/connect@port" );
		return false;
	}

	/// 连接参数
	auto recv_buffer_len = 4 * 1024ULL;
	auto send_buffer_len = 4 * 1024ULL;

	auto node_recv_buffer_len = node.child( "recv_buffer_len" );
	if( node_recv_buffer_len )
	{
		recv_buffer_len = node_recv_buffer_len.attribute( "value" ).as_ullong();
	}

	auto node_send_buffer_len = node.child( "send_buffer_len" );
	if( node_send_buffer_len )
	{
		send_buffer_len = node_send_buffer_len.attribute( "value" ).as_ullong();
	}

	auto recv_packet_max = 1024ULL;
	auto send_packet_max = 1024ULL;

	auto node_recv_packet_max = node.child( "recv_packet_max" );
	if( node_recv_packet_max )
	{
		recv_packet_max = node_recv_packet_max.attribute( "value" ).as_ullong();
	}

	auto node_send_packet_max = node.child( "send_packet_max" );
	if( node_send_packet_max )
	{
		send_packet_max = node_send_packet_max.attribute( "value" ).as_ullong();
	}

	auto inet = attr_inet.as_string();
	auto addr = attr_addr.as_string();
	auto port = attr_port.as_uint();

	auto timeout = attr_timeout.as_uint();

	connect_options options;
	memset( &options, 0, sizeof( options ) );
	options.recv_buffer_size = recv_buffer_len;
	options.send_buffer_size = send_buffer_len;
	options.recv_packet_max = recv_packet_max;
	options.send_packet_max = send_packet_max;

	xgc_bool ret = false;
	switch( mode )
	{
		case net_module::eMode_Normal:
		case net_module::eMode_Gate:
		{
			options.is_async = true;

			ret = net::Connect(
				addr,
				port,
				XGC_NEW CClientSession(),
				&options );
		}
		break;
		case net_module::eMode_Pipe:
		{
			options.is_async = true;
			options.is_reconnect_timeout = true;
			options.is_reconnect_passive = true;

			auto attr_id = node.attribute( "id" );

			if( attr_id.empty() )
			{
				SYS_ERR( "读取配置文件失败 - 未找到 /network/connect@id" );
				return false;
			}

			ret = net_module::PipeConnect( Str2NetworkId( inet ), addr, port, options );
		}
		break;
	}

	return ret;
}

xgc_bool make_listen( pugi::xml_node &node )
{
	auto attr_addr = node.attribute( "address" );
	auto attr_port = node.attribute( "port" );
	auto attr_timeout = node.attribute( "timeout" );

	if( attr_port.empty() )
	{
		SYS_ERR( "读取配置文件失败 - 未找到 /network/listen@port" );
		return false;
	}

	auto attr_mode = node.attribute( "mode" );
	if( attr_mode.empty() )
	{
		SYS_ERR( "读取配置文件失败 - 未找到 /network/listen@mode" );
		return false;
	}

	auto mode = attr_mode.as_uint();
	auto addr = attr_addr.as_string( "0.0.0.0" );
	auto port = attr_port.as_uint();
	auto timeout = attr_timeout.as_uint();

	/// 连接参数
	auto recv_buffer_len = 4 * 1024ULL;
	auto send_buffer_len = 4 * 1024ULL;

	auto node_recv_buffer_len = node.child( "recv_buffer_len" );
	if( node_recv_buffer_len )
	{
		recv_buffer_len = node_recv_buffer_len.attribute( "value" ).as_ullong();
	}

	auto node_send_buffer_len = node.child( "send_buffer_len" );
	if( node_send_buffer_len )
	{
		send_buffer_len = node_send_buffer_len.attribute( "value" ).as_ullong();
	}

	auto recv_packet_max = 1024ULL;
	auto send_packet_max = 1024ULL;

	auto node_recv_packet_max = node.child( "recv_packet_max" );
	if( node_recv_packet_max )
	{
		recv_packet_max = node_recv_packet_max.attribute( "value" ).as_ullong();
	}

	auto node_send_packet_max = node.child( "send_packet_max" );
	if( node_send_packet_max )
	{
		send_packet_max = node_send_packet_max.attribute( "value" ).as_ullong();
	}

	std::vector< std::tuple< uint64_t, uint64_t > > allow_address;
	auto inserter = std::back_inserter( allow_address );
	auto node_allow = node.child( "allow" );
	if( node_allow )
	{
		auto node_range = node_allow.child( "address" );
		while( node_range )
		{
			auto addr = node_range.attribute( "addr" );
			auto from = node_range.attribute( "from" );
			auto to = node_range.attribute( "to" );

			if( addr )
			{
				auto addr_from = inet_addr( addr.as_string() );
				auto addr_to   = inet_addr( addr.as_string() );

				inserter = std::make_tuple( ntohl( addr_from ), ntohl( addr_to ) );
			}
			else if( from && to )
			{
				auto addr_from = inet_addr( from.as_string() );
				auto addr_to   = inet_addr(   to.as_string() );

				inserter = std::make_tuple( ntohl( addr_from ), ntohl( addr_to ) );
			}
			else
			{
				SYS_WRN( "allow address range format error at '%s'", node_range.path().c_str() );
			}
			node_range = node_range.next_sibling( "address" );
		}
	}
	xgc_lpvoid srv = xgc_nullptr;
	
	server_options options;

	memset( &options, 0, sizeof( options ) );
	options.recv_buffer_size = recv_buffer_len;
	options.send_buffer_size = send_buffer_len;
	options.recv_packet_max = recv_packet_max;
	options.send_packet_max = send_packet_max;

	options.acceptor_count = node.attribute( "acceptor" ).as_uint( 10 );
	options.acceptor_smart = 100;
	options.heartbeat_interval = node.attribute( "heart" ).as_uint( 1000 );

	options.allow_count = 0;
	for( auto it : allow_address )
	{
		options.allow_addr[options.allow_count].from = std::get<0>( it );
		options.allow_addr[options.allow_count].to   = std::get<1>( it );

		++options.allow_count;

		if( options.allow_count >= XGC_COUNTOF( options.allow_addr ) )
			break;
	}
	
	if( strcasecmp( "normal", attr_mode.as_string() ) == 0 )
	{
		srv = net::StartServer(
			addr,
			port,
			&options,
			&CreateSession< CClientSession > );
	}
	else if( strcasecmp( "pipe", attr_mode.as_string() ) == 0 )
	{
		srv = net::StartServer(
			addr,
			port,
			&options,
			&CreateSession< CPipeSession > );
	}
	else if( strcasecmp( "gate", attr_mode.as_string() ) == 0 )
	{
		srv = net::StartServer(
			addr,
			port,
			&options,
			&CreateSession< CClientSession > );

		// 连接所有的转发服务器
		auto connections_node = node.child( "connections" );
		if( !connections_node.empty() )
		{
			auto connect_node = connections_node.child( "connect" );
			while( !connect_node.empty() )
			{
				if( strcmp( "connect", connect_node.name() ) == 0 )
				{
					make_connect( connect_node, net_module::eMode_Pipe );
				}

				connect_node = connect_node.next_sibling( "connect" );
			}
		}
	}

	if( srv )
	{
		g_listener.push_back( srv );
		return true;
	}

	return false;
}

xgc_bool InitializeNetwork( xgc_lpcstr conf )
{
	FUNCTION_BEGIN;

	if( false == net_module::InitPipeManager() )
	{
		SYS_ERR( "初始化管道管理器失败。" );
		return false;
	}

	net::CreateNetwork( 4 );

	char filename[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( filename, "%s", conf ) )
	{
		SYS_ERR( "读取配置文件失败 - 文件路径解析失败。" );
		return false;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file( filename );
	if( false == res )
	{
		SYS_ERR( "读取配置文件失败 - %s", res.description() );
		return false;
	}

	auto root = doc.root();
	auto node_network = root.child( "network" );
	if( node_network.empty() )
	{
		SYS_ERR( "读取配置文件失败 - 未找到 /network" );
		return false;
	}

	auto attr_id = node_network.attribute( "id" );
	if( attr_id.empty() )
	{
		SYS_ERR( "读取配置文件失败 - 未找到 /network/server@id" );
		return false;
	}

	net_module::_SetNetworkID( attr_id.as_string() );

	auto node = node_network.first_child();
	while( !node.empty() )
	{
		if( strcasecmp( "listen", node.name() ) == 0 )
		{
			if( false == make_listen( node ) )
				return false;
		}
		else if( strcasecmp( "connect", node.name() ) == 0 )
		{
			if( false == make_connect( node, net_module::eMode_Pipe ) )
				return false;
		}

		node = node.next_sibling();
	}

	SYS_TIP( "初始化网络模块完成" );

	return true;
	FUNCTION_END;

	return false;
}

xgc_void FinializeNetwork()
{
	for( auto srv : g_listener )
	{
		CloseServer( srv );
	}

	net::DestroyNetwork();

	net_module::FiniPipeManager();
}

xgc_bool ProcessNetwork()
{
	FUNCTION_BEGIN;
	return net::ProcessNetEvent( 1000 ) != 1000;
	FUNCTION_END;

	return true;
}

///
/// 获取服务器编号
/// [11/27/2014] create by albert.xu
///
NETWORK_ID GetNetworkId()
{
	return net_module::_GetNetworkID();
}

///
/// 拼凑服务器ID
/// [11/27/2014] create by albert.xu
///
NETWORK_ID GetNetworkRegion( NETWORK_REGION Id[NETWORK_REGION_N] )
{
	NETWORK_ID nNetworkID = GetNetworkId();
	NETWORK_REGION * pNetworkID = (NETWORK_REGION*)&nNetworkID;
	
	for( int i = 0; i < sizeof(nNetworkID)/sizeof(NETWORK_REGION); ++i )
		Id[i] = pNetworkID[i];

	return nNetworkID;
}

///
/// 转换服务器ID到字符串
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr NetworkId2Str( NETWORK_ID nNetworkId, xgc_char szOutput[NETWORK_REGION_S] )
{
	return net_module::_NetworkID2Str( nNetworkId, szOutput );
}

///
/// 转换服务器ID为数值
/// [11/27/2014] create by albert.xu
///
NETWORK_ID Str2NetworkId( xgc_lpcstr pNetworkId )
{
	return net_module::_Str2NetworkID( pNetworkId );
}

xgc_void MakeVirtualSock( CClientSession * pSession, NETWORK_ID nNetworkID )
{
	auto pPipe =  net_module::GetPipe( nNetworkID );
	if( pPipe )
	{
		pPipe->RelayConnect( pSession );
	}
}

xgc_void KickVirtualSock( CClientSession * pSession, NETWORK_ID nNetworkID )
{
	auto pPipe = net_module::GetPipe( nNetworkID );
	if( pPipe )
	{
		pPipe->RelayDisconnect( pSession );
	}
}

xgc_void RegistClientHandler( ClientMsgHandler fnMsgHandler, ClientEvtHandler fnEvtHandler, ClientMsgParser fnMsgParser )
{
	net_module::pfn_ClientMsgParser = fnMsgParser;
	net_module::pfn_ClientMsgHandler = fnMsgHandler;
	net_module::pfn_ClientEvtHandler = fnEvtHandler;
}

///
/// 注册管道消息处理函数
/// [11/27/2014] create by albert.xu
///
xgc_void RegistPipeHandler( xgc_lpcstr lpNetworkId, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler )
{
	net_module::RegistPipeHandler( lpNetworkId, fnMsgHandler, fnEvtHandler );
}

xgc_void RegistVirtualSockHandler( xgc_lpcstr lpNetworkId, SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler )
{
	net_module::RegistSockHandler( lpNetworkId, fnMsgHandler, fnEvtHandler );
}

///
/// \brief 默认的数据分包回调
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
int DefaultPacketParser( const void* data, size_t size )
{
	if( size < sizeof( xgc_uint16 ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*)data );
	if( size < length )
		return 0;

	return length;
}

///
/// \brief 发送数据到客户端会话
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
xgc_void SendPacket( CClientSession * pSession, xgc_lpvoid pData, xgc_size nSize )
{
	if( pSession )
		((net_module::CBaseSession*)pSession)->Send( pData, nSize );
}

///
/// \brief 发送数据到管道
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
xgc_void SendPacket( CPipeSession * pSession, xgc_lpvoid pData, xgc_size nSize )
{
	if( pSession )
		pSession->SendPacket( pData, nSize );
}

///
/// \brief 发送数据到中继
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
xgc_void SendPacket( CRelaySession * pSession, xgc_lpvoid pData, xgc_size nSize )
{
	if( pSession )
		((net_module::CBaseSession*)pSession)->Send( pData, nSize );
}

///
/// \brief 转发数据
///
/// \author albert.xu
/// \date 2017/08/01
///
xgc_void RelayPacket( CClientSession* pSession, xgc_lpvoid pData, xgc_size nSize )
{
	pSession->Relay( pData, nSize );
}

///
/// \brief 转发数据
///
/// \author albert.xu
/// \date 2017/08/01
///
xgc_void RelayPacket( CRelaySession* pSession, xgc_lpvoid pData, xgc_size nSize )
{
	pSession->Relay( pData, nSize );
}
