#include "header.h"
#include "SrvSession.h"
/// 事件回调
extern xgc_void OnServerEvt( CServerSession* net, xgc_uint32 event, xgc_uint64 code );
/// 消息回调
extern xgc_void OnServerMsg( CServerSession* net, xgc_lpvoid data, xgc_size size );

CServerSession::CServerSession()
	: handle_( INVALID_NETWORK_HANDLE )
	, pingpong_( 0 )
	, pinglast_( ticks< std::chrono::milliseconds >() )
	, userdata_( xgc_nullptr )
{
}

CServerSession::~CServerSession()
{
}

int CServerSession::OnParsePacket( const void * data, xgc_size size )
{
	if( size < sizeof( MessageHeader ) )
		return 0;

	xgc_uint16 length = ntohs( *(xgc_uint16*) data );
	if( size < length )
		return 0;

	return length;
}

xgc_void CServerSession::OnAccept( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u accepted\r\n", handle_ );

	OnServerEvt( this, EVENT_ACCEPT, handle_ );
}

xgc_void CServerSession::OnConnect( net::network_t handle )
{
	handle_ = handle;
	fprintf( stdout, "net session %u connected\r\n", handle_ );

	OnServerEvt( this, EVENT_CONNECT, handle_ );
}

xgc_void CServerSession::OnError( xgc_uint32 error_code )
{
	fprintf( stderr, "net session %u error, code = %u\r\n", handle_, error_code );
	OnServerEvt( this, EVENT_ERROR, error_code );
}

xgc_void CServerSession::OnClose()
{
	fprintf( stderr, "net session %u closed\r\n", handle_ );
	OnServerEvt( this, EVENT_CLOSE, 0 );
}

xgc_void CServerSession::OnAlive()
{
	OnServerEvt( this, EVENT_TIMER, handle_ );
}

xgc_void CServerSession::OnRecv( xgc_lpvoid data, xgc_size size )
{
	MessageHeader &header = *(MessageHeader*) data;

	OnServerMsg( this, data, ntohs( header.length ) );
}