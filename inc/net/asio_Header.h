#ifndef _ASIO_HEADER_H_
#define _ASIO_HEADER_H_

#include "defines.h"

#include <time.h>

#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

#include <queue>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#define ASIO_STANDALONE
// #define ASIO_HAS_BOOST_DATE_TIME
#include "asio.hpp"
#include "asio/thread.hpp"
#include "asio/strand.hpp"
#include "asio/deadline_timer.hpp"

#include "Netbase.h"
#include "MessageQueue.h"
#include "asio_NetworkPacket.h"
#include "asio_Socket.h"
#include "asio_SocketMgr.h"
#include "asio_Network.h"
#include "asio_Server.h"
#include "asio_AsyncConnector.h"

using namespace asio;
#define REALSIZE( S ) (((S+sizeof(xgc_uint32))&(~0x3ff))+0x400)

#define NET_WRITELOG( FMT, ... ) _tprintf( FMT, __VA_ARGS__ )

namespace xgc
{
	namespace net
	{
		// get handle manager instance.
		asio_SocketMgr& getSocketMgr();
	}
}
#endif // _ASIO_HEADER_H_