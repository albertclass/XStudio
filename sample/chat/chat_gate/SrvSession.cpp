#include "header.h"
#include "gate.pb.h"
#include "SrvSession.h"
#include "CliSession.h"

CServerSession::CServerSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, peer_port_( -1 )
{
	memset( peer_host_, 0, sizeof(peer_host_) );
}

CServerSession::~CServerSession()
{
}

int CServerSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CServerSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );
}

xgc_void CServerSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );
	net::Param_QueryHandleInfo param;
	param.handle = handle;
	if( 0 == net::ExecuteState( Operator_QueryHandleInfo, &param ) )
	{
		in_addr in;
		in.S_un.S_addr = htonl(param.addr[1]);
		char* addr = inet_ntoa( in );
		if( addr )
			strcpy_s( peer_host_, addr );
		peer_port_ = param.port[1];
	}

	chat::create_channel_req req;

	// 服务器启动后请求聊天服务器创建如下公共频道
	req.set_channel_name( "2-1-0-1.world" );
	Send( chat::CHAT_CREATE_CHANNEL_REQ, req );

	req.set_channel_name( "2-1-0-1.guild" );
	Send( chat::CHAT_CREATE_CHANNEL_REQ, req );

	req.set_channel_name( "2-1-0-1.friends" );
	Send( chat::CHAT_CREATE_CHANNEL_REQ, req );
}

xgc_void CServerSession::OnError( xgc_int16 error_type, xgc_int16 error_code )
{
	fprintf( stderr, "session %0X error, type = %d, code = %d\r\n", handle_, error_type, error_code );
}

xgc_void CServerSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
}

xgc_void CServerSession::OnAlive()
{
}

xgc_void CServerSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*)data;
	auto length = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (int)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case chat::CHAT_CREATE_CHANNEL_ACK:
		onCreateChannelAck( ptr, len );
		break;
		case chat::CHAT_LOGIN_ACK:
		onLoginAck( ptr, len );
		break;
	}
}

///
/// \brief 创建频道回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::onCreateChannelAck( xgc_lpvoid ptr, int len )
{
	chat::create_channel_ack ack;
	ack.ParseFromArray( ptr, len );

	auto channel_id   = ack.channel_id();
	auto channel_name = ack.channel_name();

	auto it = channels_.find( channel_id );
	if( it == channels_.end() )
	{
		channels_.insert( { channel_id, channel_name } );
	}
	else
	{
		it->second = channel_name;
	}
}

///
/// \brief 角色登陆聊天服务器回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::onLoginAck( xgc_lpvoid ptr, int len )
{
	chat::login_ack ack1;
	ack1.ParseFromArray( ptr, len );

	auto result = ack1.result();
	auto user_id = ack1.user_id();

	auto it = users_.find( user_id );
	if( it == users_.end() )
		return;

	auto session = CClientSession::handle_exchange( it->second );
	if( xgc_nullptr == session )
		return;

	gate::login_ack ack2;
	ack2.set_result( result );

	if( result >= 0 )
	{
		ack2.set_user_id( user_id );
		ack2.set_chat_token( ack1.token() );
		ack2.set_host( getPeerHost() );
		ack2.set_port( getPeerPort() );
	}

	session->Send( gate::GATE_LOGIN_ACK, ack2 );
}

///
/// \brief 用户登陆
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_long CServerSession::UserLogin( xgc_uint64 user_id, const xgc::string &nickname, xgc_uint32 session_id )
{
	if( handle_ == INVALID_NETWORK_HANDLE )
		return -1;

	auto it = users_.find( user_id );
	if( it != users_.end() )
		return -2;

	users_.insert( { user_id, session_id } );

	chat::login_req req;
	req.set_user_id( user_id );
	req.set_nick( nickname );
	req.set_server( "2-1-0-1" );
	req.set_channel_wild( "2-1-0-1.*" );
	req.set_extra( "1111 2222 3333 4444 5555" );

	Send( chat::CHAT_LOGIN_REQ, req );
	return 0;
}

///
/// \brief 用户登出
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::UserLogout( xgc_uint64 user_id )
{
	auto it = users_.find( user_id );
	if( it != users_.end() )
		users_.erase( it );

	chat::logout_req req;
	req.set_user_id( user_id );

	Send( chat::CHAT_LOGOUT_REQ, req );
}

///
/// \brief 发送系统通知
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::SendSystemNote( xgc_uint32 channel_id, xgc::string note, xgc_uint32 flags )
{
	chat::system_chat_req req;

	req.set_channel( channel_id );
	req.set_text( note );
	req.set_flags( flags );

	Send( chat::CHAT_SYSTEM_CHAT_REQ, req );
}

///
/// \brief 发送数据
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::Send( xgc_uint16 msgid, ::google::protobuf::Message& msg )
{
#pragma pack(1)
	struct
	{
		MessageHeader h;
		char b[1024 * 4];
	} m;
#pragma pack()

	xgc_uint16 data_size = msg.ByteSize();
	xgc_uint16 pack_size = sizeof( MessageHeader ) + data_size;

	m.h.length = htons( pack_size );
	m.h.message = htons( msgid );

	msg.SerializeToArray( m.b, sizeof( m.b ) );

	SendPacket( handle_, &m, pack_size );
}
