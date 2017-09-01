#include "StdAfx.h"
#include "LuaDebugCommander.h"

typedef std::vector< std::string >	Params;
bool PraseString( const char* lpszCommand, Params& param )
{
	const char* pCmd = lpszCommand;
	char szParam[1024];
	UINT nPos = 0;
	bool bString = false;
	while( *pCmd && !isalnum( *pCmd ) && 
		*pCmd != '[' &&
		*pCmd != '"' &&
		*pCmd != '.' &&
		*pCmd != '_' && 
		*pCmd != '/' && 
		*pCmd != '~' && 
		*pCmd != '\\'
		) ++pCmd;
	while( *pCmd )
	{
		if( *pCmd == '[' || (!bString&&*pCmd == '"') )
		{
			++pCmd;
			bString = true;
		}
		else
		{
			szParam[nPos++] = *pCmd++;
		}

		if( *pCmd == 0 || ( bString?( ( (']'==*pCmd)||('"'==*pCmd) ) && ++pCmd ):

			( 
			*pCmd != '.' &&
			*pCmd != '_' && 
			*pCmd != '/' && 
			*pCmd != '~' && 
			*pCmd != '\\' &&
			!isalnum( *pCmd ) 
			) ) )
		{
			szParam[nPos] = 0;
			param.push_back( szParam );
			nPos = 0;
			while( *pCmd && !isalnum( *pCmd ) && *pCmd != '[' && *pCmd != '"' ) ++pCmd;
			bString = false;
			continue;
		}
	}

	return true;
}

LuaDebugCommander::LuaDebugCommander(void)
: m_hPipe( INVALID_HANDLE_VALUE )
, m_hThread( INVALID_HANDLE_VALUE )
, m_bWork( TRUE )
, m_mode( lua_stop )
, m_RetFunc( NULL )
, m_buffer_head( NULL )
, m_buffer_tail( NULL )
{
}

LuaDebugCommander::~LuaDebugCommander(void)
{
	m_bWork = FALSE;
	WaitForSingleObject( m_hThread, INFINITE );
	CloseHandle( m_hSignal );
	CloseHandle( m_hPipe );
}

bool LuaDebugCommander::initialize( const char* lpszPipename, ProcessRetCmd fn )
{
	int retry = 10;
	m_RetFunc = fn;
	std::string strPipename( "\\\\.\\pipe\\" );
	strPipename.append( lpszPipename );
	while( retry )
	{
		m_hPipe = CreateFile( strPipename.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
		if( m_hPipe != INVALID_HANDLE_VALUE )
		{
			break;
		}

		DWORD dwRet = GetLastError();

		if( dwRet == ERROR_FILE_NOT_FOUND )
		{
			Sleep(1000);
			--retry;
			continue;
		}
		else if( dwRet != ERROR_PIPE_BUSY) 
		{
			return false;
		}

		if( !WaitNamedPipe(lpszPipename, 10000) )
		{ 
			return false;
		}
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	if( !SetNamedPipeHandleState( m_hPipe, &dwMode, NULL, NULL ) )
	{
		printf("SetNamedPipeHandleState failed"); 
		return false;
	}

	m_hSignal = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hThread = (HANDLE)_beginthreadex( NULL, 0, LuaDebugCommander::pipe, this, 0, NULL );
	return true;
}

bool LuaDebugCommander::waitSignal( DWORD dwTime )
{
	DWORD ret = WaitForSingleObject( m_hSignal, INFINITE );
	if( ret == WAIT_OBJECT_0 )
	{
		ResetEvent( m_hSignal );
		return true;
	}
	return false;
}

void LuaDebugCommander::Signal()
{
	SetEvent( m_hSignal );
}

bool LuaDebugCommander::command( const char* cmd )
{
	DWORD dwWrite = 0;
	releaseBuffer( getBuffer() );
	return WriteFile( m_hPipe, cmd, (DWORD)(strlen(cmd)+1), &dwWrite, NULL ) == TRUE;
}

_command_buffer* LuaDebugCommander::result()
{
	DWORD dwRead = 0;
	if( PeekNamedPipe( m_hPipe, NULL, 0, NULL, NULL, &dwRead ) && dwRead > 0 )
	{
		_command_buffer* b = new _command_buffer;
		if( ReadFile( m_hPipe, b->data, _countof(b->data), (DWORD*)&b->size, NULL ) )
		{
			b->data[b->size]	= 0;
			b->data[b->size+1]	= 0;

			if( memcmp( "~!@#$%^&*()?", b->data, b->size ) == 0 )
			{
				delete b;
				Signal();
				return  NULL;
			}

			return b;
		}
	}

	return false;
}

_command_buffer*	LuaDebugCommander::getBuffer()
{
	_command_buffer* tmp = m_buffer_head;
	m_buffer_head = m_buffer_tail = NULL;
	return tmp;
}

void LuaDebugCommander::releaseBuffer( _command_buffer* buf )
{
	while( buf )
	{
		_command_buffer* tmp = buf;
		buf = buf->next;
		delete tmp;
	}
}

unsigned int __stdcall LuaDebugCommander::pipe( void* param )
{
	LuaDebugCommander* pCommander = (LuaDebugCommander*)param;
	if( pCommander )
	{
		while( pCommander->m_bWork )
		{
			_command_buffer* buf = pCommander->result();
			if( buf && pCommander->m_RetFunc != NULL )
			{
				buf->next = NULL;
				if( pCommander->m_RetFunc( buf->data, buf->size, sizeof( buf->data ) ) )
				{
					if( pCommander->m_buffer_tail )
					{
						pCommander->m_buffer_tail->next = buf;
						pCommander->m_buffer_tail = buf;
					}
					else
					{
						pCommander->m_buffer_head = pCommander->m_buffer_tail = buf;
					}
				}
				else
				{
					pCommander->releaseBuffer( buf );
				}
			}
			else
			{
				Sleep( 1 );
			}
		}
	}
	return 0;
}

LuaDebugCommander* Create_Commander( const char* pipe, ProcessRetCmd fn )
{
	LuaDebugCommander* pCommander = new LuaDebugCommander();
	if( !pCommander->initialize( XA2T(pipe), fn ) )
	{
		Destroy_Commander( pCommander );
		return NULL;
	}
	return pCommander;
}

_command_buffer* Debug_Command( LuaDebugCommander* Debuger, const char* szFmt, ... )
{
	if( !Debuger ) return NULL;

	char sz[4096];
	va_list args;
	va_start(args, szFmt);

	size_t nSize = _countof( sz );
	int size = _vsnprintf( sz, nSize, szFmt, args );
	va_end(args);
	if( size < 0 )	return NULL;
	sz[nSize-1] = 0;

	if( Debuger->command( XA2T(sz) ) )
	{
		Debuger->waitSignal( 2000 );
		return Debuger->getBuffer();
	}
	return NULL;
}

void Debug_ReleaseBuffer( LuaDebugCommander* Debuger, _command_buffer* buf )
{
	Debuger->releaseBuffer( buf );
}

void Destroy_Commander( LuaDebugCommander* Debuger )
{
	delete Debuger;
}
