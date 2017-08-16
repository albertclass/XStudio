#include "header.h"
#include "ClientSession.h"

#include "User.h"
#include "UserMgr.h"

#include "Channel.h"
#include "ChannelMgr.h"

CClientSession::CClientSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, chat_id_( -1 )
{
}

CClientSession::~CClientSession()
{
}

int CClientSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CClientSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	//fprintf( stdout, "net session %u accepted\r\n", handle_ );
}

xgc_void CClientSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	//fprintf( stdout, "net session %u connected\r\n", handle_ );
}

xgc_void CClientSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %u error, code = %u\r\n", handle_, error_code );
}

xgc_void CClientSession::OnClose()
{
	//fprintf( stderr, "net session %u closed\r\n", handle_ );
	CUser *pUser = CUser::handle_exchange( chat_id_ );
	if( pUser )
		pUser->offline();

	delete this;
}

xgc_void CClientSession::OnAlive()
{
}

xgc_void CClientSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*)data;
	auto length = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (xgc_int32)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case chat::MSG_USERAUTH_REQ: 
		onUserAuth( ptr, len ); 
		break;
		case chat::MSG_USERINFO_REQ: 
		onUserInfoReq( ptr, len ); 
		break;
		case chat::MSG_CHANNEL_ENTER_REQ: 
		onEnterChannelReq( ptr, len ); 
		break;
		case chat::MSG_CHANNEL_LEAVE_REQ: 
		onLeaveChannelReq( ptr, len );
		break;
		case chat::MSG_USER_CHAT_REQ: 
		onUserChatReq( ptr, len );
		break;
		case chat::MSG_CHANNEL_CHAT_REQ:
		onChannelChatReq( ptr, len );
		break;
	}
}

xgc_void CClientSession::onUserAuth( xgc_lpvoid ptr, int len )
{
	chat::user_auth_req req;
	if( false == req.ParseFromArray( ptr, len ) )
		return;

	chat::user_info_ack ack;
	auto pUser = getUserMgr().GetUserByUID( req.user_id() );
	if( xgc_nullptr == pUser )
	{
		ack.set_result( -1 );
	}
	else if( !pUser->checkToken( req.token() ) )
	{
		ack.set_result( -2 );
	}
	else
	{
		chat_id_ = pUser->handle();
		pUser->online( this );

		ack.set_result( 0 );
		ack.set_chat_id( pUser->handle() );
		ack.set_user_id( req.user_id() );
		ack.set_nick( pUser->getNickName() );
		ack.set_extra( pUser->getExtra() );
	}

	Send2Client( chat::MSG_USERAUTH_ACK, ack );

	// 发送已进入的频道
	pUser->forEachChannel( [this]( xgc_uint32 channel_id ){
		auto pChannel = CChannel::handle_exchange( channel_id );
		if( pChannel )
		{
			chat::channel_enter_ntf ntf;
			ntf.set_channel_id( channel_id );
			ntf.set_channel_name( pChannel->getName() );
			Send2Client( chat::MSG_CHANNEL_ENTER_NTF, ntf );
		}
	} );
}

///
/// \brief 请求用户信息
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CClientSession::onUserInfoReq( xgc_lpvoid ptr, int len )
{
	chat::user_info_req req;
	if( false == req.ParseFromArray( ptr, len ) )
		return;

	chat::user_info_ack ack;
	auto pUser = getUserMgr().GetUserByUID( req.user_id() );
	if( xgc_nullptr == pUser )
	{
		ack.set_result( -1 );
	}
	else
	{
		ack.set_result( 0 );
		ack.set_chat_id( pUser->handle() );
		ack.set_user_id( req.user_id() );
		ack.set_nick( pUser->getNickName() );
		ack.set_extra( pUser->getExtra() );
	}

	Send2Client( chat::MSG_USERINFO_ACK, ack );
}

///
/// \brief 请求进入频道
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CClientSession::onEnterChannelReq( xgc_lpvoid ptr, int len )
{
	chat::channel_enter_req req;
	if( false == req.ParseFromArray( ptr, len ) )
		return;

	XGC_ASSERT_RETURN( chat_id_ != CUser::INVALID_OBJECT_ID, XGC_NONE );

	auto type = req.type();

	switch( type )
	{
		case chat::channel_enter_req::WILDCARD:
		{
			/// 根据通配符进入频道
			auto vChannel = getChannelMgr().getChannelByWildcard< xgc_vector >( req.channel_name() );

			for( auto pChannel : vChannel )
			{
				chat::channel_enter_ack ack;

				ack.set_token( req.token() );
				ack.set_channel_id( pChannel->handle() );
				ack.set_channel_name( pChannel->getName() );

				auto res = pChannel->Enter( chat_id_, xgc_nullptr );
				ack.set_result( res );

				Send2Client( chat::MSG_CHANNEL_ENTER_ACK, ack );
			};
		}
		break;
		case chat::channel_enter_req::CHANNEL_NAME:
		{
			auto pChannel = getChannelMgr().getChannelByName( req.channel_name() );
			if( pChannel )
			{
				chat::channel_enter_ack ack;

				ack.set_token( req.token() );
				ack.set_channel_id( pChannel->handle() );
				ack.set_channel_name( pChannel->getName() );

				auto res = pChannel->Enter( chat_id_, xgc_nullptr );
				ack.set_result( res );

				Send2Client( chat::MSG_CHANNEL_ENTER_ACK, ack );
			}
		}
		break;
		case chat::channel_enter_req::CHANNEL_ID:
		{
			auto pChannel = CChannel::handle_exchange( req.channel_id() );
			if( pChannel )
			{
				chat::channel_enter_ack ack;

				ack.set_token( req.token() );
				ack.set_channel_id( pChannel->handle() );
				ack.set_channel_name( pChannel->getName() );

				auto res = pChannel->Enter( chat_id_, xgc_nullptr );
				ack.set_result( res );

				Send2Client( chat::MSG_CHANNEL_ENTER_ACK, ack );
			}
		}
		break;
	}
}

///
/// \brief 请求离开频道
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CClientSession::onLeaveChannelReq( xgc_lpvoid ptr, int len )
{
	chat::channel_leave_req req;
	if( false == req.ParseFromArray( ptr, len ) )
		return;

	XGC_ASSERT_RETURN( chat_id_ != CUser::INVALID_OBJECT_ID, XGC_NONE );

	chat::channel_leave_ack ack;
	ack.set_channel_id( req.channel_id() );

	auto pChannel = CChannel::handle_exchange( req.channel_id() );
	if( pChannel )
	{
		ack.set_channel_name( pChannel->getName() );
		ack.set_result( 0 );
		pChannel->Leave( chat_id_ );
	}
	else
	{
		ack.set_channel_name( "" );
		ack.set_result( -1 );
	}

	Send2Client( chat::MSG_CHANNEL_LEAVE_ACK, ack );
}

///
/// \brief 用户聊天请求
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CClientSession::onUserChatReq( xgc_lpvoid ptr, int len )
{
	chat::user_chat_req req;
	if( false == req.ParseFromArray( ptr, len ) )
		return;

	XGC_ASSERT_RETURN( chat_id_ != CUser::INVALID_OBJECT_ID, XGC_NONE );

	auto pChatTo = CUser::handle_exchange( req.chat_id() );
	if( xgc_nullptr == pChatTo )
	{
		chat::chat_err err;
		err.set_result( -1 );
		err.set_token( req.token() );
		err.set_description( "user is not exist." );

		Send2Client( chat::MSG_CHAT_ERR, err );
	}
	else
	{
		chat::user_chat_ntf ntf;
		ntf.set_token( req.token() );
		ntf.set_chat_id( chat_id_ );
		ntf.set_text( req.text() );

		pChatTo->Send( chat::MSG_USER_CHAT_NTF, ntf );
		Send2Client( chat::MSG_USER_CHAT_NTF, ntf );
	}
}

///
/// \brief 用户频道发言请求
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CClientSession::onChannelChatReq( xgc_lpvoid ptr, int len )
{
	chat::channel_chat_req req;
	if( false == req.ParseFromArray( ptr, len ) )
		return;

	XGC_ASSERT_RETURN( chat_id_ != CUser::INVALID_OBJECT_ID, XGC_NONE );

	auto pChannel = CChannel::handle_exchange( req.channel_id() );
	if( xgc_nullptr == pChannel )
	{
		chat::chat_err err;
		err.set_result( -1 );
		err.set_token( req.token() );
		err.set_description( "channel is not exist." );

		Send2Client( chat::MSG_CHAT_ERR, err );
	}
	else
	{
		// 玩家发言
		chat::channel_chat_ntf ntf;

		if( false == pChannel->isValidUser( chat_id_ ) )
		{
			chat::chat_err err;
			err.set_result( -1 );
			err.set_token( req.token() );
			err.set_description( "not valid user." );

			Send2Client( chat::MSG_CHAT_ERR, err );
			return;
		}
		
		ntf.set_token( req.token() );
		ntf.set_channel_id( pChannel->handle() );
		ntf.set_text( req.text() );
		ntf.set_chat_id( chat_id_ );

		pChannel->ForEachUser( [this, &ntf]( CChannel::user& user ){
			auto pUser = CUser::handle_exchange( user.chat_id );
			if( pUser )
			{
				pUser->Send( chat::MSG_CHANNEL_CHAT_REQ, ntf );
			}
		} );
	}
}


///
/// \brief 发送数据到客户端
///
/// \author albert.xu
/// \date 2017/03/27 11:36
///
xgc_void CClientSession::Send2Client( xgc_uint16 msgid, ::google::protobuf::Message & msg )
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
