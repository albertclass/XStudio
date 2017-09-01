#pragma once

#include "LuaDebuger.h"
struct lua_State;
class LuaDebuger
{
public:
	LuaDebuger();
	~LuaDebuger();

	bool initialize( lua_State* L, const char* lpszPipename );
	void terminal();

protected:
	friend void Debug( lua_State *L, lua_Debug* ar );
	friend void line_hook( LuaDebuger *pDebuger, lua_State *L, lua_Debug *ar );
	friend void call_hook( LuaDebuger *pDebuger, lua_State *L, lua_Debug *ar );
	friend void ret_hook( LuaDebuger *pDebuger, lua_State *L, lua_Debug *ar );
	friend void count_hook( LuaDebuger *pDebuger, lua_State *L, lua_Debug *ar );

	bool command( const char* lpszCmd );

	void bp( const char* name, int line );
	void run( int mode );

	void cmd_breakpoint( const char* lpszParam );
	void cmd_checkpoint( const char* lpszParam );
	void cmd_clearpoint( const char* lpszParam );
	void cmd_step( const char* lpszParam );
	void cmd_stepout( const char* lpszParam );
	void cmd_stepin( const char* lpszParam );
	void cmd_run( const char* lpszParam );

	void cmd_stack( const char* lpszParam );
	void cmd_open( const char* lpszParam );
	void cmd_cd( const char* lpszParam );
	void cmd_dir( const char* lpszParam );
	void cmd_list( const char* lpszParam );

	void makestack( lua_State *L, lua_Debug *ar );
	void clearstack();

	bool judgeBreak( const char* name, int line );
	bool waitSignal( lua_State *L );
	void Signal();
	int	 output( const char* szFmt, ... );

	static unsigned int __stdcall guard( void *param );
private:
	struct Impl;
	struct ThreadParam;
	Impl*			m_pImpl;
	ThreadParam*	m_thread_param;
};
