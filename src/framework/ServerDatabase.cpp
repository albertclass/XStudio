#include "ServerDefines.h"
#include "ServerDatabase.h"
#include "ServerBase.h"

using namespace xgc;
using namespace xgc::sql;

/// @var ���ݿ��첽����
static sql_database		g_async_h = 0;
static sql_connection	g_sync_h = 0;

xgc_bool InitServerDatabase( ini_reader &ini )
{
	FUNCTION_BEGIN;

	// û���������ݿ⣬����Ϊ�ɹ�
	if( !ini.is_exist_section( "Database") )
		return true;

	connection_cfg conn;
	// ���ݿ�����
	conn.host      = ini.get_item_value( "Database", "Host", xgc_nullptr );
	if( xgc_nullptr == conn.host )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Host û����ȷ���á�" );
		return false;
	}
	conn.port      = ini.get_item_value( "Database", "Port", 3306 );
	conn.retry     = ini.get_item_value( "Database", "Retry", 3 );
	conn.username  = ini.get_item_value( "Database", "Username", xgc_nullptr );
	if( xgc_nullptr == conn.username )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Username û����ȷ���á�" );
		return false;
	}
	conn.password = ini.get_item_value( "Database", "Password", xgc_nullptr );
	if( xgc_nullptr == conn.password )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Password û����ȷ���á�" );
		return false;
	}
	conn.schema = ini.get_item_value( "Database", "Schema", xgc_nullptr );
	if( xgc_nullptr == conn.schema )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Schema û����ȷ���á�" );
		return false;
	}
	conn.character = ini.get_item_value( "Database", "Character", "latin1" );

	xgc_lpcstr lpParentNode = "database";
	MemMark( "init", lpParentNode );
	if( false == sql_init_library() )
	{
		SYS_ERROR( "���ݿ�ײ���ʼ��ʧ�ܡ�" );
		return false;
	}
	SYS_INFO( "���ݿ�ײ���ʼ���ɹ���" );

	MemMark( "async connect", lpParentNode );
	g_async_h = async_connect( conn );
	if( 0 == g_async_h )
	{
		SYS_ERROR( "���ݿ��첽���ӳ�ʼ��ʧ�ܡ�" );
		return false;
	}

	MemMark( "sync connect", lpParentNode );
	g_sync_h = connect( conn );
	if( 0 == g_sync_h )
	{
		SYS_ERROR( "ͬ�����ݿ�����ʧ��! %d - %s", get_error_code( g_sync_h ), get_error_info( g_sync_h ) );
		return false;
	}
	SYS_INFO( "���ݿ�ͬ�����ӳ�ʼ���ɹ���" );

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
/// ͬ��ִ�����ݿ����
/// [8/13/2014] create by albert.xu
///
xgc_bool SyncDBExecute( xgc_lpcstr lpSql )
{
	return execute_sql( g_sync_h, lpSql );
}

///
/// ͬ��ִ�����ݿ����
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
/// ͬ��ִ�����ݿ���������ؽ����
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
/// ��ȡӰ��ļ�¼����
/// [1/5/2015] create by albert.xu
///
xgc_size SyncDBAffectRow()
{
	return affect_rows( g_sync_h );
}

///
/// Ͷ���첽���ݿ����
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
