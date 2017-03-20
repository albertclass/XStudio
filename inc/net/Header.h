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
#include "NetworkPacket.h"
#include "asio_NetBuffer.h"
#include "asio_Socket.h"
#include "asio_SocketMgr.h"
#include "asio_Network.h"
#include "asio_Server.h"

using namespace asio;
#define REALSIZE( S ) (((S+sizeof(xgc_uint32))&(~0x3ff))+0x400)

#define NET_WRITELOG( FMT, ... ) _tprintf( FMT, __VA_ARGS__ )

namespace xgc
{
	namespace net
	{
		// get handle manager instance.
		asio_SocketMgr& getSocketMgr();

		#pragma pack(1)
		///
		/// \brief �¼�ͷ
		///
		/// \author albert.xu
		/// \date 2017/03/01 14:25
		///
		struct EventHeader
		{
			/// ���
			network_t handle;
			/// �¼�
			xgc_uint32 event;
			/// Я��(�����¼��в�ͬ����)
			xgc_uint64 bring;
		};

		#pragma pack()

		XGC_INLINE xgc_time64 tick()
		{
			return std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now().time_since_epoch() ).count();
		}
	}
}
#endif // _ASIO_HEADER_H_