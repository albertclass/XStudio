///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file MessageQueue.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ��Ϣ���ն���
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
		/// \brief ��Ϣ���У��̳�����Ϣ���нӿ�
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
			/// \brief �Ӷ���ͷ����һ���ѽ��յ���Ϣ
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:43
			///
			xgc_bool Kick( INetPacket** );

			///
			/// \brief ѹ��һ���ѽ��յ���Ϣ������δ
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:43
			///
			xgc_void Push( INetPacket* );

			///
			/// \brief ��ȡ���г���
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:44
			///
			xgc_size Length()const;

		private:
			/// ��������յ���Ϣ����
			std::list< INetPacket* > mPacketList;
			/// ͬ����
			mutable std::mutex mMsgGuard;
		};
	}
}

#endif // _MESSAGEQUEUE_H_