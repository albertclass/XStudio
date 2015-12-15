#pragma once
#ifndef _ASIO_SERVER_H_
#define _ASIO_SERVER_H_

#include "Netbase.h"
#include "asio_Header.h"
#include "asio_NetworkHandler.h"

namespace XGC
{
	namespace net
	{
		using namespace asio;

		class asio_ServerBase
		{
		public:
			asio_ServerBase( io_service& service_, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 time_invent, xgc_uint16 timeout );
			~asio_ServerBase(xgc_void);

			xgc_bool StartServer( const char *address, xgc_int16 port );
			xgc_void StopServer( xgc_bool bCloseAllLink = true );

		protected:
			xgc_void handle_accept( asio_SocketPtr pSocket, const asio::error_code& error);
			xgc_void post_accept();

			virtual INetworkSession* CreateHolder( PacketProtocal* protocal ) = 0;
		protected:
			io_service&			service_;
			ip::tcp::acceptor	acceptor_;
			xgc_uint16			time_invent_;
			xgc_uint16			timeout_;
			xgc_uint16			acceptor_count_;
			PacketProtocal*			protocal_;
		};

		class asio_Server : public asio_ServerBase
		{
		public:
			asio_Server( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, IMessageQueue *pMessageQueue );
			~asio_Server();

		protected:
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal );

		private:
			IMessageQueue*	m_pMessageQueue;
		};

		class asio_ServerEx	:	public asio_ServerBase
		{
		public:
			asio_ServerEx( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, create_handler_func call, xgc_uintptr param );
			~asio_ServerEx(xgc_void);

		protected:
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal );

		private:
			create_handler_func call_;
			xgc_uintptr param_;
		};

	}
}
#endif // _ASIO_SERVER_H_