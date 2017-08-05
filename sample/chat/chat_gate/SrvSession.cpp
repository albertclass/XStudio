#include "header.h"
#include "SrvSession.h"

CServerSession::CServerSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
{
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

	chat::create_channel_req req;

	// 服务器启动后请求聊天服务器创建如下公共频道
	req.set_channel_name( "2-1-0-1.world" );
	Send( chat::MSG_CREATE_CHANNEL_REQ, req );

	req.set_channel_name( "2-1-0-1.guild" );
	Send( chat::MSG_CREATE_CHANNEL_REQ, req );

	req.set_channel_name( "2-1-0-1.friends" );
	Send( chat::MSG_CREATE_CHANNEL_REQ, req );
}

xgc_void CServerSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %0X error, code = %0X\r\n", handle_, error_code );
}

xgc_void CServerSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	delete this;
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
		case chat::MSG_CREATE_CHANNEL_ACK:
		break;
		case chat::MSG_LOGIN_ACK:
		break;
		case chat::MSG_LOGOUT_ACK:
		break;
	}
}

///
/// \brief 用户登陆
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::UserLogin( xgc_uint64 user_id, const xgc_string &nickname )
{
	chat::login_req req;
	req.set_user_id( user_id );
	req.set_nick( nickname );
	req.set_server( "2-1-0-1" );
	req.set_channel_wild( "2-1-0-1.*" );
	req.set_extra( "1111 2222 3333 4444 5555" );

	Send( chat::MSG_LOGIN_REQ, req );
}

///
/// \brief 用户登出
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::UserLogout( xgc_uint64 user_id )
{
	chat::logout_req req;
	req.set_user_id( user_id );

	Send( chat::MSG_LOGOUT_REQ, req );
}

///
/// \brief 发送系统通知
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::SendSystemNote( xgc_uint32 channel_id, xgc_string note, xgc_uint32 flags )
{
	chat::system_chat_req req;

	req.set_channel( channel_id );
	req.set_text( note );
	req.set_flags( flags );

	Send( chat::MSG_SYS_CHAT_REQ, req );
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
