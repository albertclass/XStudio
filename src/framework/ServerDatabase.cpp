#include "ServerDefines.h"
#include "ServerDatabase.h"
#include "ServerBase.h"

using namespace xgc;
using namespace xgc::sql;

/// @var 数据库异步连接
static sql_database		g_async_h = 0;
static sql_connection	g_sync_h = 0;

xgc_bool InitServerDatabase( ini_reader &ini )
{
	FUNCTION_BEGIN;

	// 没有配置数据库，则认为成功
	if( !ini.is_exist_section( "Database") )
		return true;

	connection_cfg conn;
	// 数据库配置
	conn.host      = ini.get_item_value( "Database", "Host", xgc_nullptr );
	if( xgc_nullptr == conn.host )
	{
		SYS_ERROR( "%s", "数据库配置项 Database.Host 没有正确配置。" );
		return false;
	}
	conn.port      = ini.get_item_value( "Database", "Port", 3306 );
	conn.retry     = ini.get_item_value( "Database", "Retry", 3 );
	conn.username  = ini.get_item_value( "Database", "Username", xgc_nullptr );
	if( xgc_nullptr == conn.username )
	{
		SYS_ERROR( "%s", "数据库配置项 Database.Username 没有正确配置。" );
		return false;
	}
	conn.password = ini.get_item_value( "Database", "Password", xgc_nullptr );
	if( xgc_nullptr == conn.password )
	{
		SYS_ERROR( "%s", "数据库配置项 Database.Password 没有正确配置。" );
		return false;
	}
	conn.schema = ini.get_item_value( "Database", "Schema", xgc_nullptr );
	if( xgc_nullptr == conn.schema )
	{
		SYS_ERROR( "%s", "数据库配置项 Database.Schema 没有正确配置。" );
		return false;
	}
	conn.character = ini.get_item_value( "Database", "Character", "latin1" );

	xgc_lpcstr lpParentNode = "database";
	MemMark( "init", lpParentNode );
	if( false == sql_init_library() )
	{
		SYS_ERROR( "数据库底层库初始化失败。" );
		return false;
	}
	SYS_INFO( "数据库底层库初始化成功。" );

	MemMark( "async connect", lpParentNode );
	g_async_h = async_connect( conn );
	if( 0 == g_async_h )
	{
		SYS_ERROR( "数据库异步连接初始化失败。" );
		return false;
	}

	MemMark( "sync connect", lpParentNode );
	g_sync_h = connect( conn );
	if( 0 == g_sync_h )
	{
		SYS_ERROR( "同步数据库连接失败! %d - %s", get_error_code( g_sync_h ), get_error_info( g_sync_h ) );
		return false;
	}
	SYS_INFO( "数据库同步连接初始化成功。" );

	xgc_lpcstr lpConf = ini.get_item_value( "Database", "TableMaker", xgc_nullptr );
	if( lpConf )
	{
		xgc_char szTableConf[XGC_MAX_PATH];

		get_absolute_path( szTableConf, sizeof( szTableConf ), "%s", lpConf );
		if( false == make_tables( conn, szTableConf, xgc_nullptr ) )
			return false;
	}

	return true;
	FUNCTION_END;

	return false;
}

xgc_void FiniServerDatabase()
{
	FUNCTION_BEGIN;
	disconnect( g_sync_h );
	async_close( g_async_h );
	sql_fini_library();
	FUNCTION_END;
}

///
/// 同步执行数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_bool SyncDBExecute( xgc_lpcstr lpSql )
{
	return execute_sql( g_sync_h, lpSql );
}

///
/// 同步执行数据库操作
/// [12/18/2014] create by albert.xu
///
xgc_void SyncDBExecute( const postcall &post )
{
	auto resp = post( g_sync_h );
	if( resp )
	{
		resp();
	}
}

///
/// 同步执行数据库操作，返回结果集
/// [8/13/2014] create by albert.xu
///
sql_result SyncDBExecuteRc( xgc_lpcstr lpSql, sql_recordset &rs )
{
	return execute_sql_rc( g_sync_h, lpSql, rs );
}

xgc_uint32 SyncDBErrorCode()
{
	return get_error_code( g_sync_h );
}

xgc_lpcstr SyncDBErrorInfo()
{
	return get_error_info( g_sync_h );
}

///
/// 获取影响的记录条数
/// [1/5/2015] create by albert.xu
///
xgc_size SyncDBAffectRow()
{
	return affect_rows( g_sync_h );
}

///
/// 投递异步数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_void AsyncDBExecutePost( const postcall &post)
{
	async_post( g_async_h, post );
}

xgc_size AsyncDBExecuteResp( xgc_size exec )
{
	xgc_size count = 0;
	FUNCTION_BEGIN;
	while( exec-- )
	{
		respcall resp = async_resp( g_async_h );
		if( resp == xgc_nullptr )
			break;

		++count;
		resp();
	}

	FUNCTION_END;
	return count;
}

xgc_size SyncDBEscapeString( xgc_lpcstr lpSrc, xgc_int32 nSrcSize, xgc_lpstr lpDst, xgc_size nDstSize )
{
	return escape_string( g_sync_h, lpSrc, nSrcSize, lpDst, nDstSize );
}
