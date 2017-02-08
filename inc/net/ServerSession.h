///////////////////////////////////////////////////////////////
/// \file ServerSession.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// �������Ự����
///
///////////////////////////////////////////////////////////////
#ifndef _SERVERSESSION_H_
#define _SERVERSESSION_H_

#pragma once

namespace xgc
{
	namespace net
	{
		class CServerSession :	public INetworkSession, ProtocalDefault
		{
		public:
			CServerSession( MessageQueuePtr queue );
			~CServerSession();

		private:
			///
			/// \brief ���ݰ��Ƿ�
			/// \return	���ݰ�����
			///
			virtual int OnParsePacket( const void* data, xgc_size size );

			/// \brief  �µ�Զ�����ӳɹ�ʱ�Ĵ�����
			/// \param	new_handle	�����ӵ��׽���
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnAccept( network_t new_handle, xgc_lpvoid from );

			/// \brief  �ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
			/// \param	data	���ܵ�������	����len��Χ�ڿɿ���Ч
			/// \param	len		���ܵ����ݵĳ���	��������iocp��֤����ʵ�ԡ�
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnRecv( const void *data, size_t size );

			/// \brief  �ɹ������������ݰ����iocp�ϲ㴦��ĺ�����Ŀǰ��ʱδʹ��
			/// \param	data	�ɹ����͵�����
			/// \param	len		�ɹ����͵����ݳ���
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnSend( const void *data, size_t size );

			/// \brief	���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
			/// \param	��
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnClose();

			/// \brief	iocp���񵽴����ṩ��Ӧ�õĴ������
			/// \param	error	�������
			/// \return	0	�ɹ�	-1 ʧ��
			virtual int OnError( int error );

			///
			/// \brief ���ʱ�������ڶ�ʱ���ͱ�����Ϣ
			/// \return pingʧ�ܴ���
			virtual int OnAlive();

			///
			/// \brief ��ȡ�û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual int GetPing()const
			{
				return ping_;
			}

			///
			/// \brief ��ȡ�û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual xgc_lpvoid GetUserdata()const
			{
				return userdata_;
			}

			///
			/// \brief �����û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			virtual xgc_void SetUserdata( xgc_lpvoid userdata )
			{
				userdata_ = userdata;
			}

		private:
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
		};
	}
}
#endif // _SERVERSESSION_H_