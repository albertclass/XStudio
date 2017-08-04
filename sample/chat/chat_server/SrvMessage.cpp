#include "header.h"
#include "SrvSession.h"
#include "User.h"
#include "UserMgr.h"
#include "Channel.h"
#include "ChannelMgr.h"

xgc_void SendServerPacket( CServerSession* net, xgc_uint16 msgid, ::google::protobuf::Message& msg )
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
	m.h.message = htons( chat::MSG_LOGIN_ACK );

	msg.SerializeToArray( m.b, sizeof( m.b ) );

	SendPacket( net->GetHandle(), &m, pack_size );
}

/// 事件回调
xgc_void OnServerEvt( CServerSession* net, xgc_uint32 event, xgc_uint64 bring )
{
	switch( event )
	{
		case EVENT_ACCEPT:
		break;
		case EVENT_ERROR:
		break;
		case EVENT_CLOSE:
		SAFE_DELETE( net );
		break;
	}
}

/// 消息回调
xgc_void OnServerMsg( CServerSession* net, xgc_lpvoid data, xgc_size size )
{
	MessageHeader* header = (MessageHeader*)data;
	xgc_lpstr ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	xgc_int32 len = (int)(size - sizeof( MessageHeader ));

	switch( header->message )
	{
		case chat::MSG_LOGIN_REQ:
		{
			chat::login_req req;
			req.ParseFromArray( ptr, len );

			chat::login_ack ack;
			auto res = getUserMgr().UserLogin( req.user_id(), req.server(), req.nick(), req.extra() );
			if( res < 0 )
			{
				ack.set_result( res );
				SendServerPacket( net, chat::MSG_LOGIN_ACK, ack );
				break;
			}

			auto pUser = getUserMgr().GetUserByUID( req.user_id() );
			XGC_ASSERT( pUser );
			ack.set_result( res );
			ack.set_chat_id( pUser->handle() );
			ack.set_user_id( req.user_id() );
			ack.set_token( pUser->genToken() );

			SendServerPacket( net, chat::MSG_LOGIN_ACK, ack );

			auto vChannel = getChannelMgr().getChannelByWildcard< xgc_vector >( req.channel_wild() );
			for( auto pChannel : vChannel )
			{
				pChannel->Enter( pUser->handle(), xgc_nullptr );
			}
		}
		break;
		case chat::MSG_LOGOUT_REQ:
		{
			chat::logout_req req;
			req.ParseFromArray( ptr, len );

			getUserMgr().UserLogout( req.user_id() );
		}
		break;
		case chat::MSG_CREATE_CHANNEL_REQ:
		{
			chat::create_channel_req req;
			req.ParseFromArray( data, len );

			CChannel* pChannel = getChannelMgr().CreateChannel( req.channel_name() );
			auto id = pChannel ? pChannel->handle() : -1;

			chat::create_channel_ack ack;
			ack.set_channel_id( id );
			ack.set_channel_name( req.channel_name() );
			
			SendServerPacket( net, chat::MSG_CREATE_CHANNEL_ACK, ack );
		}
		break;
		case chat::MSG_SYS_CHAT_REQ:
		{
			chat::system_chat_req req;
			req.ParseFromArray( data, len );

			CChannel* pChannel = CChannel::handle_exchange( req.channel() );
			if( pChannel )
			{
				chat::system_chat_ntf ntf;
				ntf.set_channel_id( req.channel() );
				ntf.set_flags( req.flags() );
				ntf.set_text( req.text() );

				pChannel->ForEachUser( [&req, &ntf]( xgc_uint32 nChatID, xgc_bool forbid ){
					auto pUser = CUser::handle_exchange( nChatID );
					if( pUser )
					{
						pUser->Send( chat::MSG_SYS_CHAT_NTF, ntf );
					}
				} );
			}
		}
		break;
	}
}
