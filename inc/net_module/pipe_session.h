#pragma once
#ifndef _PIPE_SESSION_H_
#define _PIPE_SESSION_H_

namespace net_module
{
	class CBaseSession;
	class CPipeSession : public net::INetworkSession
	{
		friend class CPipeManager;

	private:
		/// ������
		net::network_t mHandle;
		/// ����pingֵ
		xgc_uint16 mPing;
		/// ��ʱ����
		xgc_uint16 mPingFailed;
		/// �����ʶ
		NETWORK_ID mPipeID;
		/// ��Ϣ������
		PipeMsgHandler	mMsgHandler;
		/// �¼�������
		PipeEvtHandler	mEvtHandler;
		/// �м̵���Ϣ������
		SockMsgHandler	mSockMsgHandler;
		/// �м̵��¼�������
		SockEvtHandler	mSockEvtHandler;
		/// ������Ϣ����
		struct SynInfo;
		/// ���ӵȴ��б�
		xgc_list< SynInfo* > mSynList;
		/// ��������
		volatile xgc_uint32 mEventToken;
		/// �м�����ӳ���
		xgc_unordered_map< net::network_t, CBaseSession* > mRelayMap;
	public:
		///
		/// \brief ���캯��
		///
		/// \author albert.xu
		/// \date 2017/03/09 16:01
		///
		CPipeSession();

		///
		/// \brief ��������
		///
		/// \author albert.xu
		/// \date 2017/03/09 16:01
		///
		~CPipeSession();

		///
		/// \brief ���ùܵ��Ĵ�����
		///
		/// \author albert.xu
		/// \date 2017/03/13 17:25
		///
		xgc_void SetPipeHandler( PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );

		///
		/// \brief �����м̵Ĵ�����
		///
		/// \author albert.xu
		/// \date 2017/03/13 17:26
		///
		xgc_void SetSockHandler( SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler );

		///
		/// \brief ����ԭ����Ϣ
		///
		/// \author albert.xu
		/// \date 2017/02/20 17:04
		///
		xgc_void SendPacket( xgc_lpvoid data, xgc_size size );

		///
		/// \brief �����м���Ϣ
		///
		/// \author albert.xu
		/// \date 2017/03/13 11:42
		///
		xgc_void SendRelayPacket( net::network_t handle, xgc_lpvoid data, xgc_size size );

		///
		/// \brief ��ȡ�����ʶ
		///
		/// \author albert.xu
		/// \date 2017/03/08 15:27
		///
		NETWORK_ID GetPipeID()const
		{
			return mPipeID;
		}

		///
		/// \brief �м����ӵ��¼�����
		///
		/// \author albert.xu
		/// \date 2017/03/13 16:20
		///
		xgc_ulong RelayEvtNotify( CRelaySession* session, xgc_uint32 event, xgc_uint32 result );

		///
		/// \brief �м����ӵ���Ϣ֪ͨ
		///
		/// \author albert.xu
		/// \date 2017/03/03 11:12
		///
		xgc_ulong RelayMsgNotify( CRelaySession* session, xgc_lpvoid data, xgc_size size );

		///
		/// \brief �����м�����
		///
		/// \author albert.xu
		/// \date 2017/03/14 10:39
		///
		xgc_void RelayConnect( CBaseSession* session );

		///
		/// \brief �Ͽ��м�����
		///
		/// \author albert.xu
		/// \date 2017/03/14 10:39
		///
		xgc_void RelayDisconnect( CBaseSession* session );

		///
		/// \brief ��ȡ�м�����
		///
		/// \author albert.xu
		/// \date 2017/03/13 16:42
		///
		CBaseSession* GetRelaySession( net::network_t handle ) const;

	private:
		///
		/// \brief ���ݰ��Ƿ�
		/// \return	0 ~ �ɹ�, -1 ~ ʧ��
		///
		virtual int OnParsePacket( const void* data, xgc_size size ) override;

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
		/// \brief ���Ӵ���
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:09
		///
		virtual xgc_void OnError( xgc_uint32 error_code ) override;

		///
		/// \brief ���ӹر�
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:10
		///
		virtual xgc_void OnClose() override;

		///
		/// \brief ��������
		///
		/// \author albert.xu
		/// \date 2017/02/28 11:10
		///
		virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

		///
		/// \brief ���籣���¼�
		///
		/// \author albert.xu
		/// \date 2017/03/03 10:41
		///
		virtual xgc_void OnAlive() override;

	private:
		///
		/// \brief ��Ӧ�ڲ���Ϣ
		///
		/// \author albert.xu
		/// \date 2017/03/10 14:30
		///
		virtual xgc_void OnInner( xgc_lpvoid data, xgc_size size );

		///
		/// \brief ��Ӧ�¼���Ϣ
		///
		/// \author albert.xu
		/// \date 2017/03/09 16:23
		///
		virtual xgc_void OnEvent( xgc_lpvoid data, xgc_size size );

		///
		/// \brief ��Ӧ�ڲ���Ϣ
		///
		/// \author albert.xu
		/// \date 2017/03/10 14:30
		///
		virtual xgc_void OnTrans( xgc_lpvoid data, xgc_size size );
	};
}
#endif // _PIPE_SESSION_H_