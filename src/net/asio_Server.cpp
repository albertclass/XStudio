#include "Header.h"
namespace xgc
{
	namespace net
	{
		asio_ServerBase::asio_ServerBase( io_service& service, xgc_uint16 acceptor_count, xgc_uint16 timeout, SessionCreator creator )
			: service_( service )
			, acceptor_( service_, ip::tcp::v4() )
			, timeout_( timeout )
			, acceptor_count_( acceptor_count )
			, creator_( creator )
		{
			XGC_ASSERT( acceptor_count_ );
		}

		asio_ServerBase::~asio_ServerBase()
		{
			acceptor_.close();
		}

		xgc_bool asio_ServerBase::StartServer( const char *address, xgc_int16 port )
		{
			asio::error_code ec;
			ip::address_v4 addr = ip::address_v4::from_string( address, ec );
			if( ec )
				return false;

			acceptor_.bind( ip::tcp::endpoint( addr, port ), ec );
			if( ec )
				return false;

			acceptor_.listen( socket_base::max_connections, ec );
			if( ec )
				return false;

			for( int i = 0; i < acceptor_count_; ++i )
			{
				post_accept();
			}

			return true;
		}

		xgc_void asio_ServerBase::StopServer( xgc_bool bCloseAllLink )
		{
			asio::error_code ec;
			acceptor_.close( ec );

			while( acceptor_count_ )
				std::this_thread::sleep_for( std::chrono::milliseconds(1) );

			if( bCloseAllLink )
				getSocketMgr().CloseAll( this );
		}

		xgc_void asio_ServerBase::handle_accept( asio_SocketPtr pSocket, const asio::error_code& error )
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

		xgc_void asio_ServerBase::post_accept()
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service_, creator_, timeout_ );
			acceptor_.async_accept( pSocket->socket_, std::bind( &asio_ServerBase::handle_accept, this, pSocket, std::placeholders::_1 ) );
		}
	}
}