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

#include "net.h"
#include "asio_NetBuffer.h"
#include "asio_Socket.h"
#include "asio_SocketMgr.h"
#include "asio_Network.h"
#include "asio_Server.h"

using namespace asio;
#define REALSIZE( S ) (((S+sizeof(xgc_uint32))&(~0x3ff))+0x400)

#define NET_ERR_STREAM() xgc::common::get_logger( "NET" ) << xgc::common::logger_context( __FILE__, __func__, __LINE__, "error"   )
#define NET_WAR_STREAM() xgc::common::get_logger( "NET" ) << xgc::common::logger_context( __FILE__, __func__, __LINE__, "warning" )
#define NET_TIP_STREAM() xgc::common::get_logger( "NET" ) << xgc::common::logger_context( __FILE__, __func__, __LINE__, "info"    )

#define NET_ERR( FMT, ... )	xgc::common::get_logger( "NET" ).write( xgc::common::logger_context( __FILE__, __func__, __LINE__, "error"   ), FMT, ##__VA_ARGS__ )
#define NET_WAR( FMT, ... ) xgc::common::get_logger( "NET" ).write( xgc::common::logger_context( __FILE__, __func__, __LINE__, "warning" ), FMT, ##__VA_ARGS__ )
#define NET_TIP( FMT, ... )	xgc::common::get_logger( "NET" ).write( xgc::common::logger_context( __FILE__, __func__, __LINE__, "info"    ), FMT, ##__VA_ARGS__ )

namespace xgc
{
	namespace net
	{
		// get handle manager instance.
		asio_SocketMgr& getSocketMgr();

		#pragma pack(1)
		///
		/// \brief 事件头
		///
		/// \author albert.xu
		/// \date 2017/03/01 14:25
		///
		struct NetEvent
		{
			/// 处理句柄
			xgc_lpvoid session;
			/// 句柄
			xgc_uint32 handle;
			/// 事件
			xgc_uint32 event;
		};

		struct NetError
		{
			/// 错误类型
			xgc_int16 error_type;
			/// 错误码
			xgc_int16 error_code;
		};

		struct NetData
		{
			/// 错误类型
			xgc_int32 length;
		};

		#pragma pack()

		#define NET_EVENT_SIZE (sizeof(NetEvent))
		#define NET_ERROR_SIZE (sizeof(NetEvent) + sizeof(NetError))
		#define NET_PACKET_SIZE( Length ) (sizeof(NetEvent) + sizeof(NetData) + Length)

		XGC_INLINE xgc_time64 tick()
		{
			return std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now().time_since_epoch() ).count();
		}
	}
}
#endif // _ASIO_HEADER_H_