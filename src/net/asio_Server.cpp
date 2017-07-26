#include "Header.h"
namespace xgc
{
	namespace net
	{
		asio_Server::asio_Server( io_service& service, xgc_uint16 acceptor_count, xgc_uint16 timeout, SessionCreator creator )
			: service_( service )
			, acceptor_( service_, ip::tcp::v4() )
			, timeout_( timeout )
			, acceptor_count_( acceptor_count )
			, creator_( creator )
		{
			XGC_ASSERT( acceptor_count_ );
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

			// 绑定地址
			acceptor_.bind( ep, ec );
			XGC_ASSERT_RETURN( !ec, false );

			// 开始监听
			acceptor_.listen( socket_base::max_connections, ec );
			XGC_ASSERT_RETURN( !ec, false );

			// 投递连接套接字
			for( int i = 0; i < acceptor_count_; ++i )
			{
				post_accept();
			}

			return true;
		}

		xgc_void asio_Server::StopServer()
		{
			if ( acceptor_count_ )
			{
				asio::error_code ec;
				acceptor_.cancel( ec );
				XGC_ASSERT( !ec );
				acceptor_.close( ec );
				XGC_ASSERT( !ec );

				while ( acceptor_count_ )
					std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

				getSocketMgr().CloseAll( this );
			}
		}

		xgc_void asio_Server::handle_accept( asio_SocketPtr pSocket, const asio::error_code& error )
		{
			if (!error)
			{
				pSocket->hangup( this );
				post_accept();
			}
			else
			{
				acceptor_count_--;
			}
		}

		xgc_void asio_Server::post_accept()
		{
			// asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service_, timeout_, xgc_nullptr, this );
			asio_SocketPtr pSocket = asio_SocketPtr( XGC_NEW asio_Socket( service_, timeout_, xgc_nullptr, this ) );
			if (pSocket)
			{
				LinkUp( pSocket );

				acceptor_.async_accept(pSocket->socket_, std::bind(&asio_Server::handle_accept, this, pSocket, std::placeholders::_1));
			}
		}
	}
}