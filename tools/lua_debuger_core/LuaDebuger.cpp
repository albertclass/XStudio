#include "StdAfx.h"
#include "LuaDebuger.h"
#include <io.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <direct.h>
#include <time.h>
#include "DebugerInterface.h"

struct LuaDebuger::Impl
{
	Impl()
		: call_level( 0 )
		, stop_level( 0 )
		, runmode( run )
		, begin( 0 )
	{
	}

	~Impl()
	{
	}

	struct breakinfo
	{
		std::vector< _string >	file;		// 所有源文件
		std::set< size_t >		breakline;	// 该文件内的所有断点行记录
	};

	typedef std::set< size_t >				line_set;
	typedef std::map< _string, breakinfo >	break_map;

	// 运行模式
	enum run_mode { run = 0, stop, step, stepin, stepout, mode_count };

	struct variant
	{
		variant( const char* n, const char* v, int i )
			: name( n )		// 变量名
			, value( v )	// 变量值
			, idx( i )		// 变量索引
		{

		}

		int			idx;
		std::string	name;
		std::string	value;
	};

	typedef std::list< variant >	variant_list;
	struct stackframe
	{
		int				currentline;
		std::string		filename;
		std::string		funcname;
		std::string		what;
		variant_list	variants;
	};

	struct luastack :public std::deque< stackframe* >
	{
		lua_State	*L;
		int			current;	// 当前查看的堆栈位置
	};

	// void listtable( stackframe* f, int idx );
	break_map	breakpoints;
	run_mode	runmode;

	int			call_level;	// 当前堆栈深度
	int			stop_level;	// 暂停时的堆栈深度
	size_t		begin;		// 当前文件游标

	luastack	lstack;			// lua 栈
	std::string	strFilename;	// 当前文件
	section	breakmap_lock;
};

struct LuaDebuger::ThreadParam
{
	ThreadParam( LuaDebuger* p, const char* lpszPipename, bool (LuaDebuger::* c)( const char* lpszCmd ) )
		: pThis( p )
		, strPipename( lpszPipename )
		, command( c )
	{
		debug_signal = CreateEvent( NULL, FALSE, FALSE, NULL );
		owner_thread_id		= GetCurrentThreadId();
		owner_process_id	= GetCurrentProcessId();
	}

	~ThreadParam()
	{
		if( bWork )
		{
			char buffer[2048];
			_string _strPipename = 
				"\\\\.\\pipe\\lua\\" + 
				strPipename + 
				"." +
				_itoa( owner_process_id, (char*)buffer, 10 ) + 
				"." +
				_itoa( owner_thread_id, (char*)buffer+1024, 10 );

			bWork = false;
			HANDLE p = CreateFile( _strPipename.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
			CloseHandle( p );
		}
		WaitForSingleObject( thread, INFINITE );
		CloseHandle( thread );
		CloseHandle( debug_signal );
	}

	bool _call( const char* lpszCmd )
	{
		return (pThis->*command)( lpszCmd );
	}

	typedef void (LuaDebuger::* instruct)( const char* lpszParams );
	typedef struct instruct_map : public std::map< _string, instruct >
	{
		instruct_map()
		{
			// insert( std::make_pair( _T(""), &LuaDebuger::cmd_ ) );
			insert( std::make_pair( _T("bp"),		&LuaDebuger::cmd_breakpoint ) );
			insert( std::make_pair( _T("check"),	&LuaDebuger::cmd_checkpoint ) );
			insert( std::make_pair( _T("clr"),		&LuaDebuger::cmd_clearpoint ) );

			insert( std::make_pair( _T("stack"),	&LuaDebuger::cmd_stack ) );
			insert( std::make_pair( _T("step"),		&LuaDebuger::cmd_step ) );
			insert( std::make_pair( _T("stepin"),	&LuaDebuger::cmd_stepin ) );
			insert( std::make_pair( _T("stepout"),	&LuaDebuger::cmd_stepout ) );
			insert( std::make_pair( _T("run"),		&LuaDebuger::cmd_run ) );
			insert( std::make_pair( _T("open"),		&LuaDebuger::cmd_open ) );
			insert( std::make_pair( _T("dir"),		&LuaDebuger::cmd_dir ) );
			insert( std::make_pair( _T("cd"),		&LuaDebuger::cmd_cd ) );
			insert( std::make_pair( _T("list"),		&LuaDebuger::cmd_list ) );
		}
	};

	HANDLE	thread;
	HANDLE	debug_signal;
	HANDLE	pipe;
	int		owner_thread_id, owner_process_id;
	bool	bWork;
	_string strPipename;
	static instruct_map	instructs;	// 指令映射表

private:
	LuaDebuger*	pThis;
	bool (LuaDebuger::* command)( const char* lpszCmd )	;
};

LuaDebuger::ThreadParam::instruct_map	LuaDebuger::ThreadParam::instructs;	// 指令映射表

LuaDebuger::LuaDebuger()
: m_pImpl( new Impl )
{

}

LuaDebuger::~LuaDebuger()
{
	SAFE_DELETE( m_thread_param );
	delete m_pImpl;
	m_pImpl = NULL;
}

void LuaDebuger::bp( const char* name, int line )
{
	if( name != NULL && line >= 0 )
	{
		autolock _l( m_pImpl->breakmap_lock);
		Impl::break_map::iterator iter = m_pImpl->breakpoints.find( name );
		if( iter != m_pImpl->breakpoints.end() )
		{
			iter->second.breakline.insert( line );
			output( ("#success set break at %s, line %d\n"), name, line );
		}
		else
		{
			output( ("#error file %s are not open!\n"), name );
		}
	}
	else
	{
		output( "#error param format.\n" );
	}
}

void LuaDebuger::run( int mode )
{
	if( mode >= 0 && mode < Impl::mode_count )
	{
		m_pImpl->runmode = Impl::run_mode( mode );
		Signal();
	}
}

bool LuaDebuger::judgeBreak( const char* name, int line )
{
	autolock _l( m_pImpl->breakmap_lock);
	char szFull[_MAX_DIR + _MAX_FNAME + _MAX_EXT];
	if( _fullpath( szFull, name, _countof(szFull) ) == 0 )
	{
		return false;
	}

	_string filename = XA2T( szFull );
	std::transform( filename.begin(), filename.end(), filename.begin(), tolower );
	Impl::break_map::const_iterator citer = m_pImpl->breakpoints.find( filename );
	if( citer != m_pImpl->breakpoints.end() )
	{
		const Impl::line_set &lineset = citer->second.breakline;
		Impl::line_set::const_iterator cline = lineset.find( line );
		if( cline != lineset.end() )
		{
			return true;
		}
	}
	return false;
}

bool LuaDebuger::waitSignal( lua_State *L )
{
	if( m_thread_param == NULL ) 
	{
		return false;
	}

	DWORD dwRet = WaitForSingleObject( m_thread_param->debug_signal, INFINITE );
	if( dwRet != WAIT_OBJECT_0 )
	{
		luaL_error( L, "debug signal error." );
		return false;
	}
	return true;
}

void LuaDebuger::Signal()
{
	SetEvent( m_thread_param->debug_signal );
}

static void line_hook( LuaDebuger* pDebuger, lua_State *L, lua_Debug *ar )
{
	switch( pDebuger->m_pImpl->runmode )
	{
	case LuaDebuger::Impl::stop:
		luaL_error( L, "debug stop." );
		break;
	case LuaDebuger::Impl::stepin:
		if( pDebuger->m_pImpl->stop_level <= pDebuger->m_pImpl->call_level )
		{
			pDebuger->makestack(L,ar);
			pDebuger->waitSignal(L);
			pDebuger->clearstack();
		}
		break;
	case LuaDebuger::Impl::stepout:
	case LuaDebuger::Impl::run:
		lua_getinfo( L, "S", ar );
		if( ar->source[0] == '@' && pDebuger->judgeBreak( ar->source+1, ar->currentline ) )
		{
			case LuaDebuger::Impl::step:
			pDebuger->makestack(L,ar);
			pDebuger->waitSignal(L);
			pDebuger->clearstack();
		}
		break;
	}
}

static void call_hook( LuaDebuger* pDebuger, lua_State *L, lua_Debug *ar )
{
	++ pDebuger->m_pImpl->call_level;
}

static void ret_hook( LuaDebuger* pDebuger, lua_State *L, lua_Debug *ar )
{
	-- pDebuger->m_pImpl->call_level;
}

static void count_hook( LuaDebuger* pDebuger, lua_State *L, lua_Debug *ar )
{

}

static void Debug(lua_State *L, lua_Debug *ar)
{
	lua_getglobal( L, "__debuger" );
	LuaDebuger* pDebuger = (LuaDebuger*)lua_touserdata( L, -1 );

	switch( ar->event )
	{
	case LUA_HOOKCOUNT:
		count_hook( pDebuger, L, ar);
		break;

	case LUA_HOOKCALL:
		call_hook( pDebuger, L, ar);
		break;

	case LUA_HOOKRET:
	case LUA_HOOKTAILRET:	//verify
		ret_hook( pDebuger, L, ar);
		break;

	case LUA_HOOKLINE:
		line_hook( pDebuger, L, ar );
		break;
	}
}

void LuaDebuger::makestack( lua_State *L, lua_Debug *ar )
{
	m_pImpl->lstack.L = L;
	m_pImpl->stop_level = m_pImpl->call_level;

	char szFull[_MAX_DIR+_MAX_PATH+_MAX_FNAME+_MAX_EXT];
	for ( int level = 0; lua_getstack(L, level, ar ); level++)
	{
		lua_getinfo( L, "Slnu", ar );
		Impl::stackframe* sf = new Impl::stackframe();
		sf->currentline	= ar->currentline;
		sf->funcname	= ar->name?XA2T( ar->name ):_T("");
		if( ar->source[0] != '@' || _fullpath(szFull, ar->source+1, _countof(szFull) ) == 0 ) szFull[0] = 0;
		sf->filename	= XA2T(szFull);
		std::transform( sf->filename.begin(), sf->filename.end(), sf->filename.begin(), tolower );
		sf->what		= ar->what?XA2T( ar->what ):_T("");
		const char* varname		= NULL;
		std::string varvalue;
		for( int index = 1; varname = lua_getlocal( L, ar, index ); ++index )
		{
			int top = lua_gettop(L);
			int t = lua_type( L, top );
			switch( t )
			{
			case LUA_TBOOLEAN:
				varvalue = lua_toboolean( L, top )?"true":"false";
				break;
			case LUA_TNUMBER:
				varvalue = lua_tostring( L, top );
				break;
			case LUA_TSTRING:
				varvalue = std::string("\"") + lua_tostring( L, top ) + "\"";
				break;
			case LUA_TTABLE:
				varvalue = "table";
				break;
			case LUA_TUSERDATA:
				varvalue = "userdata";
				break;
			case LUA_TLIGHTUSERDATA:
				varvalue = "lightuserdata";
				break;
			case LUA_TNIL:
				varvalue = "nil";
				break;
			case LUA_TFUNCTION:
				varvalue = "function";
				break;
			case LUA_TTHREAD:
				varvalue = "thread";
				break;
			}
			sf->variants.push_back( Impl::variant( varname, varvalue.c_str(), index ) );
			lua_pop(L,1);
		}
		m_pImpl->lstack.push_back( sf );
	}

	// 输出当前行
	Impl::stackframe* sf = m_pImpl->lstack.front();
	if( sf )
	{
		m_pImpl->begin = sf->currentline;
		m_pImpl->strFilename = sf->filename;
		autolock _l( m_pImpl->breakmap_lock );

		Impl::break_map::const_iterator c = m_pImpl->breakpoints.find( sf->filename );
		if( c != m_pImpl->breakpoints.end() )
		{
			if( sf->currentline >= 0 && sf->currentline < (int)c->second.file.size() )
			{
				output( _T("@break \"%s\" %d"), sf->filename.c_str(), sf->currentline );
				output( _T("$%04d > %s"), sf->currentline, c->second.file[sf->currentline].c_str() );
			}
		}
	}
}

void LuaDebuger::clearstack()
{
	// clear all old stackframe;
	for( Impl::luastack::iterator i = m_pImpl->lstack.begin(); i !=  m_pImpl->lstack.end(); ++i )
	{
		delete *i;
	}
	m_pImpl->lstack.clear();
}

unsigned int __stdcall LuaDebuger::guard( void *param )
{
	LuaDebuger::ThreadParam* p = ( LuaDebuger::ThreadParam* )param;

	BYTE	buffer[BUFSIZE];
	DWORD	dwRead, dwWrite;
	_string strPipename = 
		"\\\\.\\pipe\\lua\\" + 
		p->strPipename + 
		"." +
		_itoa( p->owner_process_id, (char*)buffer, 10 ) + 
		"." +
		_itoa( p->owner_thread_id, (char*)buffer+1024, 10 );

	p->bWork = true;
	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and the loop is repeated. 

	while( p->bWork )
	{ 
		p->pipe = CreateNamedPipe( 
			strPipename.c_str(),      // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			1,						  // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
			NULL);                    // default security attribute 

		if (p->pipe == INVALID_HANDLE_VALUE) 
		{
			printf("CreatePipe failed"); 
			return 0;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 


		BOOL bConnected = ConnectNamedPipe(p->pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		while( bConnected )
		{
			if( PeekNamedPipe( p->pipe, NULL, 0, NULL, NULL, &dwRead ) && dwRead > 0 )
			{
				BOOL ret = ReadFile( p->pipe, buffer, sizeof(buffer), &dwRead, NULL);
				if( ret )
				{
					p->_call( (const char*)buffer );
					WriteFile( p->pipe, "~!@#$%^&*()?", 12, &dwWrite, NULL );
				}
			}
			else 
			{
				DWORD dwCode = GetLastError();
				if( dwCode == ERROR_BROKEN_PIPE ||
					dwCode == ERROR_INVALID_HANDLE ||
					dwCode == ERROR_PIPE_CONNECTED
					)
				{
					p->_call( "clr all" );
					p->_call( "run" );
					break;
				}
				else
				{
					Sleep( 1 );
				}
			}
		}

		DisconnectNamedPipe( p->pipe );
		CloseHandle( p->pipe );
	}

	return 0;
}

bool LuaDebuger::initialize( lua_State* L, const char* lpszPipename )
{
	m_thread_param = new ThreadParam( this, lpszPipename, &LuaDebuger::command );
	
	lua_sethook( L, Debug, LUA_MASKCALL|LUA_MASKLINE|LUA_MASKRET|LUA_MASKCOUNT, 1 );
	lua_pushlightuserdata( L, this );
	lua_setglobal( L, "__debuger" );

	m_thread_param->thread = (HANDLE)_beginthreadex( NULL, 0, guard, m_thread_param, 0, NULL );

	return true;
}

bool LuaDebuger::command( const char* lpszCmd )
{
	const char* pCmd = lpszCmd;
	while( *pCmd && isalnum( *pCmd ) ) ++pCmd;

	_string strCmd( lpszCmd, pCmd - lpszCmd );
	while( *pCmd == ' ' ) ++pCmd;

	ThreadParam::instruct_map::iterator iter = m_thread_param->instructs.find( strCmd );
	if( iter != m_thread_param->instructs.end() )
	{
		(this->* iter->second)( pCmd );
	}
	else
	{
		output( "$command not exist.\n" );
	}
	return true;
}

int	 LuaDebuger::output( const char* szFmt, ... )
{
	char tszLog[4096];
	va_list args;
	va_start(args, szFmt);

	size_t nSize = _countof( tszLog );
	int size = _vsnprintf( tszLog, nSize, szFmt, args );
	va_end(args);
	if( size < 0 )	return 0;
	tszLog[nSize-1] = 0;

	WriteFile( m_thread_param->pipe, tszLog, size, (DWORD*)&size, NULL );
	return size;
}

void LuaDebuger::cmd_breakpoint( const char* lpszParam )
{
	const char* p = lpszParam;
	while( isdigit( *p ) ) ++p;
	if( *p == 0 && p != lpszParam )
	{
		// 数字
		int		line;
		if( sscanf( lpszParam, "%d", &line ) == 1 )
		{
			bp( m_pImpl->strFilename.c_str(), line );
		}
	}
}

void LuaDebuger::cmd_checkpoint( const char* lpszParam )
{
	Params params;
	PraseString( lpszParam, params );
	if( params.size() == 2 )
	{
		autolock _l( m_pImpl->breakmap_lock);
		std::transform( params[0].begin(), params[0].end(), params[0].begin(), tolower );
		Impl::break_map::const_iterator c = m_pImpl->breakpoints.find( params[0].c_str() );
		if( c != m_pImpl->breakpoints.end() )
		{
			Impl::line_set::const_iterator l = c->second.breakline.find( atoi( params[1].c_str() ) );
			if( l != c->second.breakline.end() )
			{
				output( "@true" );
			}
			else
			{
				output( "@false" );
			}
		}
		else
		{
			output( "#error file not open.\n" );
		}
	}
	else
	{
		output( "#error input\n" );
	}
}

void LuaDebuger::cmd_clearpoint( const char* lpszParam )
{
	if( _stricmp( lpszParam, ("all") ) == 0 )
	{
		autolock _l( m_pImpl->breakmap_lock);
		Impl::break_map::iterator iter = m_pImpl->breakpoints.find( m_pImpl->strFilename );
		if( iter != m_pImpl->breakpoints.end() )
		{
			iter->second.breakline.clear();
		}
	}
	else
	{
		int		line;
		sscanf( lpszParam, "%d", &line );

		autolock _l( m_pImpl->breakmap_lock);
		Impl::break_map::iterator iter = m_pImpl->breakpoints.find( m_pImpl->strFilename );
		if( iter != m_pImpl->breakpoints.end() )
		{
			iter->second.breakline.erase( line );
			output( "#success clear break.\n" );
		}
		else
		{
			output( "#error break not found.\n" );
		}
	}
}

void LuaDebuger::cmd_step( const char* lpszParam )
{
	run( Impl::step );
}

void LuaDebuger::cmd_stepout( const char* lpszParam )
{
	run( Impl::stepout );
}

void LuaDebuger::cmd_stepin( const char* lpszParam )
{
	run( Impl::stepin );
}

void LuaDebuger::cmd_run( const char* lpszParam )
{
	run( Impl::run );
}

void LuaDebuger::cmd_stack( const char* lpszParam )
{
	size_t idx = 0;
	size_t n = sscanf( lpszParam, "%d", &idx );
	switch( n )
	{
	case EOF:
	case 0:
		idx = m_pImpl->lstack.size();
		output( ("$%4s|%15s|%8s|%04s|%.30s\n"), ("idx"), ("function name"), ("what"), ("line"), ("file name") );
		output( ("$----|---------------|--------|----|----\n"), ('-') );
		for( Impl::luastack::iterator i = m_pImpl->lstack.begin(); i !=  m_pImpl->lstack.end(); ++i )
		{
			Impl::stackframe* sf = *i;
			output( 
				("#%04d|%15s|%8s|%04d|\"%s\"\n")
				, --idx
				, sf->funcname.empty()?"\"\"":sf->funcname.c_str()
				, sf->what.empty()?"\"\"":sf->what.c_str()
				, sf->currentline
				, sf->filename.empty()?"\"\"":sf->filename.c_str() 
				);
		}
		break;
	case 1:
		if( idx >= 0 && idx < m_pImpl->lstack.size() )
		{
			Impl::stackframe* sf = m_pImpl->lstack[idx];
			Impl::variant_list::iterator i = sf->variants.begin();
			n = 0;
			while( i != sf->variants.end() )
			{
				Impl::variant& v = (*i);
				output( "#%2d | %20s = %s\n", n++, v.name.c_str(), v.value.c_str() );
				++i;
			}
		}
	}
}

void LuaDebuger::cmd_open( const char* lpszParam )
{
	if( strlen( lpszParam ) == 0 )
	{
		autolock _l( m_pImpl->breakmap_lock );
		Impl::break_map::const_iterator c = m_pImpl->breakpoints.begin();
		int i = 1;
		while( c != m_pImpl->breakpoints.end() )
		{
			TCHAR mark = m_pImpl->strFilename == c->first?'*':' ';
			output( "#%c%02d | %s\n", mark, i, c->first.c_str() );
			++c;
			++i;
		}
	}
	else if( _access( lpszParam, 0 ) != -1 )
	{
		char szFull[_MAX_PATH];
		if( _fullpath( szFull, lpszParam, _countof(szFull) ) )
		{
			_strlwr( szFull );
			autolock _l( m_pImpl->breakmap_lock );
			Impl::break_map::const_iterator c = m_pImpl->breakpoints.find( szFull );
			if( c == m_pImpl->breakpoints.end() )
			{
				FILE* fp = fopen( szFull, _T("r") );
				// 读取文件
				if( fp != NULL )
				{
					TCHAR szLine[1024*4];
					
					Impl::breakinfo &info = m_pImpl->breakpoints[szFull];
					info.file.clear();
					info.breakline.clear();

					info.file.push_back( "\n" );
					while( !feof(fp) )
					{
						fgets( szLine, _countof(szLine), fp );
						info.file.push_back( szLine );
					}

					fclose( fp );
					output( "$file %s opened!\n", szFull );
				}
			}
		}
		m_pImpl->strFilename = szFull;
		output( "#success now set current file is %s\n", szFull );
	}
	else
	{
		const char* p = lpszParam;
		while( isdigit( *p ) ) ++p;
		if( *p == 0 && p != lpszParam )
		{
			autolock _l( m_pImpl->breakmap_lock );
			Impl::break_map::const_iterator c = m_pImpl->breakpoints.begin();
			int i = 1;
			int line = atoi( lpszParam );
			while( c != m_pImpl->breakpoints.end() )
			{
				if( i == line )
				{
					m_pImpl->strFilename = c->first;
					m_pImpl->begin = 0;
					output( "#success now set current file is %s\n", m_pImpl->strFilename.c_str() );
					break;
				}
				++c;
				++i;
			}
		}
	}
}

void LuaDebuger::cmd_cd( const char* lpszParam )
{
	char szFull[_MAX_PATH];
	if( _fullpath( szFull, lpszParam, _MAX_PATH ) != NULL )
	{
		_chdir( szFull );
		output( "$change director at :%s\n", szFull );
	}
}

void LuaDebuger::cmd_dir( const char* lpszParam )
{
	struct _tfinddata_t c_file;
	intptr_t hFile;

	// Find first .c file in current directory 
	if( (hFile = _findfirst( "*.*", &c_file )) != -1L )
	{
		output( "$Listing of all files\n\n" );
		output( "$RDO HID SYS ARC  DATA%25c SIZE      FILE\n", ' ' );
		output( "$--- --- --- ---  ----%25c ----      ----\n", ' ' );
		do 
		{
			char buffer[30];
			output( ( c_file.attrib & _A_RDONLY ) ? "$ Y  " : "$ N  " );
			output( ( c_file.attrib & _A_SYSTEM ) ? "$ Y  " : "$ N  " );
			output( ( c_file.attrib & _A_HIDDEN ) ? "$ Y  " : "$ N  " );
			output( ( c_file.attrib & _A_ARCH )   ? "$ Y  " : "$ N  " );
			ctime_s( buffer, _countof(buffer), &c_file.time_write );
			output( "$ %.24s  %9ld %s \n", buffer, c_file.size, c_file.name );
		} while( _findnext( hFile, &c_file ) == 0 );
		_findclose( hFile );
	}
}

void LuaDebuger::cmd_list( const char* lpszParam )
{
	autolock _l( m_pImpl->breakmap_lock );
	Impl::break_map::const_iterator c = m_pImpl->breakpoints.find( m_pImpl->strFilename );
	if( c != m_pImpl->breakpoints.end() )
	{
		size_t begin	= m_pImpl->begin;
		size_t end		= __min( begin + 20 , c->second.file.size() );
		size_t n = sscanf( lpszParam, _T("%d %d"), &begin, &end );

		switch( n )
		{
		case EOF:
		case 0:
			break;
		case 1:
			// 输入了显示行数
			end	= __min( begin + 20, c->second.file.size() );
			break;
		case 2:
			if( end < begin ) 
			{
				output( "#error for input begin > end.\n" );
				return;
			}
			break;
		}

		for( size_t i = begin; i < end; ++i )
		{
			const _string& l = c->second.file[i];
			bool b = ( c->second.breakline.find( i ) != c->second.breakline.end() );
			output( "$%c %04u> %s", b?_T('@'):_T(' '), i, l.c_str() );
		}
		m_pImpl->begin = __min( end + 1, c->second.file.size() );
	}
}