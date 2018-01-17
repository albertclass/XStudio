#ifndef __LUA_DEBUGER_H__
#define __LUA_DEBUGER_H__
#include <atomic>
#include <thread>
#include <mutex>

#include "defines.h"
#include "xutility.h"
#include "xnet.h"

using namespace xgc;
using namespace xgc::common;

#if defined( _WINDOWS )
# ifdef LUA_DEBUGER_EXPORTS
#  define LIB_API __declspec(dllexport)
# else
#  define LIB_API __declspec(dllimport)
# endif
#elif defined( _LINUX )
# if defined( LUA_DEBUGER_EXPORTS )
#  define LIB_API __attribute__((__visibility__("default")))
# endif
#endif


extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

#endif // __LUA_DEBUGER_H__
