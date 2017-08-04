#include "config.h"
#include "relay_session.h"
#include "pipe_session.h"

namespace net_module
{
	//////////////////////////////////////////////////////////////////////////
	CRelaySession::CRelaySession( CPipeSession *pPipeSession, net::network_t hClient )
		: CBaseSession()
		, mPipeSession( pPipeSession )
		, mClientHandle( hClient )
		, mStatus( eWaitConnect )
	{
	}

	CRelaySession::~CRelaySession()
	{
		mStatus = eClosed;
	}

	///
	/// \brief 获取客户端会话
	///
	/// \author albert.xu
	/// \date 2017/08/01
	///
	CClientSession* CRelaySession::GetClientSession() const
	{
		net::Param_GetSession param;
		param.handle = mClientHandle;
		
		if( -1 == net::ExecuteState( Operator_GetSession, &param ) )
			return xgc_nullptr;

		return ( CClientSession* )param.session;
	}

	///
	/// \brief 转发消息
	///
	/// \author albert.xu
	/// \date 2017/08/01
	///
	xgc_void CRelaySession::Relay( xgc_lpvoid data, xgc_size size ) const
	{
		net::SendPacket( mClientHandle, data, size );
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

	xgc_void CRelaySession::EvtNotify( xgc_uint32 event, xgc_uint32 result )
	{
		mPipeSession->RelayEvtNotify( this, event, result );
	}

	xgc_void CRelaySession::MsgNotify( xgc_lpvoid data, xgc_size size )
	{
		mPipeSession->RelayMsgNotify( this, data, size );
	}

	xgc_void CRelaySession::Send( xgc_lpvoid data, xgc_size size ) const
	{
		mPipeSession->SendRelayPacket( handle_, data, size );
	}
}