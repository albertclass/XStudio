#include "ServerDefines.h"
#include "ServerBase.h"
#include "ServerRefresh.h"
#include "ServerScript.h"

///
/// 刷新信息
/// [1/20/2015] create by albert.xu
///
struct RefreshInfo
{
	/// @var 刷新名
	xgc::string name;
	/// @var 刷新类型
	xgc::string params;
	/// @var 函数调用
	LuaRef call;
	/// @var 函数调用
	LuaRef args;
	/// @var 最后一次调用时间
	datetime next_invoke_time;
};

xgc_bool operator<( const RefreshInfo &lhs, const RefreshInfo &rhs )
{
	return lhs.next_invoke_time < rhs.next_invoke_time;
}

/// @var 刷新信息
static xgc::set< RefreshInfo > mRefreshSet;

xgc_bool InitServerRefresh( ini_reader &ini )
{
	if( !ini.is_exist_section( "RefreshConf" ) )
		return true;
	
	xgc_lpcstr lpConfName = ini.get_item_value( "RefreshConf", "ConfigName", xgc_nullptr );
	XGC_ASSERT_RETURN( lpConfName, false, "未配置RefreshConf.ConfigName" );
	xgc_lpcstr lpConfPath = ini.get_item_value( "RefreshConf", "ConfigPath", xgc_nullptr );
	XGC_ASSERT_RETURN( lpConfName, false, "未配置RefreshConf.ConfigPath" );

	xgc_char szConfPath[XGC_MAX_PATH];
	get_absolute_path( szConfPath, sizeof( szConfPath ), "%s", lpConfPath );

	LuaRef conf = luaGlobal( lpConfName );
	if( conf.isNil() )
	{
		SYS_ERR( "刷新配置项不存在" );
		return false;
	}

	RefreshInfo info { "", {0}, LuaRef( luaState() ), LuaRef( luaState() ), datetime() };

	LuaRef ref( luaState() );

	for( Iterator item( conf ); !item.isNil(); ++item )
	{
		auto val = item.value();

		ref = val["params"];
		auto params = ref ? ref.cast< xgc_lpcstr >() : xgc_nullptr;

		ref = val["name"];
		info.name = ref ? ref.cast< xgc_lpcstr >() : xgc_nullptr;

		auto invoke = val["invoke"];
		XGC_ASSERT_CONTINUE( invoke.isFunction() );

		info.params = params;
		info.call = invoke["call"];
		info.args = invoke["args"];

		ref = val["start"];
		auto start = ref ? ref.cast< xgc_lpcstr >() : xgc_nullptr;
		XGC_ASSERT_CONTINUE( start );

		info.next_invoke_time = datetime::convert( start );

		mRefreshSet.emplace( std::move( info ) );
	}

	return true;
}

xgc_void ExecServerRefresh( datetime now )
{
	auto it = mRefreshSet.begin();
	while( it != mRefreshSet.end() )
	{
		if( now < it->next_invoke_time )
			break;

		RefreshInfo info = *it;

		try
		{
			info.call.push( info.call.state() );
			int argc = 0;
			for( Iterator iter( info.args ); !iter.isNil(); ++iter, ++argc )
				(*iter).push( info.args.state() );

			LuaException::pcall( info.call.state(), argc, 0 );
		}
		catch( std::exception &err )
		{
			SYS_ERR( "Lua call error %s", err.what() );
		}

		info.next_invoke_time = adjust_cycle_next( info.next_invoke_time, info.params.c_str(), now );

		mRefreshSet.insert( info );

		it = mRefreshSet.begin();
	}
}

xgc_void FiniServerRefresh()
{
	mRefreshSet.clear();
}