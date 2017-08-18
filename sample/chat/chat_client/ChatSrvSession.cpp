#include "header.h"
#include "ChatSrvSession.h"
#include "GameSrvSession.h"

CChatSrvSession::CChatSrvSession( CGameSrvSession *session, xgc_uint64 user_id, const xgc_string &token )
	: user_id_( user_id )
	, chat_id_( -1 )
	, token_( token )
	, handle_( INVALID_NETWORK_HANDLE )
	, game_session_( session )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, chat_token_( 0xFFFFFFFF )
{
}

CChatSrvSession::~CChatSrvSession()
{
	game_session_ = xgc_nullptr;
}

int CChatSrvSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CChatSrvSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	//fprintf( stdout, "net session %u accepted\r\n", handle_ );
}

xgc_void CChatSrvSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	//fprintf( stdout, "net session %u connected\r\n", handle_ );

	game_session_->OnChatConnect( handle );
}

xgc_void CChatSrvSession::OnError( xgc_int16 error_type, xgc_int16 error_code )
{
	fprintf( stderr, "chat session %0X error, type = %d, code = %d\r\n", handle_, error_type, error_code );
	game_session_->OnChatError( error_type, error_code );
}

xgc_void CChatSrvSession::OnClose()
{
	//fprintf( stderr, "chat session %0X closed\r\n", handle_ );
	game_session_->OnChatClose();

	delete this;
}

xgc_void CChatSrvSession::OnAlive()
{
}

xgc_void CChatSrvSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*) data;
	auto length  = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (xgc_long)( size - sizeof( MessageHeader ) );

	if( false == game_session_->OnChatMsg( message, ptr, len ) )
	{
		switch( message )
		{
			case chat::MSG_USERAUTH_ACK:
			onUserAuthAck( ptr, len );
			break;
			case chat::MSG_CHANNEL_ENTER_NTF:
			onChannelEnterNtf( ptr, len );
			break;
			case chat::MSG_USERINFO_ACK:
			onUserInfoAck( ptr, len );
			break;
			case chat::MSG_CHANNEL_ENTER_ACK:
			onChannelEnterAck( ptr, len );
			break;
			case chat::MSG_CHANNEL_LEAVE_ACK:
			onchannelLeaveAck( ptr, len );
			break;
			case chat::MSG_USER_CHAT_NTF:
			onUserChatNtf( ptr, len );
			break;
			case chat::MSG_CHANNEL_CHAT_NTF:
			onChannelChatNtf( ptr, len );
			break;
			case chat::MSG_CHAT_ERR:
			onChatErr( ptr, len );
			break;
		}
	}
}

///
/// \brief 聊天服务器认证
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::ChatUserAuth()
{
	chat::user_auth_req req;
	req.set_token( token_ );
	req.set_user_id( user_id_ );

	Send2ChatSrv( chat::MSG_USERAUTH_REQ, req );
}

///
/// \brief 请求用户信息
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::ChatUserInfoReq( xgc_uint32 chat_id, xgc_uint64 user_id )
{
	chat::user_info_req req;
	req.set_chat_id( chat_id );
	req.set_user_id( user_id );

	Send2ChatSrv( chat::MSG_USERINFO_REQ, req );
}

///
/// \brief 密语
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::ChatTo( xgc_uint32 chat_id, xgc_lpcstr text )
{
	char data[4096], *utf8 = data;

	size_t size = sizeof( data );
	size_t utf8_len = mbs_to_utf8( text, xgc_nullptr, 0 );

	chat::user_chat_req req;
	req.set_token( ++chat_token_ );
	req.set_chat_id( chat_id );

	if( !( utf8_len < size - 1 ) )
	{
		utf8 = (char*)malloc( utf8_len + 1 );
		size = memsize( data );
	}

	if( -1 == mbs_to_utf8( text, utf8, size ) )
		return;
	
	utf8[utf8_len] = '\0';

	req.set_text( utf8, utf8_len );

	Send2ChatSrv( chat::MSG_USER_CHAT_REQ, req );

	free( data != utf8 ? utf8 : xgc_nullptr );
}

///
/// \brief 发言
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::Say( xgc_uint32 channel_id, xgc_lpcstr text )
{
	char data[4096], *utf8 = data;

	size_t size = sizeof( data );
	size_t utf8_len = mbs_to_utf8( text, xgc_nullptr, 0 );

	chat::channel_chat_req req;
	req.set_token( ++chat_token_ );
	req.set_channel_id( channel_id );

	if( !( utf8_len < size - 1 ) )
	{
		utf8 = (char*)malloc( utf8_len + 1 );
		size = memsize( data );
	}

	if( -1 == mbs_to_utf8( text, utf8, size ) )
		return;

	utf8[utf8_len] = '\0';

	req.set_text( utf8, utf8_len );

	Send2ChatSrv( chat::MSG_CHANNEL_CHAT_REQ, req );

	free( data != utf8 ? utf8 : xgc_nullptr );
}

///
/// \brief 用户认证回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onUserAuthAck( xgc_lpcstr ptr, xgc_size len )
{
	chat::user_auth_ack ack;
	ack.ParseFromArray( ptr, (int)len );
	if( ack.result() < 0 )
	{
		CloseLink( GetHandle() );
		return;
	}

	chat_id_ = ack.chat_id();
	nickname_ = ack.nick();
	extra_ = ack.extra();
}

///
/// \brief 已进入频道回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChannelEnterNtf( xgc_lpcstr ptr, xgc_size len )
{
	chat::channel_enter_ntf ntf;
	ntf.ParseFromArray( ptr, (int)len );

	auto channel_id = ntf.channel_id();
	auto channel_name = ntf.channel_name();

	auto ib = channels_.insert( { channel_id, channel_name } );
	XGC_ASSERT( ib.second );
}

///
/// \brief 用户信息回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onUserInfoAck( xgc_lpcstr ptr, xgc_size len )
{
	chat::user_info_ack ack;
	ack.ParseFromArray( ptr, (int)len );

	auto it = users_.find( ack.chat_id() );
	if( it != users_.end() )
	{
		auto &user = it->second;
		user.user_id = ack.user_id();
		user.chat_id = ack.chat_id();
		user.nick = ack.nick();
		user.extra = ack.extra();
	}
	else
	{
		user_info user;
		user.user_id = ack.user_id();
		user.chat_id = ack.chat_id();
		user.nick = ack.nick();
		user.extra = ack.extra();

		users_.emplace( std::make_pair( user.chat_id, std::move( user ) ) );

		auto it2 = pending_chat_.find( ack.chat_id() );
		if( it2 != pending_chat_.end() )
		{
			auto chat_id = it2->first;
			auto &list = it2->second;
			for( auto &chat : list )
			{
				showText( chat.timestamp, chat_id, chat.channel_id, chat.text.c_str() );
			}

			pending_chat_.erase( it2 );
		}
	}
}

///
/// \brief 进入频道回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChannelEnterAck( xgc_lpcstr ptr, xgc_size len )
{
	chat::channel_enter_ack ack;
	ack.ParseFromArray( ptr, (int)len );

	auto channel_id = ack.channel_id();
	auto channel_name = ack.channel_name();

	auto ib = channels_.insert( { channel_id, channel_name } );
	XGC_ASSERT( ib.second );
}

///
/// \brief 离开频道回应
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onchannelLeaveAck( xgc_lpcstr ptr, xgc_size len )
{
	chat::channel_leave_ack ack;
	ack.ParseFromArray( ptr, (int)len );
	if( ack.result() < 0 )
		return;

	channels_.erase( ack.channel_id() );
}

///
/// \brief 用户聊天内容通知
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onUserChatNtf( xgc_lpcstr ptr, xgc_size len )
{
	chat::user_chat_ntf ntf;
	ntf.ParseFromArray( ptr, (int)len );

	auto chat_id = ntf.chat_id();
	auto text = ntf.text();

	auto it = users_.find( chat_id );
	if( it != users_.end() )
	{
		showText( datetime::now().to_milliseconds(), chat_id, -1, text.c_str() );
	}
	else
	{
		chat_info chat;
		chat.timestamp = current_milliseconds();
		chat.channel_id = -1;
		chat.text = std::move( text );

		pending_chat_[chat_id].emplace_back( std::move( chat ) );
	}
}

///
/// \brief 频道聊天通知
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChannelChatNtf( xgc_lpcstr ptr, xgc_size len )
{
	chat::channel_chat_ntf ntf;
	auto channel_id = ntf.channel_id();
	auto chat_id = ntf.chat_id();
	auto text = ntf.text();

	auto it1 = users_.find( chat_id );
	if( it1 == users_.end() )
	{
		chat_info chat;
		chat.timestamp = current_milliseconds();
		chat.channel_id = channel_id;
		chat.text = std::move( text );

		pending_chat_[chat_id].emplace_back( std::move( chat ) );
	}
	else
	{
		showText( current_milliseconds(), chat_id, channel_id, text.c_str() );
	}
}

///
/// \brief 聊天错误通知
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::onChatErr( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void CChatSrvSession::showText( xgc_time64 timestamp, xgc_uint32 user_id, xgc_uint32 channel_id, xgc_lpcstr text )
{
	auto say_time = datetime::from_milliseconds( timestamp );
	char say_time_buf[128];
	say_time.to_string( say_time_buf );

	auto it1 = users_.find( user_id );
	if( it1 == users_.end() )
		return;

	if( channel_id == -1 )
	{
		// 私聊
		printf( "[%s]：%s[%s]\n", it1->second.nick.c_str(), text, say_time_buf );
	}
	else
	{
		auto it2 = channels_.find( channel_id );
		if( it2 != channels_.end() )
		{
			// 频道消息
			printf( "[%s-%s]：%s[%s]\n", it2->second.c_str(), it1->second.nick.c_str(), text, say_time_buf );
		}
		else
		{
			printf( "收到未知频道的消息 channel - %u, text - %s", channel_id, text );
		}
	}
}

///
/// \brief 发送消息到Chat服务器
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg )
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

///
/// \brief 定时发送消息到聊天服务器
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_void CChatSrvSession::OnTimer()
{
	// net::NewTimer( handle_, 1.0f, 1.0f, std::bind( &CChatSrvSession::OnTimer, this ) );

	auto channel_idx = random_range( (size_t)0, channels_.size() );

	auto it = channels_.begin();
	while( channel_idx )
	{
		if( it == channels_.end() )
			return;

		++it;
		--channel_idx;
	}

	if( it != channels_.end() )
	{
		Say( it->first, "hahaha hahahahahaha 哈哈哈， 哈哈哈哈哈哈哈！" );
	}
}

///
/// \brief 断开聊天服务器连接
///
/// \author albert.xu
/// \date 2017/08/09
///
xgc_void CChatSrvSession::Disconnect()
{
	if( handle_ != INVALID_NETWORK_HANDLE )
		CloseLink( handle_ );
}
