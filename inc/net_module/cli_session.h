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
		CRelaySession* mRelaySession;

	public:
		CClientSession();
		~CClientSession();

		///
		/// \brief 事件通知
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:10
		///
		virtual xgc_ulong EvtNotify( xgc_uint32 event, xgc_uint32 result ) override;

		///
		/// \brief 消息通知
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:12
		///
		virtual xgc_ulong MsgNotify( xgc_lpvoid data, xgc_size size ) override;

		///
		/// \brief 发送数据
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:11
		///
		virtual xgc_void Send( xgc_lpvoid data, xgc_size size );
	};
}
#endif // _CLIENT_SESSION_H_
