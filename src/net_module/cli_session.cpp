#include "config.h"
#include "cli_session.h"
#include "relay_session.h"

namespace net_module
{
	extern ClientMsgParser  pfn_ClientMsgParser;
	extern ClientMsgHandler pfn_ClientMsgHandler;
	extern ClientEvtHandler pfn_ClientEvtHandler;

	CClientSession::CClientSession()
		: CBaseSession()
		, mRelaySession( xgc_nullptr )
	{
	}

	CClientSession::~CClientSession()
	{
	}

	xgc_void CClientSession::Relay( xgc_lpvoid data, xgc_size size ) const
	{
		if( mRelaySession )
			mRelaySession->Send( data, size );
	}

	int CClientSession::OnParsePacket( const void * data, xgc_size size )
	{
		return pfn_ClientMsgParser( data, size );
	}

	xgc_void CClientSession::EvtNotify( xgc_uint32 event, xgc_uint32 result )
	{
		pfn_ClientEvtHandler( this, event, result );
	}

	xgc_void CClientSession::MsgNotify( xgc_lpvoid data, xgc_size size )
	{
		pfn_ClientMsgHandler( this, data, size, handle_ );
	}

	xgc_void CClientSession::Send( xgc_lpvoid data, xgc_size size ) const
	{
		net::SendPacket( handle_, data, size );
	}
}