#include "header.h"
#include "gate.pb.h"
#include "GateSrvSession.h"
#include "ChatSrvSession.h"

xgc_void Send2GateSrv( CGateSrvSession* net, xgc_uint16 msgid, ::google::protobuf::Message& msg )
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
xgc_void OnGateSrvEvt( CGateSrvSession* net, xgc_uint32 event, xgc_uint64 bring )
{
	switch( event )
	{
		case EVENT_ACCEPT:
		{
			gate::login_req req;
			req.set_username( "albert.xu" );
			req.set_password( "12345566778990" );

			Send2GateSrv( net, gate::GATE_MSG_LOGIN_REQ, req );
		}
		break;
		case EVENT_ERROR:
		break;
		case EVENT_CLOSE:
		SAFE_DELETE( net );
		break;
	}
}

/// 消息回调
xgc_void OnGateSrvMsg( CGateSrvSession* net, xgc_lpvoid data, xgc_size size )
{
	MessageHeader* header = (MessageHeader*)data;
	xgc_lpstr ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	xgc_int32 len = (int)( size - sizeof( MessageHeader ) );

	switch( header->message )
	{
		case gate::GATE_MSG_LOGIN_ACK:
		{
			network_t hNet = Connect(
				"127.0.0.1",
				50001,
				NET_CONNECT_OPTION_ASYNC | NET_CONNECT_OPTION_TIMEOUT,
				1000,
				XGC_NEW CChatSrvSession() );
		}
		break;
	}
}
