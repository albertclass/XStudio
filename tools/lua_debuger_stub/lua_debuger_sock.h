#pragma once
#ifndef __LUA_DEBUGER_SOCK_H__
#define __LUA_DEBUGER_SOCK_H__
enum debug_mode
{
	e_detach,
	e_attach,
	e_run,
	e_stop,
	e_step,
	e_step_in,
	e_step_out,
};

typedef bool( *execute )( lua_State* L );

struct debuger
{
	SOCKET sock;
	/// �����ֽ���
	std::atomic_int send;
	/// �����ֽ���
	std::atomic_int recv;
	/// ������
	std::atomic_int ipkg;

	/// ���ͻ���
	char send_buffer[1024 * 4];
	/// ���ջ���
	char recv_buffer[1024 * 4];

	/// �Ƿ����ģʽ�� 0 - �������У� 1 - ������
	std::atomic_long mode;

	std::atomic_bool exit;
	/// �Ƿ�������Ƿ񸽼�
	std::atomic_bool attached;

	/// �����������߳�
	std::thread server;

	/// ͬ������
	std::mutex mtx;
	/// ͬ������
	std::condition_variable signal;

	/// ���ԻỰ
	/// �ϵ㼯�� line - files
	map< int, set< string > > breakpoints_reg;
	/// �ϵ�
	map< int, tuple< int, string >, std::greater< int > > breakpoints_ids;
	/// �ϵ����
	int breakpoints_ids_max;
	/// �����б�
	set< string > watchs;

	int	call_level;	// ��ǰ��ջ���
	int	stop_level;	// ��ͣʱ�Ķ�ջ���
};

extern debuger dbg;

/// init lua debuger 
int init_debuger( lua_State* L, int port, int stop );

/// wait a debug command
void wait_command( lua_State* L, execute pfn );

/// debuger hook
void hook( lua_State *L, lua_Debug *ar );

/// debuger hook all
void hook_all( lua_State *L, lua_Debug *ar );

/// ������Ϣ��������
void send( const void* data, int size );

/// ������Ϣ��������
void resp( const char* data, ... );


#endif //__LUA_DEBUGER_SOCK_H__