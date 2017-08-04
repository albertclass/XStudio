#include "header.h"
#include "CliSession.h"
#include "User.h"
#include "UserMgr.h"

#include "Channel.h"
#include "ChannelMgr.h"

xgc_void SendClientPacket( CClientSession* net, xgc_uint16 msgid, ::google::protobuf::Message& msg )
{
#pragma pack(1)
	struct
	{
		MessageHeader h;
		char b[1024*4];
	} m;
#pragma pack()

	xgc_uint16 data_size = msg.ByteSize();
	xgc_uint16 pack_size = sizeof( MessageHeader ) + data_size;

	m.h.length = htons( pack_size );
	m.h.message = htons( chat::MSG_LOGIN_ACK );

	msg.SerializeToArray( m.b, sizeof( m.b ) );

	SendPacket( net->GetHandle(), &m, pack_size );
}

/// 事件回调
xgc_void OnClientEvt( CClientSession* net, xgc_uint32 event, xgc_uint64 bring )
{
	switch( event )
	{
		case EVENT_ACCEPT:
		break;
		case EVENT_ERROR:
		break;
		case EVENT_CLOSE:
		{
			CUser *pUser = CUser::handle_exchange( net->getChatID() );
			if( pUser )
				pUser->offline();

			SAFE_DELETE( net );
		}
		break;
	}
}

/// 消息回调
xgc_void OnClientMsg( CClientSession* net, xgc_lpvoid data, xgc_size size )
{
	MessageHeader* header = (MessageHeader*)data;
	xgc_lpstr ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	xgc_int32 len = (int)( size - sizeof( MessageHeader ) );

	switch( header->message )
	{
		case chat::MSG_USERAUTH_REQ:
		{
			chat::user_auth_req req;
			if( false == req.ParseFromArray( ptr, len ) )
				break;

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
				net->setChatID( pUser->handle() );
				pUser->online( net->GetHandle() );

				ack.set_result( 0 );
				ack.set_chat_id( pUser->handle() );
				ack.set_user_id( req.user_id() );
				ack.set_nick( pUser->getNickName() );
				ack.set_extra( pUser->getExtra() );
			}

			SendClientPacket( net, chat::MSG_USERAUTH_ACK, ack );

			// 发送已进入的频道
			pUser->forEachChannel( [net]( xgc_uint32 channel_id ){
				auto pChannel = CChannel::handle_exchange( channel_id );
				if( pChannel )
				{
					chat::channel_enter_ntf ntf;
					ntf.set_channel_id( channel_id );
					ntf.set_channel_name( pChannel->getName() );
					SendClientPacket( net, chat::MSG_CHANNEL_ENTER_NTF, ntf );
				}
			} );
		}
		break;
		case chat::MSG_USERINFO_REQ:
		{
			chat::user_info_req req;
			if( false == req.ParseFromArray( ptr, len ) )
				break;

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

			SendClientPacket( net, chat::MSG_USERINFO_ACK, ack );
		}
		break;
		case chat::MSG_CHANNEL_ENTER_REQ:
		{
			chat::channel_enter_req req;
			if( false == req.ParseFromArray( ptr, len ) )
				break;

			auto chat_id = net->getChatID();
			XGC_ASSERT_BREAK( chat_id != CUser::INVALID_OBJECT_ID );

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

						auto res = pChannel->Enter( chat_id, xgc_nullptr );
						ack.set_result( res );

						SendClientPacket( net, chat::MSG_CHANNEL_ENTER_ACK, ack );
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

						auto res = pChannel->Enter( chat_id, xgc_nullptr );
						ack.set_result( res );

						SendClientPacket( net, chat::MSG_CHANNEL_ENTER_ACK, ack );
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

						auto res = pChannel->Enter( chat_id, xgc_nullptr );
						ack.set_result( res );

						SendClientPacket( net, chat::MSG_CHANNEL_ENTER_ACK, ack );
					}
				}
				break;
			}
		}
		break;
		case chat::MSG_CHANNEL_LEAVE_REQ:
		{
			chat::channel_leave_req req;
			if( false == req.ParseFromArray( ptr, len ) )
				break;

			auto chat_id = net->getChatID();
			XGC_ASSERT_BREAK( chat_id != CUser::INVALID_OBJECT_ID );

			chat::channel_leave_ack ack;
			ack.set_channel_id( req.channel_id() );

			auto pChannel = CChannel::handle_exchange( req.channel_id() );
			if( pChannel )
			{
				ack.set_channel_name( pChannel->getName() );
				ack.set_result( 0 );
				pChannel->Leave( chat_id );
			}
			else
			{
				ack.set_channel_name("");
				ack.set_result( -1 );
			}

			SendClientPacket( net, chat::MSG_CHANNEL_LEAVE_ACK, ack );
		}
		break;
		case chat::MSG_USER_CHAT_REQ:
		{
			chat::user_chat_req req;
			if( false == req.ParseFromArray( ptr, len ) )
				break;

			auto chat_id = net->getChatID();
			XGC_ASSERT_BREAK( chat_id != CUser::INVALID_OBJECT_ID );

			auto pChatTo = CUser::handle_exchange( req.chat_id() );
			if( xgc_nullptr == pChatTo )
			{
				chat::chat_err err;
				err.set_result( -1 );
				err.set_token( req.token() );
				err.set_description( "user is not exist." );

				SendClientPacket( net, chat::MSG_CHAT_ERR, err );
			}
			else
			{
				chat::user_chat_ntf ntf;
				ntf.set_token( req.token() );
				ntf.set_chat_id( chat_id );
				ntf.set_text( req.text() );

				pChatTo->Send( chat::MSG_USER_CHAT_NTF, ntf );
				SendClientPacket( net, chat::MSG_USER_CHAT_NTF, ntf );
			}
		}
		break;
		case chat::MSG_CHANNEL_CHAT_REQ:
		{
			chat::channel_chat_req req;
			if( false == req.ParseFromArray( ptr, len ) )
				break;

			auto chat_id = net->getChatID();
			XGC_ASSERT_BREAK( chat_id != CUser::INVALID_OBJECT_ID );

			auto pChannel = CChannel::handle_exchange( req.channel_id() );
			if( xgc_nullptr == pChannel )
			{
				chat::chat_err err;
				err.set_result( -1 );
				err.set_token( req.token() );
				err.set_description( "channel is not exist." );

				SendClientPacket( net, chat::MSG_CHAT_ERR, err );
			}
			else
			{
				// 玩家发言
				auto res = pChannel->Chat( chat_id, req.text().c_str(), req.text().size(), req.token() );
				if( res != 0 )
				{
					chat::chat_err err;
					err.set_result( res );
					err.set_token( req.token() );
					err.set_description( "channel chat error." );

					SendClientPacket( net, chat::MSG_CHAT_ERR, err );
				}
			}

		}
		break;
	}
}
