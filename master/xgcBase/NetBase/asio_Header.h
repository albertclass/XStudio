#ifndef _ASIO_HEADER_H_
#define _ASIO_HEADER_H_

#include "Netbase.h"
#include <functional>
#include <memory>
#include <mutex>
#include <chrono>

#define ASIO_STANDALONE
#define ASIO_HAS_BOOST_DATE_TIME
#include <asio.hpp>
#include <asio\thread.hpp>
#include <asio\strand.hpp>
#include <asio\deadline_timer.hpp>

#include "ServerSession.h"
#include "HandlerManager.h"
#include "MessageQueue.h"
#include "asio_NetworkPacket.h"
#include "asio_NetworkHandler.h"
#include "asio_Network.h"
#include "asio_Server.h"
#include "asio_AsyncConnector.h"

using namespace asio;
#define REALSIZE( S ) (((S+sizeof(xgc_uint32))&(~0x3ff))+0x400)

#define NET_WRITELOG( FMT, ... ) _tprintf( FMT, __VA_ARGS__ )

namespace XGC
{
	namespace net
	{
		using boost::posix_time::millisec;

		// get handle manager instance.
		CHandlerManager< asio_NetworkHandler >& getHandleManager();
	}
}
#endif // _ASIO_HEADER_H_