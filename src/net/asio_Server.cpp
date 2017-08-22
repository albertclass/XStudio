#include "Header.h"
namespace xgc
{
	namespace net
	{
		asio_Server::asio_Server( io_service& service, SessionCreator creator, server_options *options )
			: service_( service )
			, acceptor_( service_, ip::tcp::v4() )
			, options_( options ? *options : server_options { 0, 0, 0, 0, 10, 0, 0, 0 } )
			, acceptor_count_( 0 )
			, creator_( creator )
			, status_( 0 )
		{
			for( xgc_uint16 i = 0; i < options_.allow_count; ++i )
			{
				if( options_.allow_addr[i].from > options_.allow_addr[i].to )
					std::swap( options_.allow_addr[i].from, options_.allow_addr[i].to );
			}
		}

		asio_Server::~asio_Server()
		{
			StopServer();
		}

		xgc_bool asio_Server::StartServer( const char *address, xgc_int16 port )
		{
			error_code ec;
			ip::address_v4 addr = ip::address_v4::from_string( address, ec );
			XGC_ASSERT_RETURN( !ec, false );

			ip::tcp::endpoint ep( addr, port );

			XGC_ASSERT_RETURN( acceptor_.is_open(), false );
			// 设置允许地址重用
			acceptor_.set_option( ip::tcp::acceptor::reuse_address( true ), ec );
			XGC_ASSERT_RETURN( !ec, false );
			// 服务器保活，不主动关闭
			acceptor_.set_option( ip::tcp::acceptor::keep_alive( true ), ec );
			XGC_ASSERT_RETURN( !ec, false );
			// 置空连接系统接收缓冲区
			acceptor_.set_option( socket_base::receive_buffer_size( 0 ), ec );
			XGC_ASSERT_RETURN( !ec, false );
			// 置空连接系统发送缓冲区
			acceptor_.set_option( socket_base::send_buffer_size( 0 ), ec );
			XGC_ASSERT_RETURN( !ec, false );

			// 绑定地址
			acceptor_.bind( ep, ec );
			XGC_ASSERT_RETURN( !ec, false );

			// 开始监听
			acceptor_.listen( socket_base::max_connections, ec );
			XGC_ASSERT_RETURN( !ec, false );

			status_ = 1;
			
			// 投递连接套接字
			while( options_.acceptor_count > acceptor_count_ )
			{
				post_accept();
			}

			return true;
		}

		xgc_void asio_Server::StopServer()
		{
			xgc_uint16 compare = 1;
			if( status_.compare_exchange_strong( compare, 2 ) )
			{
				asio::error_code ec;
				acceptor_.cancel( ec );
				XGC_ASSERT( !ec );
				acceptor_.close( ec );
				XGC_ASSERT( !ec );

				while ( acceptor_count_ )
					std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

				getSocketMgr().CloseAll( this );

				status_ = 3;
			}
		}

		xgc_bool asio_Server::allow( xgc_uint32 addr )
		{
			if( 0 == options_.allow_count )
				return true;

			for( xgc_uint16 i = 0; i < options_.allow_count; ++i )
				if( addr >= options_.allow_addr[i].from && addr <= options_.allow_addr[i].to )
					return true;

			return false;
		}

		xgc_void asio_Server::handle_accept( asio_SocketPtr pSocket, const asio::error_code& error )
		{
			if( !error )
			{
				xgc_uint32 addr[2];
				xgc_uint16 port[2];

				pSocket->get_socket_info( addr, port );
				if( allow( addr[1] ) )
					pSocket->pending( this );
				else
					pSocket->close();

				post_accept();
			}

			--acceptor_count_;
		}

		xgc_void asio_Server::post_accept()
		{
			if( status_ != 1 )
				return;
			
			//// 此处会发生不同步的情况，多个线程比较后会进入，直到 ++acceptor_count 被触发后
			//// 多投递几个也没什么大不了，所以就不处理了。
			//while( options_.acceptor_count > acceptor_count_ )
			//{
				// asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service_, timeout_, xgc_nullptr, this );
				asio_SocketPtr pSocket = asio_SocketPtr( XGC_NEW asio_Socket( service_, options_.heartbeat_interval, xgc_nullptr, this ) );
				if( xgc_nullptr == pSocket )
					return;
				
				if( options_.recv_buffer_size )
					pSocket->set_buffer_size( asio_Socket::e_recv, options_.recv_buffer_size );

				if( options_.send_buffer_size )
					pSocket->set_buffer_size( asio_Socket::e_send, options_.send_buffer_size );

				if( options_.recv_packet_max )
					pSocket->set_packet_max( asio_Socket::e_recv, options_.recv_packet_max );

				if( options_.send_packet_max )
					pSocket->set_packet_max( asio_Socket::e_send, options_.send_packet_max );

				LinkUp( pSocket );

				++acceptor_count_;
				acceptor_.async_accept( pSocket->socket_, std::bind( &asio_Server::handle_accept, this, pSocket, std::placeholders::_1 ) );

			//	if( 0 == options_.acceptor_smart )
			//		break;
			//}
		}
	}
}