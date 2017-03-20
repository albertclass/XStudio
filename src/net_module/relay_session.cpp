#include "config.h"
#include "relay_session.h"
#include "pipe_session.h"

namespace net_module
{
	//////////////////////////////////////////////////////////////////////////
	CRelaySession::CRelaySession( CPipeSession *pPipeSession )
		: CBaseSession()
		, mPipeSession( pPipeSession )
		, mStatus( eWaitConnect )
	{
	}

	CRelaySession::~CRelaySession()
	{
		mStatus = eClosed;
	}

	xgc_void CRelaySession::OnAccept( net::network_t handle )
	{
		handle_ = handle;
		mStatus = eConnected;
		EvtNotify( EVENT_ACCEPT, handle );
	}

	xgc_void CRelaySession::OnConnect( net::network_t handle )
	{
		handle_ = handle;
		mStatus = eConnected;
		EvtNotify( EVENT_CONNECT, handle );
	}

	xgc_void CRelaySession::OnClose()
	{
		EvtNotify( EVENT_CLOSE, 0 );
		mStatus = eClosed;
	}

	xgc_ulong CRelaySession::EvtNotify( xgc_uint32 event, xgc_uint32 result )
	{
		return mPipeSession->RelayEvtNotify( this, event, result );
	}

	xgc_ulong CRelaySession::MsgNotify( xgc_lpvoid data, xgc_size size )
	{
		return mPipeSession->RelayMsgNotify( this, data, size );
	}

	xgc_void CRelaySession::Send( xgc_lpvoid data, xgc_size size )
	{
		mPipeSession->SendRelayPacket( handle_, data, size );
	}
}