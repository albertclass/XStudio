#include "Header.h"

#define NETBASE_PING		EVENT_PING
#define NETBASE_PONG		EVENT_PONG

namespace xgc
{
	namespace net
	{
		using namespace asio;

		static xgc_size g_send_buffer_size = 1024 * 16;
		static xgc_size g_recv_buffer_size = 1024 * 16;

		static xgc_size g_send_packet_max = 1024 * 4;
		static xgc_size g_recv_packet_max = 1024 * 4;

		// 设置发送缓冲大小
		xgc_void set_send_buffer_size( xgc_size size )
		{
			if( size > 1024 * 16 )
				g_send_buffer_size = size;
		}

		// 设置接收缓冲大小
		xgc_void set_recv_buffer_size( xgc_size size )
		{
			if( size > 1024 * 16 )
				g_recv_buffer_size = size;
		}

		// 设置发送数据大小
		xgc_void set_send_packet_size( xgc_size size )
		{
			if( size > 1024 * 8 )
				g_send_packet_max = size;
		}

		// 设置接收数据大小
		xgc_void set_recv_packet_size( xgc_size size )
		{
			if( size > 1024 * 8 )
				g_recv_packet_max = size;
		}

		asio_Socket::asio_Socket( io_service& s, xgc_uint16 timeout, xgc_lpvoid userdata, xgc_lpvoid from )
			: socket_( s )
			, connect_info_( xgc_nullptr )
			, handle_( INVALID_NETWORK_HANDLE )
			, connect_status_( 0 )
			, operator_count_( 0 )
			, timeout_( timeout )
			, timer_( s )
			, from_( from )
			, userdata_( userdata )
			, recv_packet_max_( g_recv_packet_max )
			, send_packet_max_( g_send_packet_max )
		{
		}

		asio_Socket::~asio_Socket()
		{
			SAFE_DELETE( connect_info_ );
		}

		xgc_bool asio_Socket::set_buffer_size( set_option option, xgc_size buffer_size )
		{
			if( option == e_recv )
				return recv_buffer_.open( buffer_size );

			if( option == e_send )
				return send_buffer_.open( buffer_size );

			if( option == e_both )
				return recv_buffer_.open( buffer_size ) && send_buffer_.open( buffer_size );

			return false;
		}

		xgc_void asio_Socket::set_packet_max( set_option option, xgc_size packet_max )
		{
			if( option == e_recv )
				send_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );

			if( option == e_send )
				recv_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );

			if( option == e_both )
			{
				send_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );
				recv_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );
			}
		}

		xgc_void asio_Socket::pending( xgc_lpvoid from )
		{
			// 设置来源
			from_ = from;
			connect_status_ = 1;

			make_event( EVENT_PENDING, 0, from_ );
		}

		xgc_bool asio_Socket::connect( xgc_lpcstr address, xgc_int16 port, connect_options *options )
		{
			// 错误码
			asio::error_code ec;

			asio::ip::tcp::endpoint ep( ip::address_v4::from_string( address, ec ), port );
			XGC_ASSERT_RETURN( !ec, false );

			// 创建连接对象
			connect_info_ = XGC_NEW connect_info;
			connect_info_->address = ep;
			if( options )
			{
				connect_info_->timeout = options->timeout;
				connect_info_->is_async = options->is_async ? 1 : 0;
				connect_info_->is_reconnect_timeout = options->is_reconnect_timeout ? 1 : 0;
				connect_info_->is_reconnect_passive = options->is_reconnect_passive ? 1 : 0;
			}
			else
			{
				connect_info_->timeout = 0;
				connect_info_->is_async = 0;
				connect_info_->is_reconnect_timeout = 0;
				connect_info_->is_reconnect_passive = 0;
			}

			if( options && options->recv_buffer_size )
				set_buffer_size( e_recv, options->recv_buffer_size );
			if( options && options->send_buffer_size )
				set_buffer_size( e_send, options->send_buffer_size );

			if( options && options->recv_packet_max )
				set_packet_max( e_recv, options->recv_packet_max );
			if( options && options->send_buffer_size )
				set_packet_max( e_send, options->send_packet_max );
			

			return connect();
		}

		xgc_bool asio_Socket::connect()
		{
			if( connect_info_->is_async )
			{
				if( connect_info_->timeout )
				{
					// 通过strand对象保证超时函数和连接函数是串行的。
					asio::io_service::strand strand( socket_.get_io_service() );

					timer_.expires_from_now( std::chrono::milliseconds( connect_info_->timeout ) );
					timer_.async_wait(
						strand.wrap( std::bind( &asio_Socket::handle_timeout, shared_from_this(), std::placeholders::_1 ) ) );

					// async connect server.
					socket_.async_connect( connect_info_->address, strand.wrap( std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) ) );
				}
				else
				{
					// async connect server.
					socket_.async_connect( connect_info_->address, std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) );
				}

				return true;
			}
			else
			{
				asio::error_code ec;

				if( socket_.connect( connect_info_->address, ec ) )
					accept( EVENT_CONNECT );
				else
					close();

				return !ec;
			}
		}

		xgc_void asio_Socket::accept( int event )
		{
			// 设置连接状态
			connect_status_ = 1;

			// 缓冲未设置则设置缓冲
			if( 0 == recv_buffer_.capacity() )
			{
				if( false == set_buffer_size( e_recv, g_recv_buffer_size ) )
				{
					// 提交事件 NET_ERROR_NOT_ENOUGH_MEMROY
					make_event( EVENT_ERROR, NET_ERROR_NOT_ENOUGH_MEMROY, userdata_ );
					
					close();
					return;
				}
			}

			// 缓冲未设置则设置缓冲
			if( 0 == send_buffer_.capacity() )
			{
				if( false == set_buffer_size( e_send, g_send_buffer_size ) )
				{
					// 提交事件 NET_ERROR_NOT_ENOUGH_MEMROY
					make_event( EVENT_ERROR, NET_ERROR_NOT_ENOUGH_MEMROY, userdata_ );

					close();
					return;
				}
			}

			// 提交事件 CONNECT / ACCEPT
			// 一定等缓冲准备好后再提交事件，否则有可能事件已收到，缓冲还未准备完成。
			make_event( event, 0, userdata_ );


			++operator_count_;

			// 提交异步读取
			socket_.async_read_some(
				buffer( recv_buffer_.end(), recv_buffer_.space( recv_packet_max_ ) ),
				std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			timer_.cancel();

			// 如果需要保活，提交保活定时器
			if( timeout_ )
			{
				timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( 500, timeout_ ) ) );
				timer_.async_wait( bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
			}
		}

		xgc_void asio_Socket::handle_connect( const asio::error_code& error )
		{
			if( !socket_.is_open() )
				return;

			if( !error )
			{
				accept( EVENT_CONNECT );
			}
			else
			{
				make_event( EVENT_ERROR, NET_ERROR_CONNECT, userdata_ );
			}
		}

		xgc_void asio_Socket::handle_timeout( const asio::error_code& error )
		{
			if( connect_status_ != 0 )
				return;

			if( is_connected() )
				return;

			if ( !error )
			{
				// 连接超时
				if ( connect_info_ && connect_info_->is_reconnect_timeout && socket_.is_open() )
				{
					// 对于允许重连的，尝试重连
					connect();
				}
				else
				{
					make_event( EVENT_ERROR, NET_ERROR_CONNECT_TIMEOUT, userdata_ );
				}
			}
		}

		xgc_void asio_Socket::handle_recv( const asio::error_code& error, size_t translate )
		{
			if( error )
			{
				--operator_count_;
				close();
				return;
			}

			if( false == recv_buffer_.push( translate ) )
			{
				XGC_ASSERT( false );
				make_event( EVENT_ERROR, NET_ERROR_PACKET_SPACE, userdata_ );

				--operator_count_;
				close();
				return;
			}

			INetworkSession* session = (INetworkSession*) userdata_;
			int packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );

			if( packet_length > recv_buffer_.capacity() )
			{
				XGC_ASSERT( false );
				make_event( EVENT_ERROR, NET_ERROR_PACKET_SPACE, userdata_ );

				--operator_count_;
				close();
				return;
			}

			// 已接收到数据包
			while( packet_length && recv_buffer_.length() >= packet_length )
			{
				// 将数据包发送到队列
				EventHeader evt;
				evt.handle = handle_;
				evt.event = EVENT_DATA;
				evt.error = packet_length;
				evt.bring = userdata_;

				auto packet = CNetworkPacket::allocate( sizeof(EventHeader) + packet_length );
				packet->putn( &evt, sizeof(evt) );
				packet->putn( recv_buffer_.begin(), packet_length );

				getSocketMgr().Push( packet );

				// 弹出已收的数据包
				recv_buffer_.pop( packet_length );

				// 重新计算包长度，处理一个接收多个包的问题。
				packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );
			}

			if( false == recv_buffer_.enough( recv_packet_max_ ) )
				recv_buffer_.reset();

			if( connect_status_ == 1 )
			{
				// 继续读取协议体
				socket_.async_read_some(
					buffer( recv_buffer_.end(), recv_buffer_.space( recv_packet_max_ ) ),
					std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			}
			else
			{
				--operator_count_;
				close();
			}
		}

		xgc_void asio_Socket::send( xgc_lpvoid data, xgc_size size )
		{
			XGC_ASSERT( data && size );

			std::lock_guard< std::mutex > guard( send_buffer_lock );

			// 数据放入发送缓冲
			if( send_buffer_.put( data, size ) != size )
			{
				make_event( EVENT_ERROR, NET_ERROR_SEND_BUFFER_FULL, userdata_ );

				close();
				return;
			}

			// 发送和发送完成可能不在同一线程中
			if( send_buffer_.length() == size )
			{
				++operator_count_;

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
				xgc_lpvoid data = std::get< 0 >( t ) ;
				xgc_size   size = std::get< 1 >( t ) ;

				// 数据放入发送缓冲
				if( send_buffer_.put( data, size ) != size )
				{
					make_event( EVENT_ERROR, NET_ERROR_SEND_BUFFER_FULL, userdata_ );

					close();
					return;
				}

				total += size;
			}

			// 发送和发送完成可能不在同一线程中
			if( send_buffer_.length() == total )
			{
				++operator_count_;

				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}


		xgc_void asio_Socket::handle_send( const asio::error_code& error, size_t translate )
		{
			if( error )
			{
				--operator_count_;
				close();
				return;
			}

			std::lock_guard< std::mutex > guard( send_buffer_lock );

			auto pop_size = send_buffer_.pop( translate );
			XGC_ASSERT( pop_size == translate );

			// 如果有新添加的数据未发送，则继续发送
			if( connect_status_ == 1 && send_buffer_.length() )
			{
				++operator_count_;
				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
			
			--operator_count_;
			if( connect_status_ == 2 )
				close();
		}

		xgc_void asio_Socket::close()
		{
			asio::error_code ec;

			xgc_uint16 exp1 = 1;
			if( connect_status_.compare_exchange_strong( exp1, 2 ) )
			{
				// 先取消定时器
				timer_.cancel( ec );
				XGC_ASSERT( !ec );

				socket_.shutdown( ip::tcp::socket::shutdown_both, ec );
				XGC_ASSERT( !ec || ec.value() == 10054 );

				socket_.close( ec );
				XGC_ASSERT( !ec );
			}
			
			xgc_uint16 exp2 = 2;
			if( operator_count_ == 0 && connect_status_.compare_exchange_strong( exp2, 3 ) )
			{
				make_event( EVENT_CLOSE, 0, userdata_ );
				userdata_ = xgc_nullptr;

				connect_status_ = 0;
			}

			if( operator_count_ == 0 && connect_status_ == 0 )
			{
				LinkDown( shared_from_this() );
			}
		}

		xgc_ulong asio_Socket::get_socket_info( xgc_uint32 addr[2], xgc_uint16 port[2] )
		{
			try
			{
				// local addr
				addr[0] = socket_.local_endpoint().address().to_v4().to_ulong();
				// local port
				port[0] = socket_.local_endpoint().port();

				// remote addr
				addr[0] = socket_.remote_endpoint().address().to_v4().to_ulong();
				// remote port
				port[0] = socket_.remote_endpoint().port();
			}
			catch( asio::error_code& ec )
			{
				ec;
				return -1;
			}

			return 0;
		}

		xgc_void asio_Socket::handle_timer( const asio::error_code& e )
		{
			if( connect_status_ != 1 )
				return;

			if( e != asio::error::operation_aborted )
			{
				if( is_connected() && timeout_ )
				{
					// check socket timeout
					make_event( EVENT_PING, 0, userdata_ );

					timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( timeout_, 500 ) ) );
					timer_.async_wait( std::bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
				}
			}
		}

		xgc_void asio_Socket::make_event( xgc_uint32 event, xgc_uint32 error_code, xgc_lpvoid bring )
		{
			EventHeader evt;
			evt.handle = handle_;
			evt.event = event;
			evt.error = error_code;
			evt.bring = bring;

			getSocketMgr().Push( CNetworkPacket::allocate( &evt, sizeof( evt ), handle_ ) );
		}
	}
}