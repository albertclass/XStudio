#include "header.h"
#include "GateSrvSession.h"

/// 事件回调
extern xgc_void OnGateSrvEvt( CGateSrvSession* net, xgc_uint32 event, xgc_uint64 code );
/// 消息回调
extern xgc_void OnGateSrvMsg( CGateSrvSession* net, xgc_lpvoid data, xgc_size size );

CGateSrvSession::CGateSrvSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, chat_id_( -1 )
{
}

CGateSrvSession::~CGateSrvSession()
{
}

int CGateSrvSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CGateSrvSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );

	OnGateSrvEvt( this, EVENT_ACCEPT, handle_ );
}

xgc_void CGateSrvSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );

	OnGateSrvEvt( this, EVENT_CONNECT, handle_ );
}

xgc_void CGateSrvSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %u error, code = %u\r\n", handle_, error_code );
	OnGateSrvEvt( this, EVENT_ERROR, error_code );
}

xgc_void CGateSrvSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	OnGateSrvEvt( this, EVENT_CLOSE, 0 );
}

xgc_void CGateSrvSession::OnAlive()
{
	OnGateSrvEvt( this, EVENT_TIMER, handle_ );
}

xgc_void CGateSrvSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*) data;

	OnGateSrvMsg( this, data, ntohs( header.length ) );
}
