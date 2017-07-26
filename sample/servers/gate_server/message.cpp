#include "header.h"
#include "message.h"
#include "session.h"

/// 事件回调
xgc_void OnEvent( CNetSession* net, xgc_uint32 event, xgc_uint32 code )
{
	switch( event )
	{
	case EVENT_CLOSE:
		fprintf( stderr, "connection %u disconnect.\n", net->GetHandle() );
		SAFE_DELETE( net );
		break;
	}
}

/// 消息回调
xgc_void OnMessage( CNetSession* net, xgc_uint8 type, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{
	switch( type )
	{
	case NET_GATE_MESSAGE:
		OnGateMessage( net, code, data, size );
		break;
	case NET_GAME_MESSAGE:
		OnGameMessage( net, code, data, size );
		break;
	}
}

xgc_void OnGateMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{

}

xgc_void OnGameMessage( CNetSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{

}