#include "MessageQueue.h"
namespace XGC
{
	namespace net
	{
		CMessageQueue::CMessageQueue(xgc_void)
			: mIndex( 0 )
		{
		}

		CMessageQueue::~CMessageQueue(xgc_void)
		{
			// 清除所有消息
			for each( auto it in lstClient )
				it->release();
		}

		xgc_bool CMessageQueue::PopMessage( INetPacket** pPacket )
		{
			if( pPacket == xgc_nullptr )
				return false;

			std::unique_lock< std::mutex > _guard( mMsgGuard, std::try_to_lock );
			if( _guard.owns_lock() )
			{
				if( lstClient.empty() )
					return false;

				*pPacket = lstClient.front();
				lstClient.pop_front();

				return *pPacket != xgc_nullptr;
			}

			return false;
		}

		xgc_void CMessageQueue::PushMessage( INetPacket* pPacket )
		{
			if( !pPacket ) 
				return;

			if( pPacket->size() < 2 )
			{
				pPacket->release();
				return;
			}

			// 消息计数
			InterlockedIncrement( &mIndex );

			std::lock_guard< std::mutex > lock( mMsgGuard );
			lstClient.push_back( pPacket );
		}

		xgc_size CMessageQueue::Length() const
		{
			std::lock_guard< std::mutex > lock( mMsgGuard );

			return lstClient.size();
		}

		xgc_void CMessageQueue::Release()
		{
			delete this;
		}

	}
}