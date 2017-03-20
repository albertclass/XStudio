#pragma once
#ifndef _PIPE_SESSION_H_
#define _PIPE_SESSION_H_

namespace net_module
{
	class CBaseSession;
	class CPipeSession : public net::INetworkSession
	{
		friend class CPipeManager;

	private:
		/// 网络句柄
		net::network_t mHandle;
		/// 网络ping值
		xgc_uint16 mPing;
		/// 超时次数
		xgc_uint16 mPingFailed;
		/// 网络标识
		NETWORK_ID mPipeID;
		/// 消息处理函数
		PipeMsgHandler	mMsgHandler;
		/// 事件处理函数
		PipeEvtHandler	mEvtHandler;
		/// 中继的消息处理函数
		SockMsgHandler	mSockMsgHandler;
		/// 中继的事件处理函数
		SockEvtHandler	mSockEvtHandler;
		/// 连接信息定义
		struct SynInfo;
		/// 连接等待列表
		xgc_list< SynInfo* > mSynList;
		/// 网络令牌
		volatile xgc_uint32 mEventToken;
		/// 中继连接映射表
		xgc_unordered_map< net::network_t, CBaseSession* > mRelayMap;
	public:
		///
		/// \brief 构造函数
		///
		/// \author albert.xu
		/// \date 2017/03/09 16:01
		///
		CPipeSession();

		///
		/// \brief 析构函数
		///
		/// \author albert.xu
		/// \date 2017/03/09 16:01
		///
		~CPipeSession();

		///
		/// \brief 设置管道的处理函数
		///
		/// \author albert.xu
		/// \date 2017/03/13 17:25
		///
		xgc_void SetPipeHandler( PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );

		///
		/// \brief 设置中继的处理函数
		///
		/// \author albert.xu
		/// \date 2017/03/13 17:26
		///
		xgc_void SetSockHandler( SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler );

		///
		/// \brief 发送原生消息
		///
		/// \author albert.xu
		/// \date 2017/02/20 17:04
		///
		xgc_void SendPacket( xgc_lpvoid data, xgc_size size );

		///
		/// \brief 发送中继消息
		///
		/// \author albert.xu
		/// \date 2017/03/13 11:42
		///
		xgc_void SendRelayPacket( net::network_t handle, xgc_lpvoid data, xgc_size size );

		///
		/// \brief 获取网络标识
		///
		/// \author albert.xu
		/// \date 2017/03/08 15:27
		///
		NETWORK_ID GetPipeID()const
		{
			return mPipeID;
		}

		///
		/// \brief 中继连接的事件处理
		///
		/// \author albert.xu
		/// \date 2017/03/13 16:20
		///
		xgc_ulong RelayEvtNotify( CRelaySession* session, xgc_uint32 event, xgc_uint32 result );

		///
		/// \brief 中继连接的消息通知
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:12
		///
		xgc_ulong RelayMsgNotify( CRelaySession* session, xgc_lpvoid data, xgc_size size );

		///
		/// \brief 建立中继连接
		///
		/// \author albert.xu
		/// \date 2017/03/14 10:39
		///
		xgc_void RelayConnect( CBaseSession* session );

		///
		/// \brief 断开中继连接
		///
		/// \author albert.xu
		/// \date 2017/03/14 10:39
		///
		xgc_void RelayDisconnect( CBaseSession* session );

		///
		/// \brief 获取中继连接
		///
		/// \author albert.xu
		/// \date 2017/03/13 16:42
		///
		CBaseSession* GetRelaySession( net::network_t handle ) const;

	private:
		///
		/// \brief 数据包是否
		/// \return	0 ~ 成功, -1 ~ 失败
		///
		virtual int OnParsePacket( const void* data, xgc_size size ) override;

		///
		/// \brief 连接建立
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:09
		///
		virtual xgc_void OnAccept( net::network_t handle ) override;

		///
		/// \brief 连接建立
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:09
		///
		virtual xgc_void OnConnect( net::network_t handle ) override;

		///
		/// \brief 连接错误
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:09
		///
		virtual xgc_void OnError( xgc_uint32 error_code ) override;

		///
		/// \brief 连接关闭
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:10
		///
		virtual xgc_void OnClose() override;

		///
		/// \brief 接收数据
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:10
		///
		virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

		///
		/// \brief 网络保活事件
		///
		/// \author albert.xu
		/// \date 2017/03/03 10:41
		///
		virtual xgc_void OnAlive() override;

	private:
		///
		/// \brief 响应内部消息
		///
		/// \author albert.xu
		/// \date 2017/03/10 14:30
		///
		virtual xgc_void OnInner( xgc_lpvoid data, xgc_size size );

		///
		/// \brief 响应事件消息
		///
		/// \author albert.xu
		/// \date 2017/03/09 16:23
		///
		virtual xgc_void OnEvent( xgc_lpvoid data, xgc_size size );

		///
		/// \brief 响应内部消息
		///
		/// \author albert.xu
		/// \date 2017/03/10 14:30
		///
		virtual xgc_void OnTrans( xgc_lpvoid data, xgc_size size );
	};
}
#endif // _PIPE_SESSION_H_