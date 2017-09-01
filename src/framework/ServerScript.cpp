#include "ServerDefines.h"
#include "ServerScript.h"

static lua_State* _Lua = xgc_nullptr;

template< int Int >
struct logLevelTag
{
	enum { lv = Int };
	static xgc_lpcstr Tag;
};

xgc_lpcstr logLevelTag<0>::Tag = "DBG";
xgc_lpcstr logLevelTag<1>::Tag = "INF";
xgc_lpcstr logLevelTag<2>::Tag = "WRN";
xgc_lpcstr logLevelTag<3>::Tag = "ERR";

///
/// \brief 脚本日志
/// \date 2017/08/28 
/// \author albert.xu
///
template< int Level >
static int luaLog( lua_State *_L )
{
	lua_Debug ar = { 0, xgc_nullptr, xgc_nullptr, xgc_nullptr, xgc_nullptr };

	if( lua_getstack( _Lua, 1, &ar ) )
	{
		lua_getinfo( _L, "Sl", &ar );

		ar.name ? ar.name : "(unknowe)";
		ar.source ? ar.source : "(null)";

		get_logger( "LUA" ).write( ar.source, ar.name, ar.currentline, logLevelTag<Level>::Tag, "(%s)%s", ar.what, lua_tostring( _L, -1 ) );
	}

	return 0;
}

///
/// \brief 初始化脚本模块
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool InitServerScript( ini_reader &ini )
{
	_Lua = luaL_newstate();
	XGC_ASSERT_RETURN( _Lua, false );

	luaL_openlibs( _Lua );

	getGlobalNamespace( _Lua )
		.addCFunction( "DBG", luaLog<0> )
		.addCFunction( "INF", luaLog<1> )
		.addCFunction( "WRN", luaLog<2> )
		.addCFunction( "ERR", luaLog<3> )
		;

	LuaRef package = getGlobal( _Lua, "package" );
	std::stringstream package_path( package["path"].cast< xgc_string >() );

	auto count = ini.get_item_count( "LuaScripts", "Path" );
	for( xgc_size i = 0; i < count; ++i )
	{
		auto path = ini.get_item_value( "LuaScripts", "Path", i, xgc_nullptr );
		list_directory( path, [&package_path]( xgc_lpcstr root, xgc_lpcstr relative, xgc_lpcstr file )->bool{
			if( xgc_nullptr == file )
			{
				package_path << root << "/" << relative << "/?;";
				package_path << root << "/" << relative << "/?.lua;";
			}

			return true;
		}, -1 );
	}

	package["path"] = package_path.str();
	
	auto luaMain = ini.get_item_value( "LuaScripts", "Main", "main.lua" );
	if( luaMain )
	{
		luaDoFile( luaMain );
	}

	return true;
}

///
/// \brief 清理脚本模块
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void FiniServerScript()
{

}

///
/// \brief 执行脚本
/// \date 2017/08/28 
/// \author albert.xu
///
lua_State* luaState()
{
	return _Lua;
}

///
/// \brief 获取脚本全局量
/// \date 2017/08/28 
/// \author albert.xu
///
LuaRef luaGlobal( xgc_lpcstr lpName )
{
	return getGlobal( _Lua, lpName );
}

///
/// \brief 杂交脚本和C++类
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaHybrid( LuaRef &luaTable, xgc_lpcstr lpClassName )
{

}

///
/// \brief 执行脚本
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoString( xgc_lpcstr lpScript )
{
	int t = lua_gettop( _Lua );
	int r = luaL_dostring( _Lua, lpScript );
	if( r != 0 )
	{
		SYS_ERROR( "执行脚本[%s]发生错误。\n%s", lpScript, lua_tostring( _Lua, -1 ) );
		return false;
	}

	lua_settop( _Lua, t );

	return true;
}

///
/// \brief 执行脚本文件
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoFile( xgc_lpcstr lpScriptFile )
{
	int t = lua_gettop( _Lua );

	int r = luaL_loadfile( _Lua, lpScriptFile );
	if( r != 0 )
	{
		SYS_ERROR( "脚本[%s]编译失败。\n%s", lpScriptFile, lua_tostring( _Lua, -1 ) );
		lua_pop( _Lua, 1 );
		return false;
	}

	r = lua_pcall( _Lua, 0, LUA_MULTRET, 0 );
	if( r != 0 )
	{
		SYS_ERROR( "脚本[%s]编译失败。\n%s", lpScriptFile, lua_tostring( _Lua, -1 ) );
		lua_pop( _Lua, 1 );
		return false;
	}

	lua_settop( _Lua, t );
	return true;
}

///
/// \brief 执行脚本文件
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_bool luaDoFileEx( xgc_lpcstr lpScriptFile, LuaRef* retVal, int &nCount )
{
	int t = lua_gettop( _Lua );
	int r = luaL_loadfile( _Lua, lpScriptFile );
	if( r != 0 )
	{
		SYS_ERROR( "脚本[%s]编译失败。\n%s", lpScriptFile, lua_tostring( _Lua, -1 ) );
		lua_pop( _Lua, 1 );
		return false;
	}

	r = lua_pcall( _Lua, 0, LUA_MULTRET, 0 );
	if( r != 0 )
	{
		SYS_ERROR( "脚本[%s]编译失败。\n%s", lpScriptFile, lua_tostring( _Lua, -1 ) );
		lua_pop( _Lua, 1 );
		return false;
	}

	int n = lua_gettop( _Lua ) - t;
	int c = XGC_MIN( n, nCount );
	for( int i = 0; i < c; ++i )
		retVal[i] = LuaRef::fromStack( _Lua, i - c );

	nCount = c;
	return true;
}

///
/// \brief 打印脚本堆栈
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaTraceback( xgc_lpcstr lpFileName, xgc_lpcstr lpFuncName, xgc_int32 nLine )
{
	char stack_string[1024];
	lua_Debug ar;

	int l = 1;
	int r = lua_getstack( _Lua, l, &ar );

	while( r == 1 )
	{
		lua_getinfo( _Lua, "Sl", &ar );
		int n = sprintf_s( stack_string, "(%s)%s(%d)\n", ar.what, ar.source, ar.currentline );
		if( n <= 0 ) break;

		r = lua_getstack( _Lua, ++l, &ar );
	}

	get_logger( "LUA" ).write( lpFileName, lpFuncName, nLine, "LUA", "%s", stack_string );
}

///
/// \brief 打印脚本栈元素
/// \date 2017/08/28 
/// \author albert.xu
///
xgc_void luaStackDump( xgc_lpcstr lpMark /*= "empty"*/ )
{
	int i;
	int t = lua_gettop( _Lua );
	DBG_INFO( "luaStackDump - Mark <%s> : top = %d", lpMark, t );
	for( i = 1; i <= t; ++i )
	{
		int t = lua_type( _Lua, i );
		switch( t )
		{
			case LUA_TSTRING:
			DBG_INFO( "  %d : %s", i, lua_tostring( _Lua, i ) );
			break;
			case LUA_TBOOLEAN:
			DBG_INFO( "  %d : %s", i, lua_toboolean( _Lua, i ) ? "true" : "false" );
			break;
			case LUA_TNUMBER:
			DBG_INFO( "  %d : dec(%llf); hex(%x); oct(%o)", i,
				lua_tonumber( _Lua, i ),
				(xgc_long)lua_tonumber( _Lua, i ),
				(xgc_long)lua_tonumber( _Lua, i ) );
			break;
			default:
			DBG_INFO( "  %d : type( %s )", i, lua_typename( _Lua, t ) );
			break;
		}
	}
}
