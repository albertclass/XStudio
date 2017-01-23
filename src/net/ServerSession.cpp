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

		///function	:	新的远端连接成功时的处理函数
		///param	:	new_handle	新连接的套接字
		///return	:	0	成功	-1 失败
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

		///function	:	成功接受到网络数据包后提交到iocp上层应用处理的函数
		///param	:	data	接受到的数据	在中len范围内可靠有效
		///param	:	len		接受到数据的长度	此数据由iocp保证其真实性。
		///return	:	0	成功	-1 失败
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

		///function	:	成功发送网络数据包后给iocp上层处理的函数，目前暂时未使用
		///param	:	data	成功发送的数据
		///param	:	len		成功发送的数据长度
		///return	:	0	成功	-1 失败
		int CServerSession::OnSend( const void *data, size_t size )
		{
			return 0;
		}

		///function	:	连接关闭时的处理函数。重载此函数时需在做完应用需要做的事情后将连接删除或重置
		///param	:	无
		///return	:	0	成功	-1 失败
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

		///function	:	iocp捕获到错误并提供给应用的处理机会
		///param	:	error	错误代码
		///return	:	0	成功	-1 失败
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