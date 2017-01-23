///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file MessageQueue.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 消息接收队列
///
///////////////////////////////////////////////////////////////
#include "asio_Header.h"
namespace xgc
{
	namespace net
	{
		CMessageQueue::CMessageQueue()
			: mIndex( 0 )
		{
		}

		CMessageQueue::~CMessageQueue()
		{
			// 清除所有消息
			for( auto it : mPacketList )
				it->release();
		}

		xgc_bool CMessageQueue::PopMessage( INetPacket** pPacket )
		{
			if( pPacket == xgc_nullptr )
				return false;

			// 锁不到不要等，防止阻塞主线程
			std::unique_lock< std::mutex > _guard( mMsgGuard, std::try_to_lock );
			if( _guard.owns_lock() )
			{
				if( mPacketList.empty() )
					return false;

				*pPacket = mPacketList.front();
				mPacketList.pop_front();

				return *pPacket != xgc_nullptr;
			}

			return false;
		}

		xgc_void CMessageQueue::PushMessage( INetPacket* pPacket )
		{
			if( !pPacket ) 
				return;

			if( pPacket->length() < 2 )
			{
				pPacket->release();
				return;
			}

			// 消息计数
			++mIndex;

			std::lock_guard< std::mutex > lock( mMsgGuard );
			mPacketList.push_back( pPacket );
		}

		xgc_size CMessageQueue::Length() const
		{
			std::lock_guard< std::mutex > lock( mMsgGuard );

			return mPacketList.size();
		}

		xgc_void CMessageQueue::Release()
		{
			delete this;
		}

	}
}