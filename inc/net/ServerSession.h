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
		class CServerSession :	public INetworkSession
		{
		public:
			CServerSession( MessageQueuePtr queue_ptr, const PacketProtocal* protocal );
			~CServerSession();

		private:
			///function	:	���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
			///param	:	��
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnClose();

			///function	:	�µ�Զ�����ӳɹ�ʱ�Ĵ�����
			///param	:	new_handle	�����ӵ��׽���
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnAccept( network_t new_handle, xgc_lpvoid from );

			///function	:	�ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
			///param	:	data	���ܵ�������	����len��Χ�ڿɿ���Ч
			///param	:	len		���ܵ����ݵĳ���	��������iocp��֤����ʵ�ԡ�
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnRecv( const void *data, size_t size );

			///function	:	�ɹ������������ݰ����iocp�ϲ㴦��ĺ�����Ŀǰ��ʱδʹ��
			///param	:	data	�ɹ����͵�����
			///param	:	len		�ɹ����͵����ݳ���
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnSend( const void *data, size_t size );

			///function	:	iocp���񵽴����ṩ��Ӧ�õĴ������
			///param	:	error	�������
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnError( int error );

		private:
			/// ��Ϣ����
			MessageQueuePtr queue_ptr;
			/// Э����
			const PacketProtocal *protocol_;
		};
	}
}
#endif // _SERVERSESSION_H_