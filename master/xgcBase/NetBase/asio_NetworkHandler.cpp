#include "Netbase.h"
#include "asio_Header.h"
#include "asio_NetworkHandler.h"
#include "asio_NetworkPacket.h"
#include "asio_Server.h"

#include "HandlerManager.h"

#define NETBASE_PING		EVENT_PING
#define NETBASE_PONG		EVENT_PONG

namespace XGC
{
	namespace net
	{
		using namespace asio;
		using boost::posix_time::seconds;
		using boost::posix_time::milliseconds;

		static xgc_size send_buffer_size = 1024 * 128;
		static xgc_size recv_buffer_size = 1024 * 1024;

		// 设置发送缓冲大小
		xgc_void set_send_buffer_size( xgc_size size )
		{
			if( size > 1024 * 1024 )
				send_buffer_size = size;
		}

		// 设置接收缓冲大小
		xgc_void set_recv_buffer_size( xgc_size size )
		{
			if( size > 1024 * 128 )
				recv_buffer_size = size;
		}

		asio_Socket::asio_Socket( io_service& s, PacketProtocal* protocal, INetworkSession* holder, xgc_uint16 interval )
			: socket_( s )
			, holder_( holder )
			, handler_( INVALID_NETWORK_HANDLE )
			, send_buffer_( send_buffer_size )
			, recv_buffer_( recv_buffer_size )
			, network_( xgc_nullptr )
			, userdata_( xgc_nullptr )
			, disconnect_( false )
			, pingpang_inerval_( interval )
			, failed_count_( 0 )
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
					// 通过strand对象保证超时函数和连接函数是串行的。
					asio::strand strand( socket_.get_io_service() );

					timer_.expires_from_now( seconds( timeout ) );
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

		xgc_void asio_Socket::accept( xgc_uintptr from )
		{
			network_ = XGC_NEW asio_NetworkHandler( shared_from_this(), from );
			if( network_ == xgc_nullptr )
			{
				socket_.close();
				return;
			}

			if( holder_ )
				holder_->OnAccept( handler_, from );

			socket_.async_read_some(
				buffer( const_cast<xgc_byte*>(recv_buffer_.base()), protocal_->header_space() ),
				std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			if( pingpang_inerval_ )
			{
				timer_.expires_from_now( millisec( pingpang_inerval_ < 1000 ? 1000 : pingpang_inerval_ ) );
				timer_.async_wait( bind( &asio_Socket::handle_timer, shared_from_this() ) );
			}
		}

		xgc_void asio_Socket::handle_connect( const asio::error_code& error )
		{
			if (!error)
			{
				accept( 0 );
			}
			else
			{
				if( holder_ )
				{
					holder_->OnError( NET_ERROR_CONNET );
				}
			}
		}

		xgc_void asio_Socket::handle_timeout( const asio::error_code& error )
		{
			if( is_connected() )
				return;

			if( !error && holder_ )
			{
				holder_->OnError( NET_ERROR_CONNET_TIMEOUT );
			}
		}

		xgc_void asio_Socket::handle_recv( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				recv_buffer_.wd_plus( translate );

				xgc_size packet_length = protocal_->packet_length( recv_buffer_.base() );

				// 已接收到数据包
				while( packet_length && recv_buffer_.size() >= packet_length )
				{
					// 超出接收缓冲的消息包说明该连接发生异常，中断连接。
					if( packet_length >= recv_buffer_.capacity() )
					{
						XGC_ASSERT( FALSE );
						if( holder_ )
							holder_->OnError( NET_ERROR_NOT_ENOUGH_MEMROY );

						close();
						return;
					}

					filter_packet( recv_buffer_.base(), packet_length );

					// 弹出已收的数据包
					recv_buffer_.popn( packet_length );

					// 重新计算包长度，处理一个接收多个包的问题。
					packet_length = protocal_->packet_length( recv_buffer_.base() );
				}

				// 继续读取协议体
				socket_.async_read_some(
					buffer( const_cast< xgc_byte* >( recv_buffer_.wd_ptr() ), packet_length ),
					std::bind(&asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
			else
			{
				if( holder_ )
					holder_->OnError( error.value() );

				close();
			}
		}

		xgc_uint32 asio_Socket::filter_packet( const xgc_byte *data, xgc_size size )
		{
			// filter packet
			xgc_uint32 ret = protocal_->packet_filter( data );
			switch( ret )
			{
			case FILTER_SYSTEM_PING:
				{
					const xgc_byte* c = data + protocal_->header_space();

					xgc_byte pong[64] = { 0 };
					xgc_size length = protocal_->packet_system( EVENT_PONG, pong );
					*(clock_t*) (pong + length) = clock();
					length += sizeof( clock_t );

					protocal_->packet_finial( pong, length );
					send( pong, length );
				}
				break;
			case FILTER_SYSTEM_PONG:
				{
					if( network_ )
						network_->SetHandlePing( clock() - *(clock_t*) (data + protocal_->header_space()) );

					failed_count_ = 0;
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

			// clear failed_count so socket timeout value are reset;
			failed_count_ = 0;
			return ret;
		}

		xgc_void asio_Socket::send( const void* data, size_t size )
		{
			XGC_ASSERT( data && size );

			std::lock_guard< std::mutex > guard( send_buffer_lock );

			// 数据放入发送缓冲
			if( send_buffer_.putn( data, size ) != size )
			{
				if( holder_ )
					holder_->OnError( NET_ERROR_SEND_BUFFER_FULL );

				close();
				return;
			}

			// 发送和发送完成可能不在同一线程中
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

				// 智能指针释放将导致消息缓冲被释放 
				if( holder_ )
					holder_->OnSend( send_buffer_.base(), translate );

				send_buffer_.popn( translate );

				// 要求断开连接
				if( disconnect_ )
				{
					close();
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
			asio_NetworkHandler* network = ( asio_NetworkHandler* )InterlockedExchangePointer( (volatile PVOID*) &network_, 0 );
			if( network )
			{
				asio::error_code ec;

				socket_.shutdown(ip::tcp::socket::shutdown_both, ec);
				socket_.close(ec);
				//timer_.cancel(ec);

				if( holder_ )
					holder_->OnClose();

				SAFE_DELETE( network ); // 由于智能指针的解引用，其作用相当于delete this
			}
		}

		xgc_void asio_Socket::handle_timer()
		{
			clock_t c = clock();

			xgc_byte buf[64] = { 0 };
			// send ping message
			xgc_size bytes = protocal_->packet_system( EVENT_PING, buf );
			*(clock_t*)(buf + bytes) = c;
			bytes += sizeof( clock_t );
			protocal_->packet_finial( buf, bytes );

			send( buf, bytes );

			timer_.expires_from_now( milliseconds(pingpang_inerval_) );
			asio_NetworkHandler* network = ( asio_NetworkHandler* )InterlockedCompareExchangePointer((PVOID volatile *)&network_, 0, 0 );
			if( network )
			{
				timer_.async_wait( std::bind( &asio_Socket::handle_timer, shared_from_this() ) );
			}

			// check socket timeout
			if( failed_count_ > 5 )
			{
				close();
			}
			else
			{
				failed_count_++;
			}
		}
		
		//////////////////////////////////////////////////////////////////////////

		asio_NetworkHandler::asio_NetworkHandler( asio_SocketPtr& sock, xgc_uintptr from )
			: m_pSocket( sock )
			, m_nPingPong( 0xffffffff )
			, _from( from )
		{
			getHandleManager().LinkUp( this );
		}

		asio_NetworkHandler::~asio_NetworkHandler(xgc_void)
		{
			// 两个线程同时Close的时候一个线程删除了network_，会导致崩溃的原因是由于HandleManager没有使用智能指针。
			getHandleManager().LinkDown( this );
			// 保证解锁
			std::unique_lock< std::mutex > _guard( m_oCriResource, std::try_to_lock_t() );
		}

		xgc_uintptr asio_NetworkHandler::GetHandleInfo( xgc_int16 mask, xgc_byte* data )
		{
			ip::tcp::socket& s = m_pSocket->socket();

			size_t wd = 0;
			try
			{
				if( mask & 0x0001 )
				{
					*((xgc_uint32*)(data+wd)) = s.local_endpoint().address().to_v4().to_ulong(); 
					wd += sizeof( xgc_uint32 );
					*((xgc_uint16*)(data+wd)) = s.local_endpoint().port(); 
					wd += sizeof( xgc_uint16 );
				}

				if( mask & 0x0002 )
				{
					*((xgc_uint32*)(data+wd)) = s.remote_endpoint().address().to_v4().to_ulong();
					wd += sizeof( xgc_uint32 );
					*((xgc_uint16*)(data+wd)) = s.remote_endpoint().port();
					wd += sizeof( xgc_uint16 );
				}

				if( mask & 0x0004 )
				{
					*((xgc_uint32*)(data+wd)) = m_nPingPong;
					wd += sizeof( xgc_uint32 );
				}
			}
			catch( asio::error_code& ec )
			{
				ec;
				return -1;
			}
			return wd;
		}
	}
}