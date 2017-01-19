#include "asio_Header.h"
#include "ClientSession.h"

namespace xgc
{
	namespace net
	{
		xgc_bool Connect( io_service& service, xgc_lpcstr addr, xgc_int16 port, PacketProtocal *protocal, MessageQueuePtr queue_ptr, xgc_uint16 timeout )
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service, protocal, XGC_NEW CClientSession( queue_ptr, protocal ), 0 );

			if( xgc_nullptr == pSocket )
				return false;

			return pSocket->connect( addr, port, false, timeout );
		}

		xgc_bool ConnectAsync( io_service& service, xgc_lpcstr addr, xgc_int16 port, PacketProtocal *protocal, MessageQueuePtr queue_ptr, xgc_uint16 timeout )
		{
			asio_SocketPtr pSocket = std::make_shared< asio_Socket >( service, protocal, XGC_NEW CClientSession( queue_ptr, protocal ), 0 );

			if( xgc_nullptr == pSocket )
				return false;

			return pSocket->connect( addr, port, true, timeout );
		}
	}
}