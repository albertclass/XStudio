#pragma once
#ifndef __LUA_DEBUGER_SOCK_H__
#define __LUA_DEBUGER_SOCK_H__
enum debug_mode
{
	e_run,
	e_brk,
	e_stop,
	e_step,
	e_step_in,
	e_step_out,
};

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

/// debuger hook
void hook( lua_State *L, lua_Debug *ar );

/// server thrad
void server( int port );

/// ������Ϣ��������
void send( const void* data, int size );

/// ������Ϣ��������
void resp( const char* data, ... );


//class LuaDebugCommander
//{
//public:
//	LuaDebugCommander(void);
//	~LuaDebugCommander(void);
//
//	bool	initialize( const char* lpszPipename, ProcessRetCmd fn );
//	bool	command( const char* cmd );
//
//	_command_buffer*	getBuffer();
//	void	releaseBuffer( _command_buffer* buf );
//	bool	waitSignal( DWORD dwTimer = INFINITE );
//protected:
//	void	Signal();
//	void	command();
//	_command_buffer*	result();
//
//private:
//	static unsigned int __stdcall pipe( void* param );
//
//	HANDLE		m_hPipe;
//	HANDLE		m_hThread;
//	HANDLE		m_hSignal;
//	_command_buffer		*m_buffer_head;
//	_command_buffer		*m_buffer_tail;
//
//	bool		m_bWork;
//	run_mode	m_mode;
//	ProcessRetCmd	m_RetFunc;
//
//	friend bool Debug_CheckMode( LuaDebugCommander* pDebuger, run_mode m );
//};

#endif //__LUA_DEBUGER_SOCK_H__