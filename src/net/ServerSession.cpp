#include "asio_Header.h"
#include "ServerSession.h"

namespace xgc
{
	namespace net
	{
		CServerSession::CServerSession( MessageQueuePtr queue_ptr, const PacketProtocal* protocal )
			: queue_ptr( queue_ptr )
			, protocol_( protocal )
		{
		}

		CServerSession::~CServerSession()
		{
		}

		///function	:	�µ�Զ�����ӳɹ�ʱ�Ĵ�����
		///param	:	new_handle	�����ӵ��׽���
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnAccept( network_t new_handle, xgc_lpvoid from )
		{
			if( queue_ptr )
			{
				handle_ = new_handle;
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( protocol_->packet_system( EVENT_ACCEPT, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*)&from, sizeof(xgc_uintptr) );
				protocol_->packet_finial( pPacket->base(), pPacket->length() );

				queue_ptr->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	�ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
		///param	:	data	���ܵ�������	����len��Χ�ڿɿ���Ч
		///param	:	len		���ܵ����ݵĳ���	��������iocp��֤����ʵ�ԡ�
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnRecv( const void *data, size_t size )
		{
			if( queue_ptr )
			{
				asio_NetworkPacket *pPacket = asio_NetworkPacket::allocate( size, handle_ );
				pPacket->putn( data, size );
				pPacket->rd_reset( protocol_->header_space() );
				pPacket->done( handle_, userdata_ );
				queue_ptr->PushMessage( pPacket );
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
		int CServerSession::OnClose()
		{
			if( queue_ptr )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( protocol_->packet_system( EVENT_CLOSE, pPacket->base() ) );
				protocol_->packet_finial( pPacket->base(), pPacket->length() );

				queue_ptr->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	iocp���񵽴����ṩ��Ӧ�õĴ������
		///param	:	error	�������
		///return	:	0	�ɹ�	-1 ʧ��
		int CServerSession::OnError( int error )
		{
			if( queue_ptr )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( protocol_->packet_system( EVENT_ERROR, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*)&error, sizeof(error) );

				protocol_->packet_finial( pPacket->base(), pPacket->length() );

				queue_ptr->PushMessage( pPacket );
			}
			return 0;
		}
	}
}