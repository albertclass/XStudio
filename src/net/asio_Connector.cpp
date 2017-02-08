#include "asio_Header.h"
#include "NetSession.h"

namespace xgc
{
	namespace net
	{
		xgc_bool asio_Connect( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, MessageQueuePtr queue )
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service, XGC_NEW CClientSession( queue ), 0 );

			if( xgc_nullptr == pSocket )
				return false;

			return pSocket->connect( addr, port, false, timeout );
		}

		xgc_bool asio_ConnectAsync( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, MessageQueuePtr queue )
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service, XGC_NEW CClientSession( queue ), 0 );

			if( xgc_nullptr == pSocket )
				return false;

			return pSocket->connect( addr, port, true, timeout );
		}

		xgc_bool asio_ConnectEx( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, const pfnCreateHolder &creator )
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service, creator(), 0 );

			if( xgc_nullptr == pSocket )
				return false;

			return pSocket->connect( addr, port, false, timeout );
		}

		xgc_bool asio_ConnectAsyncEx( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, const pfnCreateHolder &creator )
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service, creator(), 0 );

			if( xgc_nullptr == pSocket )
				return false;

			return pSocket->connect( addr, port, true, timeout );
		}
	}
}