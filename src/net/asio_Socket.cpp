#include "Header.h"

#define NETBASE_PING		EVENT_PING
#define NETBASE_PONG		EVENT_PONG

namespace xgc
{
	namespace net
	{
		using namespace asio;

		static xgc_size send_buffer_size = 1024 * 16;
		static xgc_size recv_buffer_size = 1024 * 16;

		static xgc_size send_package_max = 1024 * 4;
		static xgc_size recv_package_max = 1024 * 4;

		// 设置发送缓冲大小
		xgc_void set_send_buffer_size( xgc_size size )
		{
			if( size > 1024 * 1024 )
				send_buffer_size = size;
		}

		// 设置接收缓冲大小
		xgc_void set_recv_buffer_size( xgc_size size )
		{
			if( size > 1024 * 256 )
				recv_buffer_size = size;
		}

		asio_Socket::asio_Socket( io_service& s, xgc_lpvoid userdata, xgc_uint16 timeout )
			: socket_( s )
			, handle_( INVALID_NETWORK_HANDLE )
			, send_buffer_( send_buffer_size )
			, recv_buffer_( recv_buffer_size )
			, connect_status_( 0 )
			, timeout_( timeout )
			, timer_( s )
			, from_( xgc_nullptr )
			, userdata_( userdata )
		{
		}

		asio_Socket::~asio_Socket()
		{
		}

		xgc_void asio_Socket::hangup( xgc_lpvoid from )
		{
			getSocketMgr().LinkUp( shared_from_this() );

			// 设置来源
			from_ = from;

			make_event( EVENT_HANGUP, (xgc_uint64)from_ );
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
					asio::io_service::strand strand( socket_.get_io_service() );

					timer_.expires_from_now( std::chrono::milliseconds( timeout ) );
					timer_.async_wait(
						strand.wrap( std::bind( &asio_Socket::handle_timeout, shared_from_this(), std::placeholders::_1 ) ) );

					// async connect server.
					socket_.async_connect(
						asio::ip::tcp::endpoint( addr, port ),
						strand.wrap( std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) ) );
				}
				else
				{
					// async connect server.
					socket_.async_connect(
						asio::ip::tcp::endpoint( addr, port ),
						std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) );
				}

				return true;
			}
			else
			{
				if( !socket_.connect( asio::ip::tcp::endpoint( addr, port ), ec ) )
				{
					accept( EVENT_CONNECT );
					return true;
				}
			}

			return false;
		}

		xgc_void asio_Socket::accept( int event )
		{
			make_event( event, (xgc_uint64)from_ );

			connect_status_ = 1;

			socket_.async_read_some(
				buffer( recv_buffer_.end(), recv_buffer_.space( recv_package_max ) ),
				std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			if( timeout_ )
			{
				timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( 500, timeout_.load() ) ) );
				timer_.async_wait( bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
			}
		}

		xgc_void asio_Socket::handle_connect( const asio::error_code& error )
		{
			if( !error )
			{
				accept( EVENT_CONNECT );
			}
			else
			{
				make_event( EVENT_ERROR, NET_ERROR_CONNECT );
			}
		}

		xgc_void asio_Socket::handle_timeout( const asio::error_code& error )
		{
			if( is_connected() )
				return;

			if( !error )
			{
				make_event( EVENT_ERROR, NET_ERROR_CONNECT_TIMEOUT );
			}
		}

		xgc_void asio_Socket::handle_recv( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				recv_buffer_.push( translate );

				INetworkSession* session = (INetworkSession*) userdata_;
				xgc_size packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );

				// 已接收到数据包
				while( packet_length && recv_buffer_.length() >= packet_length )
				{
					// 超出接收缓冲的消息包说明该连接发生异常，中断连接。
					if( packet_length >= recv_buffer_.space() )
					{
						XGC_ASSERT( false );
						make_event( EVENT_ERROR, NET_ERROR_NOT_ENOUGH_MEMROY );

						close();
						return;
					}

					// 将数据包发送到队列
					EventHeader evt;
					evt.handle = handle_;
					evt.event = EVENT_DATA;
					evt.bring = packet_length;
						
					auto packet = CNetworkPacket::allocate( sizeof(EventHeader) + packet_length );
					packet->putn( &evt, sizeof(evt) );
					packet->putn( recv_buffer_.begin(), packet_length );

					getSocketMgr().Push( packet );

					// 弹出已收的数据包
					recv_buffer_.pop( packet_length );

					// 重新计算包长度，处理一个接收多个包的问题。
					packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );
				}

				// 继续读取协议体
				socket_.async_read_some(
					buffer( recv_buffer_.end(), recv_buffer_.space( recv_package_max ) ),
					std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
			else
			{
				make_event( EVENT_ERROR, error.value() );

				close();
			}
		}

		xgc_void asio_Socket::send( xgc_lpvoid data, xgc_size size, xgc_bool last /*= false*/ )
		{
			XGC_ASSERT( data && size );

			std::lock_guard< std::mutex > guard( send_buffer_lock );

			// 数据放入发送缓冲
			if( send_buffer_.put( data, size ) != size )
			{
				make_event( EVENT_ERROR, NET_ERROR_SEND_BUFFER_FULL );

				close();
				return;
			}

			// 发送后是否断开连接
			if( last )
			{
				xgc_uint16 status = 1;
				connect_status_.compare_exchange_strong( status, 2 ); // 等待关闭
			}

			// 发送和发送完成可能不在同一线程中
			if( send_buffer_.length() == size )
			{
				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}

		xgc_void asio_Socket::send( const std::list< std::tuple< xgc_lpvoid, xgc_size > >& buffers )
		{
			xgc_size total = 0;
			std::lock_guard< std::mutex > guard( send_buffer_lock );

			for( auto &t : buffers )
			{
				auto data = std::get< xgc_lpvoid >( t ) ;
				auto size = std::get< xgc_size >( t ) ;

				// 数据放入发送缓冲
				if( send_buffer_.put( data, size ) != size )
				{
					make_event( EVENT_ERROR, NET_ERROR_SEND_BUFFER_FULL );

					close();
					return;
				}

				total += size;
			}

			// 发送和发送完成可能不在同一线程中
			if( send_buffer_.length() == total )
			{
				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}


		xgc_void asio_Socket::handle_send( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				std::lock_guard< std::mutex > guard( send_buffer_lock );

				// 智能指针释放将导致消息缓冲被释放 
				// make_event( EVENT_SEND, translate );

				send_buffer_.pop( translate );

				// 要求断开连接
				if( connect_status_ == 2 )
				{
					close();
				}
				else if( send_buffer_.length() )
				{
					socket_.async_send(
						buffer( send_buffer_.begin(), send_buffer_.length() ),
						std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
				}
			}
			else
			{
				make_event( EVENT_ERROR, error.value() );

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

				make_event( EVENT_CLOSE, 0 );
				getSocketMgr().LinkDown( shared_from_this() );
			}
		}

		xgc_ulong asio_Socket::get_socket_info( xgc_int16 mask, xgc_byte * data )
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
			}
			catch( asio::error_code& ec )
			{
				ec;
				return -1;
			}

			return xgc_ulong(ptr - data);
		}

		xgc_void asio_Socket::handle_timer( const asio::error_code& e )
		{
			if( e != asio::error::operation_aborted )
			{
				if( is_connected() && timeout_ )
				{
					// check socket timeout
					make_event( EVENT_PING, 0 );

					timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( timeout_.load(), 500 ) ) );
					timer_.async_wait( std::bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
				}
			}
		}

		xgc_void asio_Socket::make_event( xgc_uint32 event, xgc_uint64 bring )
		{
			EventHeader evt;
			evt.handle = handle_;
			evt.event = event;
			evt.bring = bring;

			getSocketMgr().Push( CNetworkPacket::allocate( &evt, sizeof( evt ), handle_ ) );
		}
	}
}