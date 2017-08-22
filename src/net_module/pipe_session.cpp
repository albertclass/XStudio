#include "config.h"
#include "pipe_session.h"
#include "cli_session.h"
#include "relay_session.h"

#include "pipe_manager.h"

namespace net_module
{
	extern xgc_bool OnPipeConnect( NETWORK_ID nID, CPipeSession* pPipe );

	#define ePipeInnerPacket	(0)
	#define ePipeEventPacket	(1)
	#define ePipeTransPacket	(2)

	#define PIPE_VERSION 20170302

	#define PIPE_INNER_VERSION_REQ (0)
	#define PIPE_INNER_VERSION_ACK (1)
	#define PIPE_INNER_PING		(2)
	#define PIPE_INNER_PONG		(3)

	/// 握手消息
	#define PIPE_EVENT_CONNECT		(0)
	#define PIPE_EVENT_SHUTDOWN		(1)

	/// 握手消息掩码
	#define PIPE_MASK_SYN		(1)
	#define PIPE_MASK_FIN		(2)
	#define PIPE_MASK_ACK		(4)

	#pragma pack(1)
	///
	/// \brief 中转连接的协议头
	///
	/// \author albert.xu
	/// \date 2017/02/27 15:42
	///
	struct PipeHeader
	{
		/// 中转消息长度
		xgc_uint16 length;
		/// 中转消息类型 （eRelayInnerPacket - 内部消息， eRelayEventPacket - 事件消息， eRelayRelayPacket - 中继消息 ）
		xgc_uint16 kind;
	};

	///
	/// \brief 中转数据
	///
	/// \author albert.xu
	/// \date 2017/02/27 15:42
	///
	struct PipeTransPacket : public PipeHeader
	{
		/// 中转消息句柄
		xgc_uint32 trans;
	};

	///
	/// \brief 消息号
	///
	/// \author albert.xu
	/// \date 2017/03/02 17:40
	///
	struct PipeInner : public PipeHeader
	{
		xgc_uint16 code;
	};

	///
	/// \brief 连接建立请求
	///
	/// \author albert.xu
	/// \date 2017/03/02 17:31
	///
	struct PipeVersion : public PipeInner
	{
		/// 网络标识
		NETWORK_ID network;
		/// Relay连接版本号
		xgc_uint32 version;
		/// Relay连接版本号
		xgc_uint32 error;
	};

	///
	/// \brief PING消息
	///
	/// \author albert.xu
	/// \date 2017/03/02 17:37
	///
	struct PipePing : public PipeInner
	{
		/// 时间戳
		xgc_uint64 timestamp;
	};

	struct PipeEvent : public PipeHeader
	{
		/// 事件号
		xgc_uint16 code;
		/// 事件掩码
		xgc_uint16 mask;
		/// 事件对象
		xgc_uint32 trans;
		/// 令牌
		xgc_uint32 token;
	};
	#pragma pack()
	//////////////////////////////////////////////////////////////////////////
	struct CPipeSession::SynInfo
	{
		/// 时间戳
		xgc_time64 timestamp;
		/// 标识
		xgc_uint32 trans;
		/// 令牌
		xgc_uint32 token;
		/// 会话
		CBaseSession *session;
	};

	CPipeSession::CPipeSession()
		: mMsgHandler( xgc_nullptr )
		, mEvtHandler( xgc_nullptr )
		, mSockMsgHandler( xgc_nullptr )
		, mSockEvtHandler( xgc_nullptr )
		, mEventToken( 0 )
	{
	}

	CPipeSession::~CPipeSession()
	{
		mMsgHandler = xgc_nullptr;
		mEvtHandler = xgc_nullptr;

		mSockMsgHandler = xgc_nullptr;
		mSockEvtHandler = xgc_nullptr;
	}

	xgc_void CPipeSession::SetPipeHandler( PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler )
	{
		mMsgHandler = fnMsgHandler;
		mEvtHandler = fnEvtHandler;
	}

	xgc_void CPipeSession::SetSockHandler( SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler )
	{
		mSockMsgHandler = fnMsgHandler;
		mSockEvtHandler = fnEvtHandler;
	}

	xgc_void CPipeSession::SendPacket( xgc_lpvoid data, xgc_size size ) const
	{
		net::SendPacket( mHandle, data, size );
	}

	xgc_void CPipeSession::SendRelayPacket( net::network_t handle, xgc_lpvoid data, xgc_size size ) const
	{
		PipeTransPacket pkg;
		XGC_ASSERT( size < std::numeric_limits<xgc_uint16>::max() );
		pkg.length = htons( (xgc_uint16) (size + sizeof( pkg )) );
		pkg.kind = htons( ePipeTransPacket );
		pkg.trans = htonl( handle );

		net::SendPacketChains( mHandle, net::BufferChains {
			std::tuple< xgc_lpvoid, xgc_size >{ &pkg, sizeof( pkg ) },
			std::tuple< xgc_lpvoid, xgc_size >{ data, size },
		} );
	}

	xgc_void CPipeSession::RelayEvtNotify( CRelaySession* session, xgc_uint32 event, xgc_uint32 result )
	{
		return mSockEvtHandler( session, event, result );
	}

	xgc_void CPipeSession::RelayMsgNotify( CRelaySession* session, xgc_lpvoid data, xgc_size size )
	{
		return mSockMsgHandler( session, data, size, mHandle );
	}

	xgc_void CPipeSession::RelayConnect( CBaseSession * session )
	{
		// 发送连接请求
		PipeEvent req;
		req.length = htons( sizeof( PipeEvent ) );
		req.kind = htons( ePipeEventPacket );
		req.code = htons( PIPE_EVENT_CONNECT );
		req.mask = PIPE_MASK_SYN;
		req.mask = htons( req.mask );
		req.trans = htonl( session->GetHandle() );
		req.token = htonl( mEventToken );

		SendPacket( &req, sizeof( req ) );
	}

	xgc_void CPipeSession::RelayDisconnect( CBaseSession * session )
	{
		auto trans = session->GetHandle();

		PipeEvent rpn;
		rpn.length = htons( sizeof( rpn ) );
		rpn.kind = htons( ePipeEventPacket );
		rpn.code = htons( PIPE_EVENT_SHUTDOWN );
		rpn.mask = htons( PIPE_MASK_FIN );
		rpn.trans = htonl( trans );
		rpn.token = htonl( mEventToken );

		SendPacket( &rpn, sizeof( rpn ) );
	}

	CBaseSession* CPipeSession::GetRelaySession( net::network_t handle ) const
	{
		auto it = mRelayMap.find( handle );
		if( it != mRelayMap.end() )
			return it->second;

		return nullptr;
	}

	int CPipeSession::OnParsePacket( const void * data, xgc_size size )
	{
		return ntohs( *(xgc_uint16*) data );
	}

	xgc_void CPipeSession::OnAccept( net::network_t handle )
	{
		mHandle = handle;

		PipeVersion pkg;
		pkg.length = htons( sizeof( PipeVersion ) );
		pkg.kind = htons( ePipeInnerPacket );
		pkg.code = htons( PIPE_INNER_VERSION_REQ );
		pkg.version = htonl( PIPE_VERSION );
		pkg.network = htonll( _GetNetworkID() );
		pkg.error = 0;

		net::SendPacket( mHandle, &pkg, sizeof( pkg ) );
	}

	xgc_void CPipeSession::OnConnect( net::network_t handle )
	{
		mHandle = handle;
	}

	xgc_void CPipeSession::OnError( xgc_int16 error_type, xgc_int16 error_code )
	{
		mEvtHandler( this, EVENT_ERROR, error_code );
	}

	xgc_void CPipeSession::OnClose()
	{
		SYS_INFO( "Pipe %u disconnect.", GetPipeID() );
		mEvtHandler( this, EVENT_CLOSE, 0 );
	}

	xgc_void CPipeSession::OnRecv( xgc_lpvoid data, xgc_size size )
	{
		auto pkg = static_cast<PipeHeader*>(data);
		auto length = ntohs( pkg->length );
		XGC_ASSERT( length == size );
		auto kind = ntohs( pkg->kind );

		switch( kind )
		{
		case ePipeInnerPacket:
			OnInner( data, size );
			break;
		case ePipeEventPacket:
			OnEvent( data, size );
			break;
		case ePipeTransPacket:
			OnTrans( data, size );
			break;
		}
	}

	xgc_void CPipeSession::OnAlive()
	{
		PipePing ack;
		ack.length = htons( sizeof( ack ) );
		ack.kind = htons( ePipeInnerPacket );
		ack.code = htons( PIPE_INNER_PING );
		ack.timestamp = htonll( tick() );

		SendPacket( &ack, sizeof( ack ) );

		// 搜索超时的连接请求
		auto it = mSynList.begin();
		while( it != mSynList.end() )
		{
			auto info = *it;
			if( tick() - info->timestamp )
			{

			}
			++it;
		}
	}

	xgc_void CPipeSession::OnInner( xgc_lpvoid data, xgc_size size )
	{
		XGC_ASSERT_RETURN( size >= sizeof( PipeInner ), XGC_NONE );
		auto inner = static_cast<PipeInner*>(data);
		auto code = ntohs( inner->code );

		switch( code )
		{
		case PIPE_INNER_VERSION_REQ:
			{
				XGC_ASSERT_RETURN( size >= sizeof( PipeVersion ), XGC_NONE );
				auto req = static_cast<PipeVersion*>(data);
				auto pid = ntohll( req->network );

				PipeVersion ack;
				ack.length = htons( sizeof( ack ) );
				ack.kind = htons( ePipeInnerPacket );
				ack.code = htons( PIPE_INNER_VERSION_ACK );
				ack.version = htonl( PIPE_VERSION );
				ack.network = htonll( _GetNetworkID() );
				ack.error = 0;

				if( ntohl( req->version ) != PIPE_VERSION )
				{
					SYS_ERROR( "relay version does not match! server ver: %u, client ver: %s", req->version, PIPE_VERSION );
					ack.error = -1;
					net::SendPacket( mHandle, &ack, sizeof( ack ) );
					net::CloseLink( mHandle );
				}
				else
				{
					mPipeID = pid;
					mEvtHandler( this, EVENT_CONNECT, mPipeID );
					net::SendPacket( mHandle, &ack, sizeof( ack ) );
				}
			}
			break;
		case PIPE_INNER_VERSION_ACK:
			{
				XGC_ASSERT_RETURN( size >= sizeof( PipeVersion ), XGC_NONE );
				auto ack = static_cast<PipeVersion*>(data);
				if( ack->error != 0 )
				{
					SYS_ERROR( "relay version does not match! server ver: %u, client ver: %s", PIPE_VERSION, ack->version );
					net::CloseLink( mHandle );
				}
				else
				{
					mPipeID = ntohll( ack->network );
					auto connected = OnPipeConnect( mPipeID, this );
					XGC_ASSERT( connected );

					mEvtHandler( this, EVENT_ACCEPT, mPipeID );
				}
			}
			break;
		case PIPE_INNER_PING:
			{
				XGC_ASSERT_RETURN( size >= sizeof( PipePing ), XGC_NONE );
				auto req = static_cast<PipePing*>(data);

				PipePing ack;
				ack.length = htons( sizeof( ack ) );
				ack.kind = htons( ePipeInnerPacket );
				ack.code = htons( PIPE_INNER_PONG );
				ack.timestamp = req->timestamp;

				SendPacket( &ack, sizeof( ack ) );
			}
			break;
		case PIPE_INNER_PONG:
			{
				XGC_ASSERT_RETURN( size >= sizeof( PipePing ), XGC_NONE );
				auto ping = static_cast<PipePing*>(data);
				mPing = (xgc_uint16) (tick() - ntohll( ping->timestamp ));
				mPingFailed = 0;
				mEvtHandler( this, EVENT_PING, mPing );
			}
			break;
		}
	}

	xgc_void CPipeSession::OnEvent( xgc_lpvoid data, xgc_size size )
	{
		XGC_ASSERT_RETURN( size >= sizeof( PipeEvent ), XGC_NONE );
		auto evt = static_cast<PipeEvent*>(data);

		auto code = ntohs( evt->code );
		auto mask = ntohs( evt->mask );

		auto trans = ntohl( evt->trans );
		auto token = ntohl( evt->token );

		mEventToken = token + 2;
		switch( code )
		{
		case PIPE_EVENT_CONNECT:
			if( XGC_CHK_FLAGS( mask, PIPE_MASK_SYN ) )
			{
				auto syn = XGC_NEW SynInfo;
				if( syn )
				{
					syn->timestamp = tick();
					syn->trans = trans;
					syn->token = token;
					syn->session = XGC_NEW CRelaySession( this, trans );
					mSynList.push_back( syn );

					// 回复连接请求
					PipeEvent rpn;
					rpn.length = htons( sizeof( PipeEvent ) );
					rpn.kind = htons( ePipeEventPacket );
					rpn.code = htons( PIPE_EVENT_CONNECT );
					rpn.mask = XGC_CHK_FLAGS( mask, PIPE_MASK_ACK ) ? PIPE_MASK_ACK : (PIPE_MASK_SYN | PIPE_MASK_ACK);
					rpn.mask = htons( rpn.mask );
					rpn.trans = htonl( trans );
					rpn.token = htonl( token + 1 );

					SendPacket( &rpn, sizeof( rpn ) );
				}
			}

			if( XGC_CHK_FLAGS( mask, PIPE_MASK_ACK ) )
			{
				for( auto it = mSynList.begin(); it != mSynList.end(); ++it )
				{
					auto info = *it;
					XGC_ASSERT_CONTINUE( info );
					if( info->token + 1 == token )
					{
						XGC_ASSERT( mRelayMap.find( trans ) == mRelayMap.end() );
						net::Param_GetSession param;
						param.handle = trans;

						if( -1 != net::ExecuteState( Operator_GetSession, &param ) )
						{
							auto pClientSession = (CClientSession*)param.session;
							if( pClientSession )
							{
								pClientSession->SetRelaySession( info->session );
								// regist relay session in pipe
								mRelayMap[trans] = info->session;
								// trigger connect event
								if( XGC_CHK_FLAGS( mask, PIPE_MASK_SYN ) )
								{
									info->session->OnConnect( trans );
								}
								else
								{
									info->session->OnAccept( trans );
								}
							}
						}
						// erase syn info
						SAFE_DELETE( info );
						mSynList.erase( it );
						break;
					}
				}
			}
			break;
		case PIPE_EVENT_SHUTDOWN:
			if( XGC_CHK_FLAGS( mask, PIPE_MASK_FIN ) )
			{
				// 转换连接状态
				auto cli = (CRelaySession*) GetRelaySession( trans );
				if( cli && cli->GetStatus() == CRelaySession::eConnected )
				{
					PipeEvent rpn;
					rpn.length = htons( sizeof( rpn ) );
					rpn.kind = htons( ePipeEventPacket );
					rpn.code = htons( PIPE_EVENT_SHUTDOWN );
					rpn.mask = htons( PIPE_MASK_FIN );
					rpn.trans = htonl( trans );
					rpn.token = htonl( mEventToken );

					SendPacket( &rpn, sizeof( rpn ) );

					cli->SetStatus( CRelaySession::eCloseWait );
				}

				// 回应关闭连接
				PipeEvent rpn;
				rpn.length = htons( sizeof( rpn ) );
				rpn.kind = htons( ePipeEventPacket );
				rpn.code = htons( PIPE_EVENT_SHUTDOWN );
				rpn.mask = htons( PIPE_MASK_ACK );
				rpn.trans = htonl( trans );
				rpn.token = htonl( token + 1 );

				SendPacket( &rpn, sizeof( rpn ) );
			}

			if( XGC_CHK_FLAGS( mask, PIPE_MASK_ACK ) )
			{
				auto cli = (CRelaySession*)GetRelaySession( trans );
				if( cli )
				{
					cli->OnClose();
					cli->SetStatus( CRelaySession::eClosed );
				}
			}
			break;
		}
	}

	xgc_void CPipeSession::OnTrans( xgc_lpvoid data, xgc_size size )
	{
		XGC_ASSERT_RETURN( size >= sizeof( PipeTransPacket ), XGC_NONE );
		auto pkg = static_cast<PipeTransPacket*>(data);
		auto trans = ntohl( pkg->trans );

		auto pSession = GetRelaySession( trans );
		if( pSession )
			pSession->OnRecv( (char*) data + sizeof( pkg ), size - sizeof( pkg ) );
	}
}