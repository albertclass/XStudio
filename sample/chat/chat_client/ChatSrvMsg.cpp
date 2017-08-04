#include "header.h"
#include "ChatSrvSession.h"

xgc_void Send2ChatSrv( CChatSrvSession* net, xgc_uint16 msgid, ::google::protobuf::Message& msg )
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
xgc_void OnChatSrvEvt( CChatSrvSession* net, xgc_uint32 event, xgc_uint64 bring )
{
	switch( event )
	{
		case EVENT_ACCEPT:
		{
			chat::user_auth_req req;
			req.set_token( "" );
			req.set_user_id( 10000 );

			Send2ChatSrv( net, chat::MSG_USERAUTH_REQ, req );
		}
		break;
		case EVENT_ERROR:
		break;
		case EVENT_CLOSE:
		SAFE_DELETE( net );
		break;
	}
}

xgc_void onUserAuthAck( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onChannelEnterNtf( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onUserInfoAck( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onChannelEnterAck( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onchannelLeaveAck( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onUserChatNtf( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onChannelChatNtf( xgc_lpcstr ptr, xgc_size len )
{

}

xgc_void onChatErr( xgc_lpcstr ptr, xgc_size len )
{

}

/// 消息回调
xgc_void OnChatSrvMsg( CChatSrvSession* net, xgc_lpvoid data, xgc_size size )
{
	MessageHeader* header = (MessageHeader*)data;
	xgc_lpstr ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	xgc_int32 len = (int)( size - sizeof( MessageHeader ) );

	switch( header->message )
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
