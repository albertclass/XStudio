#ifndef __LUA_DEBUGER_PROC_H__
#define __LUA_DEBUGER_PROC_H__

struct session;

/// 执行调试指令
/// return : 是否放开，继续执行代码
bool execute_cmd( lua_State* L, lua_Debug* ar );

#endif // __LUA_DEBUGER_PROC_H__