#pragma once
#ifndef _RELAY_HANDLER_H_
#define _RELAY_HANDLER_H_

#include "base_session.h"
namespace net_module
{
	class CClientSession;
	class CPipeSession;
	///
	/// \brief 中转连接，将大量的硬连接数据通过一个代理连接发送到内部网络
	///
	/// \author albert.xu
	/// \date 2017/02/21 16:36
	///
	class CRelaySession : public CBaseSession
	{
	friend CPipeSession;
	private:
		/// 通过哪个管道传输数据
		CPipeSession *mPipeSession;
		/// 当前状态
		enum enStatus
		{
			eWaitConnect,
			eConnected,

			eCloseWait,
			eClosed,
		};

		/// 当前连接状态
		enStatus mStatus;
		
		/// 状态改变时间戳
		xgc_time64 mTimeStamp;

		/// 客户端的连接句柄
		net::network_t mClientHandle;
	public:
		///
		/// \brief 构造函数
		///
		/// \author albert.xu
		/// \date 2017/02/17 16:51
		///
		CRelaySession( CPipeSession *pPipeSession, net::network_t hClient );

		///
		/// \brief 析构函数
		///
		/// \author albert.xu
		/// \date 2017/02/17 16:51
		///
		~CRelaySession();

		///
		/// \brief 获取客户端会话
		///
		/// \author albert.xu
		/// \date 2017/08/01
		///
		CClientSession* GetClientSession() const;

		///
		/// \brief 转发消息
		///
		/// \author albert.xu
		/// \date 2017/08/01
		///
		xgc_void Relay( xgc_lpvoid data, xgc_size size ) const;
	private:
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
		/// \brief 连接关闭
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:10
		///
		virtual xgc_void OnClose() override;

		///
		/// \brief 事件通知
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:10
		///
		virtual xgc_void EvtNotify( xgc_uint32 event, xgc_uint32 result ) override;

		///
		/// \brief 消息通知
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:12
		///
		virtual xgc_void MsgNotify( xgc_lpvoid data, xgc_size size ) override;

		///
		/// \brief 发送原生消息
		///
		/// \author albert.xu
		/// \date 2017/02/20 17:04
		///
		virtual xgc_void Send( xgc_lpvoid data, xgc_size size ) const override;

	protected:
		///
		/// \brief 等待连接关闭
		///
		/// \author albert.xu
		/// \date 2017/03/14 11:06
		///
		enStatus GetStatus() const
		{
			return mStatus;
		}

		///
		/// \brief 等待连接关闭
		///
		/// \author albert.xu
		/// \date 2017/03/14 11:06
		///
		xgc_void SetStatus( enStatus eStatus )
		{
			mStatus = eStatus;
			mTimeStamp = tick();
		}

		///
		/// \brief 获取状态时间戳
		///
		/// \author albert.xu
		/// \date 2017/03/14 17:13
		///
		xgc_time64 GetTimeStamp()const
		{
			return mTimeStamp;
		}
	};
}
#endif // _RELAY_HANDLER_H_