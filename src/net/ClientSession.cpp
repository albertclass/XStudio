#include "asio_Header.h"
#include "ClientSession.h"

namespace xgc
{
	namespace net
	{
		CClientSession::CClientSession( MessageQueuePtr queue_ptr, const PacketProtocal* protocal )
			: queue_ptr_( queue_ptr )
			, protocol_( protocal )
		{
		}

		CClientSession::~CClientSession( xgc_void )
		{
		}

		///function	:	新的远端连接成功时的处理函数
		///param	:	new_handle	新连接的套接字
		///return	:	0	成功	-1 失败
		int CClientSession::OnAccept( network_t new_handle, xgc_lpvoid from )
		{
			if( queue_ptr_ )
			{
				handle_ = new_handle;
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, new_handle );

				pPacket->reset( protocol_->packet_system( EVENT_CONNECT, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*) &from, sizeof( xgc_uintptr ) );
				protocol_->packet_finial( pPacket->base(), pPacket->length() );

				queue_ptr_->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	成功接受到网络数据包后提交到iocp上层应用处理的函数
		///param	:	data	接受到的数据	在中len范围内可靠有效
		///param	:	len		接受到数据的长度	此数据由iocp保证其真实性。
		///return	:	0	成功	-1 失败
		int CClientSession::OnRecv( const void *data, size_t size )
		{
			if( queue_ptr_ )
			{
				asio_NetworkPacket *pPacket = asio_NetworkPacket::allocate( size, handle_ );
				pPacket->putn( data, size );
				pPacket->rd_reset( protocol_->header_space() );
				pPacket->done( handle_, userdata_ );

				queue_ptr_->PushMessage( pPacket );
			}
			return 0;
		}

		///function	:	连接关闭时的处理函数。重载此函数时需在做完应用需要做的事情后将连接删除或重置
		///param	:	无
		///return	:	0	成功	-1 失败
		int CClientSession::OnClose( xgc_void )
		{
			if( queue_ptr_ )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( protocol_->packet_system( EVENT_CLOSE, pPacket->base() ) );
				protocol_->packet_finial( pPacket->base(), pPacket->length() );

				queue_ptr_->PushMessage( pPacket );
			}

			return 0;
		}

		///function	:	成功发送网络数据包后给iocp上层处理的函数，目前暂时未使用
		///param	:	data	成功发送的数据
		///param	:	len		成功发送的数据长度
		///return	:	0	成功	-1 失败
		int CClientSession::OnSend( const void *data, size_t size )
		{
			return 0;
		}

		///function	:	iocp捕获到错误并提供给应用的处理机会
		///param	:	error	错误代码
		///return	:	0	成功	-1 失败
		int CClientSession::OnError( int error )
		{
			if( queue_ptr_ )
			{
				asio_NetworkPacket* pPacket = asio_NetworkPacket::allocate( 32, handle_ );

				pPacket->reset( protocol_->packet_system( EVENT_ERROR, pPacket->base() ) );
				pPacket->putn( (const xgc_byte*) &error, sizeof( error ) );
				protocol_->packet_finial( pPacket->base(), pPacket->length() );

				queue_ptr_->PushMessage( pPacket );
			}

			return 0;
		}
	}
}