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
		class CMessageQueue	:	public IMessageQueue
		{
		public:
			CMessageQueue();
			~CMessageQueue();

			CMessageQueue( const CMessageQueue& ) = delete;
			CMessageQueue( const CMessageQueue&& ) = delete;

			///
			/// \brief �Ӷ���ͷ����һ���ѽ��յ���Ϣ
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:43
			///
			virtual xgc_bool PopMessage( INetPacket** ) override;

			///
			/// \brief ѹ��һ���ѽ��յ���Ϣ������δ
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:43
			///
			virtual xgc_void PushMessage( INetPacket* ) override;

			///
			/// \brief ��ȡ���г���
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:44
			///
			virtual xgc_size Length()const override;

			///
			/// \brief �Ƿ���Ϣ����
			///
			/// \author albert.xu
			/// \date 2016/02/23 17:44
			///
			virtual xgc_void Release() override;
		private:
			/// ��������յ���Ϣ����
			std::list< INetPacket* > mPacketList;
			/// ��Ϣ���
			std::atomic< xgc_ulong > mIndex;
			/// ͬ����
			mutable std::mutex mMsgGuard;
		};
	}
}

#endif // _MESSAGEQUEUE_H_