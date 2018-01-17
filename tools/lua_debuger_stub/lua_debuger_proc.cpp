#include "lua_debuger.h"
#include "lua_debuger_sock.h"
#include "lua_debuger_proc.h"


/// 栈帧信息
struct stackframe
{
	int		line;
	string	file;
	string	func;
	string	what;
};

bool cmd_bp( lua_State* L, lua_Debug* ar, int argc, char *argv[] );
bool cmd_bc( lua_State* L, lua_Debug* ar, int argc, char *argv[] );
bool cmd_bl( lua_State* L, lua_Debug* ar, int argc, char *argv[] );
bool cmd_bt( lua_State* L, lua_Debug* ar, int argc, char *argv[] );

bool cmd_stp( lua_State* L, lua_Debug* ar, int argc, char *argv[] );
bool cmd_sti( lua_State* L, lua_Debug* ar, int argc, char *argv[] );
bool cmd_sto( lua_State* L, lua_Debug* ar, int argc, char *argv[] );
bool cmd_run( lua_State* L, lua_Debug* ar, int argc, char *argv[] );

long parse_cmd( char* cmd, long argc, char** argv )
{
	long icnt = 0;

	bool quot = false;
	bool find = true;

	while( *cmd && icnt < argc )
	{
		if( find )
		{
			if( cmd[0] == '"' && quot == true )
			{
				cmd[0] = '\0';
				quot = false;
				find = false;
			}

			if( cmd[0] == ' ' && quot == false )
			{
				cmd[0] = '\0';
				find = false;
			}
		}
		else if( cmd[0] == '"' )
		{
			find = true;
			quot = true;

			argv[icnt] = cmd + 1;
		}
		else if( cmd[0] != ' ' )
		{
			find = true;
			argv[icnt] = cmd;
		}

		++cmd;
	}

	return icnt;
}

bool execute_cmd( lua_State* L, lua_Debug* ar )
{
	// 定义命令缓冲
	char cmd[1024], *ptr = cmd;
	
	// 缓冲长度不够的，则重新分配
	if( dbg.ipkg > sizeof( cmd ) )
	{
		if( NULL == ( ptr = (char*)malloc( dbg.ipkg ) ) )
		{
			resp( "error memory bad_alloc. size = %d", dbg.ipkg.load() );
			// wait next command
			return true;
		}
	}

	// 跳过数据头
	memcpy( cmd, dbg.recv_buffer + sizeof( int ), dbg.ipkg - sizeof( int ) );

	int   argc = 0;
	char* argv[32];

	argc = parse_cmd( cmd, argc, argv );

	// 处理指令
	if( strcasecmp( "bp", argv[0] ) == 0 )
		return cmd_bp( L, ar, argc, argv );
	else if( strcasecmp( "bl" , argv[0] ) == 0 )
		return cmd_bl( L, ar, argc, argv );
	else if( strcasecmp( "bc" , argv[0] ) == 0 )
		return cmd_bc( L, ar, argc, argv );
	else if( strcasecmp( "bt" , argv[0] ) == 0 )
		return cmd_bt( L, ar, argc, argv );
	else if( strcasecmp( "stp", argv[0] ) == 0 )
		return cmd_stp( L, ar, argc, argv );
	else if( strcasecmp( "sti", argv[0] ) == 0 )
		return cmd_sti( L, ar, argc, argv );
	else if( strcasecmp( "sto", argv[0] ) == 0 )
		return cmd_sto( L, ar, argc, argv );
	else if( strcasecmp( "run", argv[0] ) == 0 )
		return cmd_run( L, ar, argc, argv );

	return true;
}

bool cmd_bp( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	int line = str2numeric< int >( argv[1] );
	auto ln = dbg.breakpoints_reg[line];
	auto it = ln.find( argv[2] );
	if( it == ln.end() )
	{
		ln.insert( argv[2] );
		dbg.breakpoints_ids[dbg.breakpoints_ids_max] = std::make_tuple( line, argv[2] );
	}

	resp( "ok" );

	return true;
}

bool cmd_bc( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	int id = str2numeric< int >( argv[1] );
	auto it = dbg.breakpoints_ids.find( id );
	if( it != dbg.breakpoints_ids.end() )
	{
		auto ln = dbg.breakpoints_reg.find( std::get< 0 >( it->second ) );
		if( ln != dbg.breakpoints_reg.end() )
		{
			auto &file = std::get< 1 >( it->second );
			ln->second.erase( file );
		}
	}

	resp( "ok" );
	return true;
}

bool cmd_bl( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	for( auto &it : dbg.breakpoints_ids )
		resp( "%02d - %s:%d", it.first, std::get< 1 >( it.second ).c_str(), std::get< 0 >( it.second ) );

	resp( "ok" );
	return true;
}

bool cmd_bt( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	lua_Debug frame;
	int level = 1;
	while( lua_getstack( L, level, &frame ) )
	{
		lua_getinfo( L, "Sln", &frame );

		int line = frame.currentline;
		const char *func = frame.name;
		const char *file = frame.source;
		const char *what = frame.what;

		resp( "%s(%d): %s", file, line, func ? func : "?" );
		++level;
	}
	
	resp( "ok" );
	return true;
}

bool cmd_stp( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	dbg.mode = debug_mode::e_step;
	return false;
}

bool cmd_sti( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	dbg.mode = debug_mode::e_step_in;
	return false;
}

bool cmd_sto( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	dbg.mode = debug_mode::e_step_out;
	return false;
}

bool cmd_run( lua_State* L, lua_Debug* ar, int argc, char *argv[] )
{
	dbg.mode = debug_mode::e_run;
	return false;
}


//void LuaDebuger::makestack( lua_State *L, lua_Debug *ar )
//{
//	m_pImpl->lstack.L = L;
//	m_pImpl->stop_level = m_pImpl->call_level;
//
//	char szFull[_MAX_DIR+_MAX_PATH+_MAX_FNAME+_MAX_EXT];
//	for ( int level = 0; lua_getstack(L, level, ar ); level++)
//	{
//		lua_getinfo( L, "Slnu", ar );
//		Impl::stackframe* sf = new Impl::stackframe();
//		sf->currentline	= ar->currentline;
//		sf->funcname	= ar->name?XA2T( ar->name ):_T("");
//		if( ar->source[0] != '@' || _fullpath(szFull, ar->source+1, _countof(szFull) ) == 0 ) szFull[0] = 0;
//		sf->filename	= XA2T(szFull);
//		std::transform( sf->filename.begin(), sf->filename.end(), sf->filename.begin(), tolower );
//		sf->what		= ar->what?XA2T( ar->what ):_T("");
//		const char* varname		= NULL;
//		std::string varvalue;
//		for( int index = 1; varname = lua_getlocal( L, ar, index ); ++index )
//		{
//			int top = lua_gettop(L);
//			int t = lua_type( L, top );
//			switch( t )
//			{
//			case LUA_TBOOLEAN:
//				varvalue = lua_toboolean( L, top )?"true":"false";
//				break;
//			case LUA_TNUMBER:
//				varvalue = lua_tostring( L, top );
//				break;
//			case LUA_TSTRING:
//				varvalue = std::string("\"") + lua_tostring( L, top ) + "\"";
//				break;
//			case LUA_TTABLE:
//				varvalue = "table";
//				break;
//			case LUA_TUSERDATA:
//				varvalue = "userdata";
//				break;
//			case LUA_TLIGHTUSERDATA:
//				varvalue = "lightuserdata";
//				break;
//			case LUA_TNIL:
//				varvalue = "nil";
//				break;
//			case LUA_TFUNCTION:
//				varvalue = "function";
//				break;
//			case LUA_TTHREAD:
//				varvalue = "thread";
//				break;
//			}
//			sf->variants.push_back( Impl::variant( varname, varvalue.c_str(), index ) );
//			lua_pop(L,1);
//		}
//		m_pImpl->lstack.push_back( sf );
//	}
//
//	// 输出当前行
//	Impl::stackframe* sf = m_pImpl->lstack.front();
//	if( sf )
//	{
//		m_pImpl->begin = sf->currentline;
//		m_pImpl->strFilename = sf->filename;
//		autolock _l( m_pImpl->breakmap_lock );
//
//		Impl::break_map::const_iterator c = m_pImpl->breakpoints.find( sf->filename );
//		if( c != m_pImpl->breakpoints.end() )
//		{
//			if( sf->currentline >= 0 && sf->currentline < (int)c->second.file.size() )
//			{
//				output( _T("@break \"%s\" %d"), sf->filename.c_str(), sf->currentline );
//				output( _T("$%04d > %s"), sf->currentline, c->second.file[sf->currentline].c_str() );
//			}
//		}
//	}
//}

