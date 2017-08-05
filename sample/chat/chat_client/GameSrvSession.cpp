#include "header.h"
#include "gate.pb.h"

#include "GameSrvSession.h"
#include "ChatSrvSession.h"

CGameSrvSession::CGameSrvSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, userdata_( xgc_nullptr )
{
}

CGameSrvSession::~CGameSrvSession()
{
}

int CGameSrvSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CGameSrvSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );
}

xgc_void CGameSrvSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );

	gate::login_req req;
	req.set_username( "albert.xu" );
	req.set_password( "12345566778990" );

	Send2GateSrv( gate::GATE_MSG_LOGIN_REQ, req );
}

xgc_void CGameSrvSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %0X error, code = %0X\r\n", handle_, error_code );
}

xgc_void CGameSrvSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	auto pChatSession = (CChatSrvSession*)getUserdata();
	if( pChatSession )
		pChatSession->GateSrvClosed();

	delete this;
}

xgc_void CGameSrvSession::OnAlive()
{
}

xgc_void CGameSrvSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*)data;
	auto length = htons( header.length );
	auto message = htons( header.message );

	auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
	auto len = (int)( size - sizeof( MessageHeader ) );

	switch( message )
	{
		case gate::GATE_MSG_LOGIN_ACK:
		{
			chat::login_ack ack;
			ack.ParseFromArray( ptr, len );

			if( ack.result() >= 0 )
			{
				CChatSrvSession* pChatSrvSession = XGC_NEW CChatSrvSession( handle_, ack.user_id(), ack.chat_id(), ack.token() );
				if( pChatSrvSession )
				{
					setUserdata( pChatSrvSession );

					Connect(
						"127.0.0.1",
						50001,
						NET_CONNECT_OPTION_ASYNC | NET_CONNECT_OPTION_TIMEOUT,
						1000,
						pChatSrvSession );
				}
			}
			else
			{
				CloseLink( handle_ );
			}
		}
		break;
	}
}

xgc_void CGameSrvSession::Send2GateSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg )
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
