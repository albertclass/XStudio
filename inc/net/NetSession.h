///////////////////////////////////////////////////////////////
/// \file ClientSession.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// �ͻ��˻Ự����
///
///////////////////////////////////////////////////////////////
#pragma once
#ifndef _CLIENTSESSION_H_
#define _CLIENTSESSION_H_

namespace xgc
{
	namespace net
	{
		class CSession : public INetworkSession, public ProtocalDefault
		{
		protected:
			/// ����pingֵ
			xgc_ulong		ping_;
			/// ����pingֵ
			xgc_ulong		pingfailed_;
			/// Socket���
			network_t		handle_;
			/// �û�����
			xgc_lpvoid		userdata_;
			/// ��Ϣ����
			MessageQueuePtr queue_;
		public:
			CSession( MessageQueuePtr Queue );

			~CSession();
		private:
			///
			/// \brief ���ݰ��Ƿ�
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnParsePacket( const void* data, xgc_size size ) override;

			///
			/// \brief ���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnClose() override;

			///
			/// \brief �ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
			/// \param	data ���ܵ�������,����len��Χ�ڿɿ���Ч
			/// \param	len	���ܵ����ݵĳ���,��������iocp��֤����ʵ�ԡ�
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnRecv( const void *data, xgc_size size ) override;

			///
			/// \brief �ɹ������������ݰ����iocp�ϲ㴦��ĺ�����Ŀǰ��ʱδʹ��
			/// \param data �ɹ����͵�����
			/// \param len �ɹ����͵����ݳ���
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnSend( const void *data, xgc_size size ) override;

			///
			/// \brief iocp���񵽴����ṩ��Ӧ�õĴ������
			/// \param error �������
			/// \return 0 ~ �ɹ�, -1 ~ ʧ��
			virtual int OnError( int error ) override;

			///
			/// \brief ���ʱ�������ڶ�ʱ���ͱ�����Ϣ
			/// \return 0 ~ �ɹ�, -1 ~ ʧ��
			virtual int OnAlive() override;

			///
			/// \brief ��ȡ�û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual int GetPing()const override
			{
				return ping_;
			}

			///
			/// \brief ��ȡ�û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual xgc_lpvoid GetUserdata()const override
			{
				return userdata_;
			}

			///
			/// \brief �����û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual xgc_void SetUserdata( xgc_lpvoid userdata ) override
			{
				userdata_ = userdata;
			}
		};

		class CClientSession : public CSession
		{
		public:
			CClientSession( MessageQueuePtr QueuePtr );
			~CClientSession();

		private:
			/// \brief  �µ�Զ�����ӳɹ�ʱ�Ĵ�����
			/// \param	new_handle	�����ӵ��׽���
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnAccept( network_t new_handle, xgc_lpvoid from );
		};

		class CServerSession : public CSession
		{
		public:
			CServerSession( MessageQueuePtr QueuePtr );
			~CServerSession();

		private:
			/// \brief  �µ�Զ�����ӳɹ�ʱ�Ĵ�����
			/// \param	new_handle	�����ӵ��׽���
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnAccept( network_t new_handle, xgc_lpvoid from );
		};

	}
}
#endif // _CLIENTSESSION_H_