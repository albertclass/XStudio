#include "header.h"
#include "session.h"
/// 事件回调
extern xgc_void OnEvent( CNetSession* net, xgc_uint32 event, xgc_uint64 code );
/// 消息回调
extern xgc_void OnMessage( CNetSession* net, xgc_lpvoid data, xgc_size size );

CNetSession::CNetSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, userdata_( xgc_nullptr )
{
}

CNetSession::~CNetSession()
{
}

int CNetSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CNetSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );

	OnEvent( this, EVENT_ACCEPT, handle_ );
}

xgc_void CNetSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );

	OnEvent( this, EVENT_CONNECT, handle_ );
}

xgc_void CNetSession::OnError( xgc_int16 error_type, xgc_int16 error_code )
{
	fprintf( stderr, "net session %u error, code = %u\r\n", handle_, error_code );
	OnEvent( this, EVENT_ERROR, error_code );
}

xgc_void CNetSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	OnEvent( this, EVENT_CLOSE, 0 );
}

xgc_void CNetSession::OnAlive()
{
	MessageHeader header;
	header.length = htons( sizeof( MessageHeader ) + sizeof( MessagePing ) );
	header.type = SOCKET_MESSAGE_TYPE;
	header.code = EVENT_PING;

	MessagePing ping;
	ping.timestamp = htonll( ticks< std::chrono::milliseconds >() );

	net::SendPacketChains( handle_, net::BufferChains {
		std::make_tuple( &header, sizeof( header ) ),
		std::make_tuple( &ping, sizeof( ping ) ),
	} );

	if( ticks< std::chrono::milliseconds >() - pingpong_ > 5000 )
		net::CloseLink( handle_ );
}

xgc_void CNetSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*) data;

	if( header.type == SOCKET_MESSAGE_TYPE )
	{
		switch( header.code )
		{
		case EVENT_PING:
			{
				MessageHeader header;
				header.length = htons( sizeof( MessageHeader ) + sizeof( MessagePing ) );
				header.type = SOCKET_MESSAGE_TYPE;
				header.code = EVENT_PONG;

				MessagePing &ping = *(MessagePing*) ((unsigned char*) data + sizeof( MessageHeader ));

				net::SendPacketChains( handle_, net::BufferChains {
					std::make_tuple( &header, sizeof( header ) ),
					std::make_tuple( &ping, sizeof( ping ) ),
				} );
			}
			break;
		case EVENT_PONG:
			{
				MessagePing &ping = *(MessagePing*) ((unsigned char*) data + sizeof( MessageHeader ));
				xgc_time64 timestamp = ntohll( ping.timestamp );
				pinglast_ = ticks< std::chrono::milliseconds >();
				pingpong_ = (xgc_ulong) (pinglast_ - timestamp);
			}
			break;
		}
	}
	else
	{
		OnMessage( this, data, ntohs( header.length ) );
	}
}

xgc_void CNetSession::SendPacket( xgc_lpvoid data, xgc_size size )
{
	net::SendPacket( handle_, data, size );
}
