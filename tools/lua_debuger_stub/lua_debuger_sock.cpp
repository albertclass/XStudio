#include "lua_debuger.h"
#include "lua_debuger_sock.h"
#include "lua_debuger_proc.h"

debuger dbg;
typedef bool( *execute )( lua_State* L, lua_Debug* ar );

void wait_signal()
{
	if( !dbg.exit )
	{
		// 调用fini退出时，可能导致server中即将进入wait_signal
		// 此时等待信号量将会与fini中的thread.join进入互锁，故此处需判定。
		dbg.signal.notify_one();

		std::unique_lock< std::mutex > lock( dbg.mtx );
		dbg.signal.wait( lock );
	}
}

void wait_command( lua_State* L, execute pfn, lua_Debug* ar )
{
	do { wait_signal();  } while( pfn( L, ar ) );
}

void server( int port )
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

	dbg.signal.notify_all();

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

				// 设置附加状态
				dbg.attached = true;
				// 设置调试模式
				dbg.mode = debug_mode::e_brk;

				// 等待调试回应
				wait_signal();
			}
		}
		else
		{
			int r = net_recv( dbg.sock, dbg.recv_buffer + dbg.recv, sizeof( dbg.recv_buffer ) - dbg.recv );
			if( r > 0 )
			{
				dbg.recv += r;

				if( dbg.recv > sizeof( int ) )
				{
					// 解包
					dbg.ipkg = *(int*)dbg.recv_buffer;
				}

				if( dbg.recv >= dbg.ipkg )
				{
					// 包已收全
					if( sizeof( dbg.send_buffer ) - dbg.send >= dbg.ipkg )
					{
						// 等待处理包数据
						wait_signal();

						// 移动后续的数据
						memmove( dbg.recv_buffer, dbg.recv_buffer + dbg.ipkg, dbg.ipkg );

						// 重新计算已接收数据包尺寸
						dbg.recv -= dbg.ipkg;
						dbg.ipkg = 0;
					}
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

/// 发送消息给调试器
void send( const void* data, int size )
{
	if( size + sizeof(int) > sizeof( dbg.send_buffer ) - dbg.send )
		return;

	// 注意，send_buffer处于多线程环境下，这里未做数据保护
	memcpy( dbg.send_buffer + dbg.recv, &size, sizeof( size ) );
	dbg.recv += sizeof( size );
	memcpy( dbg.send_buffer + dbg.recv, data, size );
	dbg.recv += size;
}

/// 回应服务器消息
void resp( const char* data, ... )
{
	char buf[1024];

	va_list ap;
	va_start( ap, data );

	vsprintf_s( buf, data, ap );
	send( data, (int)strlen( data ) + 1 );

	va_end( ap );
}

void hook( lua_State *L, lua_Debug *ar );
void hook_all( lua_State *L, lua_Debug *ar );

void on_call( lua_State *L, lua_Debug *ar );
void on_iret( lua_State *L, lua_Debug *ar );
void on_line( lua_State *L, lua_Debug *ar );

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

	if( stop )
	{
		wait_signal();
	}
	else
	{
		std::unique_lock< std::mutex > lock( dbg.mtx );
		dbg.server = std::thread( server, port );
		dbg.signal.wait( lock );
	}

	return 1;
}

void hook( lua_State *L, lua_Debug *ar )
{
	if( dbg.attached && dbg.mode == debug_mode::e_brk )
	{
		lua_sethook( L, hook_all, LUA_MASKCALL | LUA_MASKLINE | LUA_MASKRET, 0 );
		wait_command( L, execute_cmd, ar );
		return;
	}
}

void hook_all( lua_State *L, lua_Debug *ar )
{
	if( dbg.attached == false || dbg.mode == 0 )
	{
		lua_sethook( L, hook, LUA_MASKCOUNT, 1 );
		return;
	}

	switch( ar->event )
	{
		case LUA_HOOKCALL:
		on_call( L, ar );
		break;

		case LUA_HOOKRET:
		case LUA_HOOKTAILRET:	//verify
		on_iret( L, ar );
		break;

		case LUA_HOOKLINE:
		on_line( L, ar );
		break;
	}
}

void on_call( lua_State *L, lua_Debug *ar )
{
	if( dbg.mode == debug_mode::e_step_in )
		wait_command( L, execute_cmd, ar );
}

void on_iret( lua_State *L, lua_Debug *ar )
{
	if( dbg.mode == debug_mode::e_step_out )
		wait_command( L, execute_cmd, ar );
}

void on_line( lua_State *L, lua_Debug *ar )
{
	if( dbg.mode == debug_mode::e_step )
	{
		// 查找断点
		auto it1 = dbg.breakpoints_reg.find( ar->currentline );
		if( it1 == dbg.breakpoints_reg.end() )
			// 该行没有断点则返回
			return;

		// 查找文件
		auto it2 = it1->second.find( ar->source + 1 );
		if( it2 == it1->second.end() )
			// 没有匹配的文件则返回
			return;

		// 断点，等待下一条指令
		wait_command( L, execute_cmd, ar );
	}
}

//typedef std::vector< std::string >	Params;
//bool PraseString( const char* lpszCommand, Params& param )
//{
//	const char* pCmd = lpszCommand;
//	char szParam[1024];
//	UINT nPos = 0;
//	bool bString = false;
//	while( *pCmd && !isalnum( *pCmd ) && 
//		*pCmd != '[' &&
//		*pCmd != '"' &&
//		*pCmd != '.' &&
//		*pCmd != '_' && 
//		*pCmd != '/' && 
//		*pCmd != '~' && 
//		*pCmd != '\\'
//		) ++pCmd;
//	while( *pCmd )
//	{
//		if( *pCmd == '[' || (!bString&&*pCmd == '"') )
//		{
//			++pCmd;
//			bString = true;
//		}
//		else
//		{
//			szParam[nPos++] = *pCmd++;
//		}
//
//		if( *pCmd == 0 || ( bString?( ( (']'==*pCmd)||('"'==*pCmd) ) && ++pCmd ):
//
//			( 
//			*pCmd != '.' &&
//			*pCmd != '_' && 
//			*pCmd != '/' && 
//			*pCmd != '~' && 
//			*pCmd != '\\' &&
//			!isalnum( *pCmd ) 
//			) ) )
//		{
//			szParam[nPos] = 0;
//			param.push_back( szParam );
//			nPos = 0;
//			while( *pCmd && !isalnum( *pCmd ) && *pCmd != '[' && *pCmd != '"' ) ++pCmd;
//			bString = false;
//			continue;
//		}
//	}
//
//	return true;
//}
//
//LuaDebugCommander::LuaDebugCommander(void)
//: m_hPipe( INVALID_HANDLE_VALUE )
//, m_hThread( INVALID_HANDLE_VALUE )
//, m_bWork( TRUE )
//, m_mode( lua_stop )
//, m_RetFunc( NULL )
//, m_buffer_head( NULL )
//, m_buffer_tail( NULL )
//{
//}
//
//LuaDebugCommander::~LuaDebugCommander(void)
//{
//	m_bWork = FALSE;
//	WaitForSingleObject( m_hThread, INFINITE );
//	CloseHandle( m_hSignal );
//	CloseHandle( m_hPipe );
//}
//
//bool LuaDebugCommander::initialize( const char* lpszPipename, ProcessRetCmd fn )
//{
//	int retry = 10;
//	m_RetFunc = fn;
//	std::string strPipename( "\\\\.\\pipe\\" );
//	strPipename.append( lpszPipename );
//	while( retry )
//	{
//		m_hPipe = CreateFile( strPipename.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
//		if( m_hPipe != INVALID_HANDLE_VALUE )
//		{
//			break;
//		}
//
//		DWORD dwRet = GetLastError();
//
//		if( dwRet == ERROR_FILE_NOT_FOUND )
//		{
//			Sleep(1000);
//			--retry;
//			continue;
//		}
//		else if( dwRet != ERROR_PIPE_BUSY) 
//		{
//			return false;
//		}
//
//		if( !WaitNamedPipe(lpszPipename, 10000) )
//		{ 
//			return false;
//		}
//	}
//
//	DWORD dwMode = PIPE_READMODE_MESSAGE;
//	if( !SetNamedPipeHandleState( m_hPipe, &dwMode, NULL, NULL ) )
//	{
//		printf("SetNamedPipeHandleState failed"); 
//		return false;
//	}
//
//	m_hSignal = CreateEvent( NULL, TRUE, FALSE, NULL );
//	m_hThread = (HANDLE)_beginthreadex( NULL, 0, LuaDebugCommander::pipe, this, 0, NULL );
//	return true;
//}
//
//bool LuaDebugCommander::waitSignal( DWORD dwTime )
//{
//	DWORD ret = WaitForSingleObject( m_hSignal, INFINITE );
//	if( ret == WAIT_OBJECT_0 )
//	{
//		ResetEvent( m_hSignal );
//		return true;
//	}
//	return false;
//}
//
//void LuaDebugCommander::Signal()
//{
//	SetEvent( m_hSignal );
//}
//
//bool LuaDebugCommander::command( const char* cmd )
//{
//	DWORD dwWrite = 0;
//	releaseBuffer( getBuffer() );
//	return WriteFile( m_hPipe, cmd, (DWORD)(strlen(cmd)+1), &dwWrite, NULL ) == TRUE;
//}
//
//_command_buffer* LuaDebugCommander::result()
//{
//	DWORD dwRead = 0;
//	if( PeekNamedPipe( m_hPipe, NULL, 0, NULL, NULL, &dwRead ) && dwRead > 0 )
//	{
//		_command_buffer* b = new _command_buffer;
//		if( ReadFile( m_hPipe, b->data, _countof(b->data), (DWORD*)&b->size, NULL ) )
//		{
//			b->data[b->size]	= 0;
//			b->data[b->size+1]	= 0;
//
//			if( memcmp( "~!@#$%^&*()?", b->data, b->size ) == 0 )
//			{
//				delete b;
//				Signal();
//				return  NULL;
//			}
//
//			return b;
//		}
//	}
//
//	return false;
//}
//
//_command_buffer*	LuaDebugCommander::getBuffer()
//{
//	_command_buffer* tmp = m_buffer_head;
//	m_buffer_head = m_buffer_tail = NULL;
//	return tmp;
//}
//
//void LuaDebugCommander::releaseBuffer( _command_buffer* buf )
//{
//	while( buf )
//	{
//		_command_buffer* tmp = buf;
//		buf = buf->next;
//		delete tmp;
//	}
//}
//
//unsigned int __stdcall LuaDebugCommander::pipe( void* param )
//{
//	LuaDebugCommander* pCommander = (LuaDebugCommander*)param;
//	if( pCommander )
//	{
//		while( pCommander->m_bWork )
//		{
//			_command_buffer* buf = pCommander->result();
//			if( buf && pCommander->m_RetFunc != NULL )
//			{
//				buf->next = NULL;
//				if( pCommander->m_RetFunc( buf->data, buf->size, sizeof( buf->data ) ) )
//				{
//					if( pCommander->m_buffer_tail )
//					{
//						pCommander->m_buffer_tail->next = buf;
//						pCommander->m_buffer_tail = buf;
//					}
//					else
//					{
//						pCommander->m_buffer_head = pCommander->m_buffer_tail = buf;
//					}
//				}
//				else
//				{
//					pCommander->releaseBuffer( buf );
//				}
//			}
//			else
//			{
//				Sleep( 1 );
//			}
//		}
//	}
//	return 0;
//}
//
//LuaDebugCommander* Create_Commander( const char* pipe, ProcessRetCmd fn )
//{
//	LuaDebugCommander* pCommander = new LuaDebugCommander();
//	if( !pCommander->initialize( XA2T(pipe), fn ) )
//	{
//		Destroy_Commander( pCommander );
//		return NULL;
//	}
//	return pCommander;
//}
//
//_command_buffer* Debug_Command( LuaDebugCommander* Debuger, const char* szFmt, ... )
//{
//	if( !Debuger ) return NULL;
//
//	char sz[4096];
//	va_list args;
//	va_start(args, szFmt);
//
//	size_t nSize = _countof( sz );
//	int size = _vsnprintf( sz, nSize, szFmt, args );
//	va_end(args);
//	if( size < 0 )	return NULL;
//	sz[nSize-1] = 0;
//
//	if( Debuger->command( XA2T(sz) ) )
//	{
//		Debuger->waitSignal( 2000 );
//		return Debuger->getBuffer();
//	}
//	return NULL;
//}
//
//void Debug_ReleaseBuffer( LuaDebugCommander* Debuger, _command_buffer* buf )
//{
//	Debuger->releaseBuffer( buf );
//}
//
//void Destroy_Commander( LuaDebugCommander* Debuger )
//{
//	delete Debuger;
//}
