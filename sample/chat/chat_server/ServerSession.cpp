#include "header.h"
#include "User.h"
#include "UserMgr.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "ServerSession.h"

CServerSession::CServerSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, userdata_( xgc_nullptr )
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
}

xgc_void CServerSession::OnError( xgc_int16 error_type, xgc_int16 error_code )
{
	fprintf( stderr, "net session %u error, code = %u\r\n", handle_, error_code );
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
	auto len = (xgc_int32)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case chat::MSG_LOGIN_REQ:
		onUserLoginReq( ptr, len );
		break;
		case chat::MSG_LOGOUT_REQ:
		onUserLogoutReq( ptr, len );
		break;
		case chat::MSG_CREATE_CHANNEL_REQ:
		onCreateChannelReq( ptr, len );
		break;
		case chat::MSG_SYS_CHAT_REQ:
		onSystemNoteReq( ptr, len );
		break;
	}
}

///
/// \brief 角色登陆游戏，通知聊天服务器
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::onUserLoginReq( xgc_lpvoid ptr, int len )
{
	chat::login_req req;
	req.ParseFromArray( ptr, len );

	chat::login_ack ack;
	auto res = getUserMgr().UserLogin( req.user_id(), req.server(), req.nick(), req.extra() );
	if( res < 0 )
	{
		ack.set_result( res );
		Send2GameServer( chat::MSG_LOGIN_ACK, ack );
		return;
	}

	auto pUser = getUserMgr().GetUserByUID( req.user_id() );
	XGC_ASSERT( pUser );
	ack.set_result( res );
	ack.set_chat_id( pUser->handle() );
	ack.set_user_id( req.user_id() );
	ack.set_token( pUser->genToken() );

	Send2GameServer( chat::MSG_LOGIN_ACK, ack );

	auto vChannel = getChannelMgr().getChannelByWildcard< xgc::vector >( req.channel_wild() );
	for( auto pChannel : vChannel )
	{
		pChannel->Enter( pUser->handle(), xgc_nullptr );
	}
}

///
/// \brief 角色登出游戏
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::onUserLogoutReq( xgc_lpvoid ptr, int len )
{
	chat::logout_req req;
	req.ParseFromArray( ptr, len );

	getUserMgr().UserLogout( req.user_id() );
}

///
/// \brief 游戏服务器创建频道
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::onCreateChannelReq( xgc_lpvoid ptr, int len )
{
	chat::create_channel_req req;
	req.ParseFromArray( ptr, len );

	CChannel* pChannel = getChannelMgr().CreateChannel( req.channel_name() );
	auto id = pChannel ? pChannel->handle() : -1;

	chat::create_channel_ack ack;
	ack.set_channel_id( id );
	ack.set_channel_name( req.channel_name() );

	Send2GameServer( chat::MSG_CREATE_CHANNEL_ACK, ack );
}

///
/// \brief 服务器发送系统消息
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::onSystemNoteReq( xgc_lpvoid ptr, int len )
{
	chat::system_chat_req req;
	req.ParseFromArray( ptr, len );

	CChannel* pChannel = CChannel::handle_exchange( req.channel() );
	if( pChannel )
	{
		chat::system_chat_ntf ntf;
		ntf.set_channel_id( req.channel() );
		ntf.set_flags( req.flags() );
		ntf.set_text( req.text() );

		pChannel->ForEachUser( [&req, &ntf]( CChannel::user &user ){
			auto pUser = CUser::handle_exchange( user.chat_id );
			if( pUser )
				pUser->Send( chat::MSG_SYS_CHAT_NTF, ntf );
		} );
	}
}

///
/// \brief 发送数据到游戏服务器
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CServerSession::Send2GameServer( xgc_uint16 msgid, ::google::protobuf::Message & msg )
{
#pragma pack(1)
	struct
	{
		MessageHeader h;
		char b[4096];
	} m;
#pragma pack()

	xgc_uint16 data_size = msg.ByteSize();
	xgc_uint16 pack_size = sizeof( MessageHeader ) + data_size;

	m.h.length = htons( pack_size );
	m.h.message = htons( msgid );

	msg.SerializeToArray( m.b, sizeof( m.b ) );

	SendPacket( handle_, &m, pack_size );
}
