#include "config.h"
#include "cli_session.h"
#include "relay_session.h"
#include "relay_manager.h"

namespace net_module
{
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

	xgc_ulong CClientSession::EvtNotify( xgc_uint32 event, xgc_uint32 result )
	{
		return pfn_ClientEvtHandler( this, event, result );
	}

	xgc_ulong CClientSession::MsgNotify( xgc_lpvoid data, xgc_size size )
	{
		return pfn_ClientMsgHandler( this, data, size, handle_ );
	}

	xgc_void CClientSession::Send( xgc_lpvoid data, xgc_size size )
	{
		net::SendPacket( handle_, data, size );
	}

}