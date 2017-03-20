#include "Header.h"

namespace xgc
{
	namespace net
	{
		asio_SocketMgr& getSocketMgr()
		{
			static asio_SocketMgr Mgr;
			return Mgr;
		}
	}
}
