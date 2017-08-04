#include "config.h"
#include "base_session.h"

namespace net_module
{
	CBaseSession::CBaseSession()
		: handle_( INVALID_NETWORK_HANDLE )
		, userdata_( xgc_nullptr )
	{
	}

	CBaseSession::~CBaseSession()
	{
	}

	int CBaseSession::OnParsePacket( const void * data, xgc_size size )
	{
		if( size < sizeof(MessageHeader) )
			return 0;

		xgc_uint16 length = ntohs( *(xgc_uint16*)data );
		if( size < length )
			return 0;

		return length;
	}

	xgc_void CBaseSession::OnAccept( net::network_t handle )
	{
		handle_ = handle;
		EvtNotify( EVENT_ACCEPT, handle );
	}

	xgc_void CBaseSession::OnConnect( net::network_t handle )
	{
		handle_ = handle;
		EvtNotify( EVENT_CONNECT, handle );
	}

	xgc_void CBaseSession::OnError( xgc_uint32 error_code )
	{
		EvtNotify( EVENT_ERROR, error_code );
	}

	xgc_void CBaseSession::OnClose()
	{
		EvtNotify( EVENT_CLOSE, 0 );
	}

	xgc_void CBaseSession::OnAlive()
	{
		EvtNotify( EVENT_PING, clock() );
	}

	xgc_void CBaseSession::OnRecv( xgc_lpvoid data, xgc_size size )
	{
		MsgNotify( data, size );
	}
}