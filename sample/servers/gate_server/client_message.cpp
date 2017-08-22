#include "header.h"
#include "client_message.h"
#include "cli_session.h"

/// �¼��ص�
xgc_void OnClientEvt( CClientSession* net, xgc_uint32 event, xgc_uint64 code )
{
	switch( event )
	{
	case EVENT_CLOSE:
		fprintf( stderr, "connection %p disconnect.\n", net );
		SAFE_DELETE( net );
		break;
	}
}

/// ��Ϣ�ص�
xgc_void OnClientMsg( CClientSession* net, xgc_lpvoid data, xgc_size size, xgc_uint32 trans )
{
	auto &header = *(MessageHeader*)data;
	switch( header.type )
	{
	case NET_GATE_MESSAGE:
		OnGateMessage( net, header.code, data, size );
		break;
	case NET_GAME_MESSAGE:
		OnGameMessage( net, header.code, data, size );
		break;
	}
}

xgc_void OnGateMessage( CClientSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{

}

xgc_void OnGameMessage( CClientSession* net, xgc_uint8 code, xgc_lpvoid data, xgc_size size )
{
	RelayPacket( net, data, size );
}