#include "asio_Header.h"
#include "NetSession.h"

namespace xgc
{
	namespace net
	{
		CSession::CSession( MessageQueuePtr Queue )
			: ping_( 0 )
			, pingfailed_( 0 )
			, handle_( INVALID_NETWORK_HANDLE )
			, userdata_( xgc_nullptr )
			, queue_( Queue )
		{
		}

		CSession::~CSession()
		{
		}

		int CSession::OnParsePacket( const void * data, xgc_size size )
		{
			return (int)packet_length((char*)data);
		}

		///function	:	�ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
		///param	:	data	���ܵ�������	����len��Χ�ڿɿ���Ч
		///param	:	len		���ܵ����ݵĳ���	��������iocp��֤����ʵ�ԡ�
		///return	:	0	�ɹ�	-1 ʧ��
		int CSession::OnRecv( const void *data, xgc_size size )
		{
			// filter packet
			xgc_uint32 ret = packet_filter( (char*)data );
			switch( ret )
			{
			case FILTER_SYSTEM_PING:
				{
					xgc_lpcstr c = (char*)data + header_space();

					xgc_char pong[64] = { 0 };
					xgc_size length = packet_system( EVENT_PONG, pong );
					*(clock_t*) (pong + length) = clock();
					length += sizeof( clock_t );

					packet_finial( pong, length );
					SendPacket( handle_, pong, length );
				}
				break;
			case FILTER_SYSTEM_PONG:
				{
					ping_ = clock() - *(clock_t*) ((char*)data + header_space());
					pingfailed_ = 0;
				}
				break;
			case FILTER_PASS:
				{
					if( queue_ )
					{
						asio_NetworkPacket *pPacket = asio_NetworkPacket::allocate( size, handle_ );
						pPacket->putn( data, size );
						pPacket->rd_reset( header_space() );
						pPacket->done( handle_, userdata_ );
						queue_->PushMessage( pPacket );
					}
				}
				break;
			case FILTER_REFUSE:
				break;
			}

			return 0;
		}

		///function	:	���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
		///param	:	��
		///return	:	0	�ɹ�	-1 ʧ��
		int CSession::OnClose()
		{
			if( queue_ )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( packet_system( EVENT_CLOSE, pPacket->base() ) );
				packet_finial( pPacket->base(), pPacket->length() );

				queue_->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	�ɹ������������ݰ����iocp�ϲ㴦��ĺ�����Ŀǰ��ʱδʹ��
		///param	:	data	�ɹ����͵�����
		///param	:	len		�ɹ����͵����ݳ���
		///return	:	0	�ɹ�	-1 ʧ��
		int CSession::OnSend( const void *data, xgc_size size )
		{
			return 0;
		}

		///function	:	iocp���񵽴����ṩ��Ӧ�õĴ������
		///param	:	error	�������
		///return	:	0	�ɹ�	-1 ʧ��
		int CSession::OnError( int error )
		{
			if( queue_ )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( packet_system( EVENT_ERROR, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*) &error, sizeof( error ) );
				packet_finial( pPacket->base(), pPacket->length() );

				queue_->PushMessage( pPacket );
			}

			return 0;
		}

		int CSession::OnAlive()
		{
			xgc_char buf[256] = { 0 };
			// send ping message
			xgc_size bytes = packet_system( EVENT_PING, buf );
			XGC_ASSERT( bytes < XGC_COUNTOF( buf ) );

			// fill timestamp
			*(clock_t*) (buf + bytes) = clock();
			bytes += sizeof( clock_t );

			XGC_ASSERT( bytes < XGC_COUNTOF( buf ) );
			packet_finial( buf, bytes );

			SendPacket( handle_, buf, bytes );

			// timeout
			return (int)pingfailed_++;
		}

		///////////////////////////////////////////////////////////////////////////
		CClientSession::CClientSession( MessageQueuePtr QueuePtr )
			: CSession( QueuePtr )
		{
		}

		CClientSession::~CClientSession()
		{
		}

		///function	:	�µ�Զ�����ӳɹ�ʱ�Ĵ�����
		///param	:	new_handle	�����ӵ��׽���
		///return	:	0	�ɹ�	-1 ʧ��
		int CClientSession::OnAccept( network_t new_handle, xgc_lpvoid from )
		{
			if( queue_ )
			{
				handle_ = new_handle;
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, new_handle );

				pPacket->reset( packet_system( EVENT_CONNECT, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*) &from, sizeof( xgc_lpvoid ) );
				packet_finial( pPacket->base(), pPacket->length() );

				queue_->PushMessage( pPacket );
			}

			return 0;
		}

		///////////////////////////////////////////////////////////////////////////
		CServerSession::CServerSession( MessageQueuePtr QueuePtr )
			: CSession( QueuePtr )
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
			if( queue_ )
			{
				handle_ = new_handle;
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( packet_system( EVENT_ACCEPT, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*)&from, sizeof(xgc_uintptr) );
				packet_finial( pPacket->base(), pPacket->length() );

				queue_->PushMessage( pPacket );
			}

			return 0;
		}
	}
}