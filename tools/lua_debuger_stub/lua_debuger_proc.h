#ifndef __LUA_DEBUGER_PROC_H__
#define __LUA_DEBUGER_PROC_H__

struct session;

// ���öϵ�
int sbrk( lua_State* L );

// ����ϵ�
int cbrk( lua_State* L );

// ��ӱ�������
int addw( lua_State* L );

// ɾ����������
int delw( lua_State* L );

// ʹ�ÿո�ָ��Ĳ����б��������д��ո��������ʹ�����ţ�""��
long parse_cmd( char* cmd, long argc, char** argv );

/// ִ�е���ָ��
/// return : �Ƿ�ſ�������ִ�д���
bool execute_cmd( lua_State* L );

#endif // __LUA_DEBUGER_PROC_H__