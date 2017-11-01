#pragma once
#ifndef _RELAY_HANDLER_H_
#define _RELAY_HANDLER_H_

#include "base_session.h"
namespace net_module
{
	class CClientSession;
	class CPipeSession;
	///
	/// \brief ��ת���ӣ���������Ӳ��������ͨ��һ���������ӷ��͵��ڲ�����
	///
	/// \author albert.xu
	/// \date 2017/02/21 16:36
	///
	class CRelaySession : public CBaseSession
	{
	friend CPipeSession;
	private:
		/// ͨ���ĸ��ܵ���������
		CPipeSession *mPipeSession;
		/// ��ǰ״̬
		enum enStatus
		{
			eWaitConnect,
			eConnected,

			eCloseWait,
			eClosed,
		};

		/// ��ǰ����״̬
		enStatus mStatus;
		
		/// ״̬�ı�ʱ���
		xgc_time64 mTimeStamp;

		/// �ͻ��˵����Ӿ��
		net::network_t mClientHandle;
	public:
		///
		/// \brief ���캯��
		///
		/// \author albert.xu
		/// \date 2017/02/17 16:51
		///
		CRelaySession( CPipeSession *pPipeSession, net::network_t hClient );

		///
		/// \brief ��������
		///
		/// \author albert.xu
		/// \date 2017/02/17 16:51
		///
		~CRelaySession();

		///
		/// \brief ��ȡ�ͻ��˻Ự
		///
		/// \author albert.xu
		/// \date 2017/08/01
		///
		CClientSession* GetClientSession() const;

		///
		/// \brief ת����Ϣ
		///
		/// \author albert.xu
		/// \date 2017/08/01
		///
		xgc_void Relay( xgc_lpvoid data, xgc_size size ) const;
	private:
		///
		/// \brief ���ӽ���
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:09
		///
		virtual xgc_void OnAccept( net::network_t handle ) override;

		///
		/// \brief ���ӽ���
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:09
		///
		virtual xgc_void OnConnect( net::network_t handle ) override;

		///
		/// \brief ���ӹر�
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:10
		///
		virtual xgc_void OnClose() override;

		///
		/// \brief �¼�֪ͨ
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:10
		///
		virtual xgc_void EvtNotify( xgc_uint32 event, xgc_uint32 result ) override;

		///
		/// \brief ��Ϣ֪ͨ
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:12
		///
		virtual xgc_void MsgNotify( xgc_lpvoid data, xgc_size size ) override;

		///
		/// \brief ����ԭ����Ϣ
		///
		/// \author albert.xu
		/// \date 2017/02/20 17:04
		///
		virtual xgc_void Send( xgc_lpvoid data, xgc_size size ) const override;

	protected:
		///
		/// \brief �ȴ����ӹر�
		///
		/// \author albert.xu
		/// \date 2017/03/14 11:06
		///
		enStatus GetStatus() const
		{
			return mStatus;
		}

		///
		/// \brief �ȴ����ӹر�
		///
		/// \author albert.xu
		/// \date 2017/03/14 11:06
		///
		xgc_void SetStatus( enStatus eStatus )
		{
			mStatus = eStatus;
			mTimeStamp = tick();
		}

		///
		/// \brief ��ȡ״̬ʱ���
		///
		/// \author albert.xu
		/// \date 2017/03/14 17:13
		///
		xgc_time64 GetTimeStamp()const
		{
			return mTimeStamp;
		}
	};
}
#endif // _RELAY_HANDLER_H_