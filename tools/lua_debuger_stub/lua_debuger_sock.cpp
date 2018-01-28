#include "lua_debuger.h"
#include "lua_debuger_sock.h"
#include "lua_debuger_proc.h"

debuger dbg;

bool wait_signal( int sleep = 0 )
{
	if( !dbg.exit )
	{
		// ����fini�˳�ʱ�����ܵ���server�м�������wait_signal
		// ��ʱ�ȴ��ź���������fini�е�thread.join���뻥�����ʴ˴����ж���
		dbg.signal.notify_one();

		std::unique_lock< std::mutex > lock( dbg.mtx );
		if( sleep )
		{
			if( std::cv_status::timeout == dbg.signal.wait_for( lock, std::chrono::milliseconds( sleep ) ) )
				return false;
		}
		else
		{
			dbg.signal.wait( lock );
		}
	}

	return true;
}

void wait_command( lua_State* L, execute pfn )
{
	do { wait_signal();  } while( pfn( L ) );
}

void server( int port, int stop )
{
	bool attached = false;
	char err[XNET_ERR_LEN];
	SOCKET server = tcp_server( err, port, "127.0.0.1", 1 );

	if( XNET_ERR == net_nonblock( err, server ) )
	{
		printf( "nonblock set error %s\n", err );
		return;
	}

	char peer_addr[64];
	int  peer_port;

	wait_signal();

	while( !dbg.exit )
	{
		if( false == dbg.attached )
		{
			SOCKET s = tcp_accept( err, server, peer_addr, sizeof( peer_addr ), &peer_port );
			if( s != -1 )
			{
				printf( "new client accept %s:%d\n", peer_addr, peer_port );

				dbg.sock = s;
				dbg.send = 0;
				dbg.recv = 0;

				// ���ø���״̬
				dbg.attached = true;
				// ���õ���ģʽ
				dbg.mode = debug_mode::e_attach;

				// �ȴ����Ի�Ӧ
				wait_signal();
			}
		}
		else
		{
			int r = net_recv( dbg.sock, dbg.recv_buffer + dbg.recv, sizeof( dbg.recv_buffer ) - dbg.recv );
			if( r > 0 )
			{
				dbg.recv += r;

				while( dbg.recv > sizeof( int ) )
				{
					// ���
					dbg.ipkg = *(int*)dbg.recv_buffer + sizeof( int );

					if( dbg.recv < dbg.ipkg )
						break;
				
					// ������ȫ���ȴ����������
					wait_signal();

					// �ƶ�����������
					memmove( dbg.recv_buffer, dbg.recv_buffer + dbg.ipkg, dbg.ipkg );

					// ���¼����ѽ������ݰ��ߴ�
					dbg.recv -= dbg.ipkg;
					dbg.ipkg = 0;
				}
			}

			if( dbg.send )
			{
				int w = net_send( dbg.sock, dbg.send_buffer, dbg.send );
				if( w > 0 )
				{
					memmove( dbg.send_buffer, dbg.send_buffer + w, w );
					dbg.send -= w;
				}
			}
		}
	}

	if( dbg.sock != -1 )
	{
		net_close( dbg.sock );
		dbg.sock = -1;
		dbg.attached = false;
	}
}

/// ������Ϣ��������
void send( const void* data, int size )
{
	if( size + sizeof(int) > sizeof( dbg.send_buffer ) - dbg.send )
		return;

	// ע�⣬send_buffer���ڶ��̻߳����£�����δ�����ݱ���
	memcpy( dbg.send_buffer + dbg.send, &size, sizeof( size ) );
	dbg.send += sizeof( size );
	memcpy( dbg.send_buffer + dbg.send, data, size );
	dbg.send += size;
}

/// ��Ӧ��������Ϣ
void resp( const char* fmt, ... )
{
	char buf[1024];

	va_list ap;
	va_start( ap, fmt );

	int cpy = vsprintf_s( buf, fmt, ap );
	if( cpy >= 0 )
		send( buf, cpy + 1 );

	va_end( ap );
}

void hook( lua_State *L, lua_Debug *ar );
void hook_all( lua_State *L, lua_Debug *ar );

void on_call( lua_State *L );
void on_iret( lua_State *L );
void on_line( lua_State *L, int line );

int init_debuger( lua_State* L, int port, int stop )
{
	dbg.sock = -1;
	dbg.send = 0;
	dbg.recv = 0;

	dbg.exit = false;
	dbg.mode = 0;

	dbg.attached = false;

	dbg.breakpoints_ids_max = 0;
	lua_sethook( L, hook, LUA_MASKCOUNT, 1 );

	dbg.server = std::thread( server, port, stop );
	// the first wiat is detemine thread is started.
	wait_signal();

	if( stop == 1 )
	{
		wait_signal();
	}
	else
	{
		dbg.signal.notify_one();
	}

	return 1;
}

void hook( lua_State *L, lua_Debug *ar )
{
	if( dbg.mode == debug_mode::e_attach )
	{
		lua_sethook( L, hook_all, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0 );
		wait_command( L, execute_cmd );

		return;
	}
}

void hook_all( lua_State *L, lua_Debug *ar )
{
	if( dbg.mode == debug_mode::e_detach )
	{
		lua_sethook( L, hook, LUA_MASKCOUNT, 1 );
		return;
	}

	switch( ar->event )
	{
		case LUA_HOOKCALL:
		on_call( L );
		break;

		case LUA_HOOKRET:
		case LUA_HOOKTAILRET:	//verify
		on_iret( L );
		break;

		case LUA_HOOKLINE:
		on_line( L, ar->currentline );
		break;
	}
}

void on_call( lua_State *L )
{
	if( dbg.mode == debug_mode::e_step_in )
	{
		lua_Debug ar;
		if( lua_getstack( L, 0, &ar ) )
		{
			lua_getinfo( L, "Sln", &ar );

			resp( "break '%s(%d):%s'\n\n", ar.short_src, ar.currentline, ar.what );

			// �ϵ㣬�ȴ���һ��ָ��
			wait_command( L, execute_cmd );
		}
	}
}

void on_iret( lua_State *L )
{
	if( dbg.mode == debug_mode::e_step_out )
	{
		lua_Debug ar;
		if( lua_getstack( L, 0, &ar ) )
		{
			lua_getinfo( L, "Sln", &ar );

			resp( "break '%s(%d):%s'\n\n", ar.short_src, ar.currentline, ar.what );

			// �ϵ㣬�ȴ���һ��ָ��
			wait_command( L, execute_cmd );
		}
	}
}

void on_line( lua_State *L, int line )
{
	if( dbg.mode == debug_mode::e_step )
	{
		lua_Debug ar;
		if( lua_getstack( L, 0, &ar ) )
		{
			lua_getinfo( L, "Sln", &ar );

			resp( "break '%s(%d):%s'\n\n", ar.short_src, ar.currentline, ar.what );

			// �ϵ㣬�ȴ���һ��ָ��
			wait_command( L, execute_cmd );
		}
	}
	else if( dbg.mode == debug_mode::e_run )
	{
		// ���Ҷϵ�
		auto it1 = dbg.breakpoints_reg.find( line );
		if( it1 == dbg.breakpoints_reg.end() )
			// ����û�жϵ��򷵻�
			return;

		lua_Debug ar;
		if( lua_getstack( L, 0, &ar ) )
		{
			lua_getinfo( L, "Sln", &ar );

			// �����ļ�
			auto it2 = it1->second.find( ar.short_src );
			if( it2 == it1->second.end() )
				// û��ƥ����ļ��򷵻�
				return;

			resp( "break '%s(%d):%s'\n\n", ar.short_src, ar.currentline, ar.what );

			// �ϵ㣬�ȴ���һ��ָ��
			wait_command( L, execute_cmd );
		}
	}
}