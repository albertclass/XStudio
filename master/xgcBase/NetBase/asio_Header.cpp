#include "asio_Header.h"

namespace XGC
{
	namespace net
	{
		CHandlerManager< asio_NetworkHandler >& getHandleManager()
		{
			static CHandlerManager< asio_NetworkHandler > Mgr;
			return Mgr;
		}
	}
}
