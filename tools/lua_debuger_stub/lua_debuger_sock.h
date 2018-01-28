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
	/// 发送字节数
	std::atomic_int send;
	/// 接收字节数
	std::atomic_int recv;
	/// 包长度
	std::atomic_int ipkg;

	/// 发送缓冲
	char send_buffer[1024 * 4];
	/// 接收缓冲
	char recv_buffer[1024 * 4];

	/// 是否调试模式： 0 - 正常运行， 1 - 调试中
	std::atomic_long mode;

	std::atomic_bool exit;
	/// 是否调试器是否附加
	std::atomic_bool attached;

	/// 调试器监听线程
	std::thread server;

	/// 同步对象
	std::mutex mtx;
	/// 同步条件
	std::condition_variable signal;

	/// 调试会话
	/// 断点集合 line - files
	map< int, set< string > > breakpoints_reg;
	/// 断点
	map< int, tuple< int, string >, std::greater< int > > breakpoints_ids;
	/// 断点序号
	int breakpoints_ids_max;
	/// 监视列表
	set< string > watchs;

	int	call_level;	// 当前堆栈深度
	int	stop_level;	// 暂停时的堆栈深度
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

/// 发送消息给调试器
void send( const void* data, int size );

/// 发送消息给调试器
void resp( const char* data, ... );


#endif //__LUA_DEBUGER_SOCK_H__