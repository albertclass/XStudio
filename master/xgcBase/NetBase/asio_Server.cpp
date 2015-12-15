#include "asio_header.h"
#include "asio_Server.h"
#include "asio_NetworkHandler.h"
#include "ServerSession.h"

namespace XGC
{
	namespace net
	{
		asio_ServerBase::asio_ServerBase( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 time_invent, xgc_uint16 timeout )
			: service_( service )
			, acceptor_( service_, ip::tcp::v4() )
			, time_invent_( time_invent )
			, timeout_( timeout )
			, acceptor_count_( acceptor_count )
			, protocal_( XGC_NEW PacketProtocal( protocal ) )
		{
			XGC_ASSERT( acceptor_count_ );
		}

		asio_ServerBase::~asio_ServerBase(xgc_void)
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

			while( acceptor_count_ ) Sleep(1);

			if( bCloseAllLink )
				getHandleManager().CloseAll( (xgc_uintptr)this );
		}

		xgc_void asio_ServerBase::handle_accept( asio_SocketPtr pSocket, const asio::error_code& error )
		{
			if (!error)
			{
				pSocket->accept( (xgc_uintptr)this );
				post_accept();
			}
			else
			{
				acceptor_count_--;
			}
		}

		xgc_void asio_ServerBase::post_accept()
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service_, protocal_, CreateHolder( protocal_ ), time_invent_ );
			acceptor_.async_accept( pSocket->socket(), std::bind( &asio_Server::handle_accept, this, pSocket, std::placeholders::_1 ) );
		}
	}

	//----------------------------------------------------------------
	namespace net
	{
		//////////////////////////////////////////////////////////////////////////
		// Server

		asio_Server::asio_Server( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, IMessageQueue *pMessageQueue )
			: asio_ServerBase( service, protocal, acceptor_count, ping_invent, timeout )
			, m_pMessageQueue( pMessageQueue )
		{

		}

		asio_Server::~asio_Server()
		{

		}

		INetworkSession* asio_Server::CreateHolder( PacketProtocal* protocal )
		{
			return XGC_NEW CServerSession( m_pMessageQueue, protocal );
		}

		//////////////////////////////////////////////////////////////////////////
		// ServerEx
		asio_ServerEx::asio_ServerEx( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, create_handler_func call, xgc_uintptr param )
			: asio_ServerBase( service, protocal, acceptor_count, ping_invent, timeout )
			, call_( call )
			, param_( param )
		{
		}

		asio_ServerEx::~asio_ServerEx(xgc_void)
		{
		}

		INetworkSession* asio_ServerEx::CreateHolder( PacketProtocal* protocal )
		{
			return call_( param_, protocal_ );
		}
	}
}