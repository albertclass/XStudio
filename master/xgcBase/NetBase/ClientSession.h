#pragma once
#include "NetBase.h"

namespace XGC
{
	namespace net
	{

		class CClientSession	:	public INetworkSession
		{
		public:
			CClientSession( IMessageQueue *pMessageQueue, const PacketProtocal* pProtocal );
			~CClientSession(xgc_void);

		private:
			///function	:	���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
			///param	:	��
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnClose(xgc_void);

			///function	:	�µ�Զ�����ӳɹ�ʱ�Ĵ�����
			///param	:	new_handle	�����ӵ��׽���
			///return	:	0	�ɹ�	-1 ʧ��
			virtual int OnAccept( network_t new_handle, xgc_uintptr from );

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
			IMessageQueue	*m_pMessageQueue;
			const
			PacketProtocal	*m_pProtocal;
		};

		INetworkSession* create_client_handler( xgc_uintptr lParam );
	}
}