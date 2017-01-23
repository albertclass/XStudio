#include "asio_Header.h"

#define NETBASE_PING		EVENT_PING
#define NETBASE_PONG		EVENT_PONG

namespace xgc
{
	namespace net
	{
		using namespace asio;

		static xgc_size send_buffer_size = 1024 * 256;
		static xgc_size recv_buffer_size = 1024 * 1024;

		// ���÷��ͻ����С
		xgc_void set_send_buffer_size( xgc_size size )
		{
			if( size > 1024 * 1024 )
				send_buffer_size = size;
		}

		// ���ý��ջ����С
		xgc_void set_recv_buffer_size( xgc_size size )
		{
			if( size > 1024 * 256 )
				recv_buffer_size = size;
		}

		asio_Socket::asio_Socket( io_service& s, PacketProtocal* protocal, INetworkSession* holder, xgc_uint16 interval )
			: socket_( s )
			, holder_( holder )
			, handle_( INVALID_NETWORK_HANDLE )
			, send_buffer_( send_buffer_size )
			, recv_buffer_( recv_buffer_size )
			, userdata_( xgc_nullptr )
			, connect_status_( 0 )
			, pingpang_inerval_( interval )
			, pingpong_failed_( 0 )
			, timer_( s )
			, protocal_( protocal )
		{
		}

		asio_Socket::~asio_Socket()
		{
			SAFE_DELETE( holder_ );
		}

		xgc_bool asio_Socket::connect( xgc_lpcstr address, xgc_int16 port, xgc_bool async, xgc_uint16 timeout )
		{
			asio::error_code ec;
			ip::address_v4 addr( ip::address_v4::from_string( address, ec ) );
			if( ec )
				return false;

			if( async )
			{
				if( timeout )
				{
					// ͨ��strand����֤��ʱ���������Ӻ����Ǵ��еġ�
					asio::io_service::strand strand( socket_.get_io_service() );

					timer_.expires_from_now( std::chrono::milliseconds( timeout ) );
					timer_.async_wait(
						strand.wrap( std::bind( &asio_Socket::handle_timeout, shared_from_this(), std::placeholders::_1 ) ) );

					// async connect server.
					socket().async_connect(
						asio::ip::tcp::endpoint( addr, port ),
						strand.wrap( std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) ) );
				}
				else
				{
					// async connect server.
					socket().async_connect(
						asio::ip::tcp::endpoint( addr, port ),
						std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) );
				}

				return true;
			}
			else
			{
				if( !socket_.connect( asio::ip::tcp::endpoint( addr, port ), ec ) )
				{
					accept( 0 );
					return true;
				}
			}

			return false;
		}

		xgc_void asio_Socket::accept( xgc_lpvoid from )
		{
			getSocketMgr().LinkUp( shared_from_this() );

			// ������Դ
			from_ = from;
			// ��������״̬
			connect_status_ = 1;

			if( holder_ )
				holder_->OnAccept( handle_, from );

			socket_.async_read_some(
				buffer( const_cast<xgc_lpstr>(recv_buffer_.base()), protocal_->header_space() ),
				std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			if( pingpang_inerval_ )
			{
				timer_.expires_from_now( std::chrono::milliseconds( pingpang_inerval_ < 1000 ? 1000 : pingpang_inerval_.load() ) );
				timer_.async_wait( bind( &asio_Socket::handle_timer, shared_from_this() ) );
			}
		}

		xgc_void asio_Socket::handle_connect( const asio::error_code& error )
		{
			if( !error )
			{
				accept( 0 );
			}
			else
			{
				if( holder_ )
				{
					holder_->OnError( NET_ERROR_CONNECT );
				}
			}
		}

		xgc_void asio_Socket::handle_timeout( const asio::error_code& error )
		{
			if( is_connected() )
				return;

			if( !error && holder_ )
			{
				holder_->OnError( NET_ERROR_CONNECT_TIMEOUT );
			}
		}

		xgc_void asio_Socket::handle_recv( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				recv_buffer_.wd_plus( translate );

				xgc_size packet_length = protocal_->packet_length( recv_buffer_.base() );

				// �ѽ��յ����ݰ�
				while( packet_length && recv_buffer_.size() >= packet_length )
				{
					// �������ջ������Ϣ��˵�������ӷ����쳣���ж����ӡ�
					if( packet_length >= recv_buffer_.capacity() )
					{
						XGC_ASSERT( false );
						if( holder_ )
							holder_->OnError( NET_ERROR_NOT_ENOUGH_MEMROY );

						close();
						return;
					}

					filter_packet( recv_buffer_.base(), packet_length );

					// �������յ����ݰ�
					recv_buffer_.popn( packet_length );

					// ���¼�������ȣ�����һ�����ն���������⡣
					packet_length = protocal_->packet_length( recv_buffer_.base() );
				}

				// ������ȡЭ����
				socket_.async_read_some(
					buffer( const_cast<xgc_lpstr>(recv_buffer_.wd_ptr()), recv_buffer_.space() ),
					std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
			else
			{
				if( holder_ )
					holder_->OnError( error.value() );

				close();
			}
		}

		xgc_uint32 asio_Socket::filter_packet( xgc_lpstr data, xgc_size size )
		{
			// filter packet
			xgc_uint32 ret = protocal_->packet_filter( data );
			switch( ret )
			{
			case FILTER_SYSTEM_PING:
				{
					xgc_lpcstr c = data + protocal_->header_space();

					xgc_char pong[64] = { 0 };
					xgc_size length = protocal_->packet_system( EVENT_PONG, pong );
					*(clock_t*) (pong + length) = clock();
					length += sizeof( clock_t );

					protocal_->packet_finial( pong, length );
					send( pong, length );
				}
				break;
			case FILTER_SYSTEM_PONG:
				{
					pingpong_ = clock() - *(clock_t*) (data + protocal_->header_space());

					// clear failed_count so socket timeout value are reset;
					pingpong_failed_ = 0;
				}
				break;
			case FILTER_PASS:
				{
					if( holder_ )
					{
						holder_->OnRecv( data, size );
					}
				}
				break;
			case FILTER_REFUSE:
				break;
			}

			return ret;
		}

		xgc_void asio_Socket::send( xgc_lpvoid data, xgc_size size, xgc_bool last /*= false*/ )
		{
			XGC_ASSERT( data && size );

			std::lock_guard< std::mutex > guard( send_buffer_lock );

			// ���ݷ��뷢�ͻ���
			if( send_buffer_.putn( data, size ) != size )
			{
				if( holder_ )
					holder_->OnError( NET_ERROR_SEND_BUFFER_FULL );

				close();
				return;
			}

			// ���ͺ��Ƿ�Ͽ�����
			if( last )
			{
				xgc_uint16 status = 1;
				connect_status_.compare_exchange_strong( status, 2 ); // �ȴ��ر�
			}

			// ���ͺͷ�����ɿ��ܲ���ͬһ�߳���
			if( send_buffer_.size() == size )
			{
				socket_.async_send(
					buffer( send_buffer_.base(), send_buffer_.size() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}

		xgc_void asio_Socket::handle_send( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				std::lock_guard< std::mutex > guard( send_buffer_lock );

				// ����ָ���ͷŽ�������Ϣ���屻�ͷ� 
				if( holder_ )
					holder_->OnSend( send_buffer_.base(), translate );

				send_buffer_.popn( translate );

				// Ҫ��Ͽ�����
				if( connect_status_ == 2 )
				{
					close();
				}
				else if( send_buffer_.size() )
				{
					socket_.async_send(
						buffer( send_buffer_.base(), send_buffer_.size() ),
						std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
				}
			}
			else
			{
				if( holder_ )
					holder_->OnError( error.value() );

				close();
			}
		}

		xgc_void asio_Socket::close()
		{
			asio::error_code ec;

			xgc_uint16 status_1 = 1;
			xgc_uint16 status_2 = 2;
			if( connect_status_.compare_exchange_strong( status_1, 0 ) || 
				connect_status_.compare_exchange_strong( status_2, 0 ) )
			{
				connect_status_ = 0;

				socket_.shutdown( ip::tcp::socket::shutdown_both, ec );
				socket_.close( ec );

				timer_.cancel( ec );

				if( holder_ )
					holder_->OnClose();

				getSocketMgr().LinkDown( shared_from_this() );
			}
		}

		xgc_uintptr asio_Socket::get_socket_info( xgc_int16 mask, xgc_byte * data )
		{
			xgc_byte* ptr = data;
			try
			{
				if( mask & NET_LOCAL_ADDRESS )
				{
					Param_QueryHandleInfo::endpoint* point = (Param_QueryHandleInfo::endpoint*)ptr;
					point->addr = socket_.local_endpoint().address().to_v4().to_ulong();
					point->port = socket_.local_endpoint().port();

					ptr = (xgc_byte*) (point + 1);
				}

				if( mask & NET_REMOT_ADDRESS )
				{
					Param_QueryHandleInfo::endpoint* point = (Param_QueryHandleInfo::endpoint*)ptr;
					point->addr = socket_.remote_endpoint().address().to_v4().to_ulong();
					point->port = socket_.remote_endpoint().port();

					ptr = (xgc_byte*) (point + 1);
				}

				if( mask & NET_LOCAL_PING )
				{
					*(xgc_uint32*) (ptr) = pingpong_;
					ptr += sizeof( xgc_uint32 );
				}
			}
			catch( asio::error_code& ec )
			{
				ec;
				return -1;
			}

			return ptr - data;
		}

		xgc_void asio_Socket::handle_timer()
		{
			clock_t c = clock();

			xgc_char buf[64] = { 0 };
			// send ping message
			xgc_size bytes = protocal_->packet_system( EVENT_PING, buf );
			*(clock_t*) (buf + bytes) = c;
			bytes += sizeof( clock_t );
			protocal_->packet_finial( buf, bytes );

			send( buf, bytes );

			if( is_connected() && pingpang_inerval_ )
			{
				timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( pingpang_inerval_.load(), 1000 ) ) );
				timer_.async_wait( std::bind( &asio_Socket::handle_timer, shared_from_this() ) );

				// check socket timeout
				if( pingpong_failed_ > 5 )
				{
					close();
				}
				else
				{
					pingpong_failed_++;
				}
			}
		}
	}
}