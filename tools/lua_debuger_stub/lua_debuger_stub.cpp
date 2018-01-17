#include "lua_debuger.h"
#include "lua_debuger_stub.h"
#include "lua_debuger_sock.h"

extern "C"
{
	int init( lua_State* L )
	{
		int top = lua_gettop( L );
		int port = 30090;
		int stop = 1;

		if( lua_type( L, -1 ) != LUA_TNIL )
			stop = (int)lua_tonumber( L, -1 );

		if( lua_type( L, -2 ) != LUA_TNIL )
			port = (int)lua_tonumber( L, -2 );

		int iret = init_debuger( L, port, stop );

		lua_pushboolean( L, iret );
		return 1;
	}

	int fini( lua_State* L )
	{
		dbg.exit = true;

		// 此处挂起线程，给server线程一个缓冲时间。
		std::this_thread::yield();

		dbg.signal.notify_all();
		dbg.server.join();

		return 0;
	}

	static const struct luaL_Reg lib[] = {
		{ "init", init },
		{ "fini", fini },
		{ NULL, NULL },
	};

	LIB_API int luaopen_lua_debuger_stub( lua_State *L )
	{
		puts( "open library 'debuger'" );
		luaL_register( L, "debuger", lib );

		return 1;
	}
}