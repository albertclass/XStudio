#include "Netbase.h"
#include "ServerSession.h"
#include "asio_NetworkPacket.h"

namespace XGC
{
	namespace net
	{
		CServerSession::CServerSession( IMessageQueue *pMessageQueue, const PacketProtocal* pProtocal )
			: m_pMessageQueue( pMessageQueue )
			, m_pProtocal( pProtocal )
		{
		}

		CServerSession::~CServerSession(xgc_void)
		{
		}

		///function	:	�µ�Զ�����ӳɹ�ʱ�Ĵ�����
		///param	:	new_handle	�����ӵ��׽���
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnAccept( network_t new_handle, xgc_uintptr from )
		{
			if( m_pMessageQueue )
			{
				m_nLinkHandle = new_handle;
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, m_nLinkHandle, m_pUserdata );

				pPacket->wd_plus( m_pProtocal->packet_system( EVENT_ACCEPT, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*)&from, sizeof(xgc_uintptr) );
				m_pProtocal->packet_finial( pPacket->base(), pPacket->size() );

				m_pMessageQueue->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	�ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
		///param	:	data	���ܵ�������	����len��Χ�ڿɿ���Ч
		///param	:	len		���ܵ����ݵĳ���	��������iocp��֤����ʵ�ԡ�
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnRecv( const void *data, size_t size )
		{
			if( m_pMessageQueue )
			{
				asio_NetworkPacket *pPacket = asio_NetworkPacket::allocate( size, m_nLinkHandle, m_pUserdata );
				pPacket->putn( data, size );
				pPacket->done( m_nLinkHandle, m_pUserdata );
				m_pMessageQueue->PushMessage( pPacket );
			}
			return 0;
		}

		///function	:	�ɹ������������ݰ����iocp�ϲ㴦��ĺ�����Ŀǰ��ʱδʹ��
		///param	:	data	�ɹ����͵�����
		///param	:	len		�ɹ����͵����ݳ���
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnSend( const void *data, size_t size )
		{
			return 0;
		}

		///function	:	���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
		///param	:	��
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnClose(xgc_void)
		{
			if( m_pMessageQueue )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, m_nLinkHandle, m_pUserdata );

				pPacket->wd_plus( m_pProtocal->packet_system( EVENT_CLOSE, pPacket->base() ) );
				m_pProtocal->packet_finial( pPacket->base(), pPacket->size() );

				m_pMessageQueue->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	iocp���񵽴����ṩ��Ӧ�õĴ������
		///param	:	error	�������
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnError( int error )
		{
			if( m_pMessageQueue )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, m_nLinkHandle, m_pUserdata );

				pPacket->wd_plus( m_pProtocal->packet_system( EVENT_ERROR, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*)&error, sizeof(error) );

				m_pProtocal->packet_finial( pPacket->base(), pPacket->size() );

				m_pMessageQueue->PushMessage( pPacket );
			}
			return 0;
		}

		INetworkSession* create_server_handler( xgc_uintptr lParam, const PacketProtocal* pProtocal )
		{
			return XGC_NEW CServerSession( (IMessageQueue*)lParam, pProtocal );
		}
	}
}