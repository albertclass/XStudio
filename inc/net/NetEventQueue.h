///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file MessageQueue.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 消息接收队列
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_

namespace xgc
{
	namespace net
	{
		///
		/// \brief 消息队列，继承与消息队列接口
		///
		/// \author albert.xu
		/// \date 2016/02/23 17:43
		///
		class CNetEventQueue
		{
		public:
			CNetEventQueue();
			~CNetEventQueue();

			CNetEventQueue( const CNetEventQueue& ) = delete;
			CNetEventQueue( const CNetEventQueue&& ) = delete;

			///
			/// \brief 从队列头弹出一个已接收的消息
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:43
			///
			xgc_bool Kick( INetPacket** );

			///
			/// \brief 压入一个已接收的消息到队列未
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:43
			///
			xgc_void Push( INetPacket* );

			///
			/// \brief 获取队列长度
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:44
			///
			xgc_size Length()const;

		private:
			/// 从网络接收的消息队列
			std::list< INetPacket* > mPacketList;
			/// 同步锁
			mutable std::mutex mMsgGuard;
		};
	}
}

#endif // _MESSAGEQUEUE_H_