#ifndef __LUA_DEBUGER_PROC_H__
#define __LUA_DEBUGER_PROC_H__

struct session;

// 使用空格分隔的参数列表，若参数中带空格，则参数需使用引号（""）
long parse_cmd( char* cmd, long argc, char** argv );

/// 执行调试指令
/// return : 是否放开，继续执行代码
bool execute_cmd( lua_State* L );

#endif // __LUA_DEBUGER_PROC_H__