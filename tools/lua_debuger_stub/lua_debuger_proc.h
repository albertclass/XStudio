#ifndef __LUA_DEBUGER_PROC_H__
#define __LUA_DEBUGER_PROC_H__

struct session;

/// ִ�е���ָ��
/// return : �Ƿ�ſ�������ִ�д���
bool execute_cmd( lua_State* L, lua_Debug* ar );

#endif // __LUA_DEBUGER_PROC_H__