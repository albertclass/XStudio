///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file MessageQueue.cpp
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ��Ϣ���ն���
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
			// ���������Ϣ
			for( auto it : mPacketList )
				it->release();
		}

		xgc_bool CMessageQueue::PopMessage( INetPacket** pPacket )
		{
			if( pPacket == xgc_nullptr )
				return false;

			// ��������Ҫ�ȣ���ֹ�������߳�
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

			// ��Ϣ����
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