#pragma once
#include "Netbase.h"
#include <set>
#include <mutex>

using namespace XGC::common;
namespace XGC
{
	namespace net
	{
		class CMessageQueue	:	public IMessageQueue
		{
		public:
			CMessageQueue(xgc_void);
			~CMessageQueue(xgc_void);

			CMessageQueue( const CMessageQueue& ) = delete;
			CMessageQueue( const CMessageQueue&& ) = delete;

			virtual xgc_bool PopMessage( INetPacket** ) override;
			virtual xgc_void PushMessage( INetPacket* ) override;

			virtual xgc_size Length()const override;

			virtual xgc_void Release() override;
		private:
			typedef std::list< INetPacket* > PacketList;
			PacketList	lstClient;

			volatile long mIndex;

			mutable std::mutex mMsgGuard;
		};
	}
}