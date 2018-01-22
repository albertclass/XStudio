#include "lua_debuger.h"
#include "lua_debuger_stub.h"
#include "lua_debuger_sock.h"
#include "lua_debuger_proc.h"

extern "C"
{
	int init( lua_State* L )
	{
		int top = lua_gettop( L );
		int port = 30090;
		int stop = 1;

		if( lua_type( L, -1 ) != LUA_TNIL )
			stop = (int)lua_toboolean( L, -1 );

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

	// 设置断点
	int sbrk( lua_State* L )
	{
		auto file = lua_tostring( L, -2 );
		auto line = lua_tointeger( L, -1 );

		auto ln = dbg.breakpoints_reg[(int)line];
		auto it = ln.find( file );
		if( it == ln.end() )
		{
			ln.insert( file );
			dbg.breakpoints_ids[dbg.breakpoints_ids_max] = std::make_tuple( (int)line, file );
		}

		return 0;
	}

	// 清除断点
	int cbrk( lua_State* L )
	{
		auto id = lua_tointeger( L, -1 );
		auto it = dbg.breakpoints_ids.find( id );
		if( it != dbg.breakpoints_ids.end() )
		{
			auto ln = dbg.breakpoints_reg.find( std::get< 0 >( it->second ) );
			if( ln != dbg.breakpoints_reg.end() )
			{
				auto &file = std::get< 1 >( it->second );
				ln->second.erase( file );
			}
		}

		return 0;
	}

	// 添加变量监视
	int addw( lua_State* L )
	{
		return 0;
	}

	// 删除变量监视
	int delw( lua_State* L )
	{
		return 0;
	}

	// 等待调试命令
	int wait( lua_State* L )
	{
		wait_command( L, execute_cmd );
		return 0;
	}

	static const struct luaL_Reg lib[] = {
		{ "init", init },
		{ "fini", fini },
		{ "sbrk", sbrk },
		{ "cbrk", cbrk },
		{ "addw", addw },
		{ "delw", delw },
		{ "wait", wait },
		{ NULL, NULL },
	};

	LIB_API int luaopen_lua_debuger_stub( lua_State *L )
	{
		luaL_register( L, "debuger", lib );
		return 1;
	}
}