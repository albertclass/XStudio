#pragma once
#ifndef _CLIENT_SESSION_H_
#define _CLIENT_SESSION_H_
#include "base_session.h"

namespace net_module
{
	class CRelaySession;

	class CClientSession : public CBaseSession
	{
	private:
		/// 当前转发的服务器ID
		CBaseSession* mRelaySession;

	public:
		CClientSession();
		~CClientSession();

		///
		/// \brief 设置中继会话
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:10
		///
		xgc_void SetRelaySession( CBaseSession* pSession )
		{
			mRelaySession = pSession;
		}

		///
		/// \brief 转发消息
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:10
		///
		xgc_void Relay( xgc_lpvoid data, xgc_size size ) const;

	private:
		///
		/// \brief 数据包是否
		/// \return	0 ~ 成功, -1 ~ 失败
		///
		virtual int OnParsePacket( const void* data, xgc_size size ) override;

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
		/// \brief 发送数据
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:11
		///
		virtual xgc_void Send( xgc_lpvoid data, xgc_size size ) const override;
	};
}
#endif // _CLIENT_SESSION_H_
