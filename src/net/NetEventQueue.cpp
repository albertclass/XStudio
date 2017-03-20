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
#include "Header.h"
#include "NetEventQueue.h"
namespace xgc
{
	namespace net
	{
		CNetEventQueue::CNetEventQueue()
		{
		}

		CNetEventQueue::~CNetEventQueue()
		{
			// ���������Ϣ
			for( auto it : mPacketList )
				it->freedom();
		}

		xgc_bool CNetEventQueue::Kick( INetPacket** pPacket )
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

		xgc_void CNetEventQueue::Push( INetPacket* pPacket )
		{
			if( !pPacket ) 
				return;

			std::lock_guard< std::mutex > lock( mMsgGuard );
			mPacketList.push_back( pPacket );
		}

		xgc_size CNetEventQueue::Length() const
		{
			std::lock_guard< std::mutex > lock( mMsgGuard );

			return mPacketList.size();
		}
	}
}