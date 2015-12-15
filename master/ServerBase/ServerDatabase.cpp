#include "stdafx.h"
#include "TableMaker.h"
#include "ServerDatabase.h"
#include "ServerLogger.h"

using namespace XGC;
using namespace XGC::DB;

/// @var ���ݿ��첽����
static sql_database		g_async_h = 0;
static sql_connection	g_sync_h = 0;

///
/// ���ݿ���־
/// [8/13/2014] create by albert.xu
///
static xgc_int32 DBLogger( xgc_lpcstr fmt, ... )
{
	FUNCTION_BEGIN;
	xgc_char szLog[1024 * 16];
	va_list aplist;

	va_start( aplist, fmt );
	xgc_int32 write = vsprintf_s( szLog, fmt, aplist );
	va_end( aplist );

	WriteServerLog( szLog );

	return write;
	FUNCTION_END;
	return 0;
}

xgc_bool InitServerDatabase( IniFile &ini )
{
	FUNCTION_BEGIN;

	connection conn;
	// ���ݿ�����
	conn.host      = ini.GetItemValue( "Database", "Host", xgc_nullptr );
	if( xgc_nullptr == conn.host )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Host û����ȷ���á�" );
		return false;
	}
	conn.port      = ini.GetItemValue( "Database", "Port", 3306 );
	conn.retry     = ini.GetItemValue( "Database", "Retry", 3 );
	conn.username  = ini.GetItemValue( "Database", "Username", xgc_nullptr );
	if( xgc_nullptr == conn.username )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Username û����ȷ���á�" );
		return false;
	}
	conn.password = ini.GetItemValue( "Database", "Password", xgc_nullptr );
	if( xgc_nullptr == conn.password )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Password û����ȷ���á�" );
		return false;
	}
	conn.schema = ini.GetItemValue( "Database", "Database", xgc_nullptr );
	if( xgc_nullptr == conn.schema )
	{
		SYS_ERROR( "%s", "���ݿ������� Database.Database û����ȷ���á�" );
		return false;
	}
	conn.character = ini.GetItemValue( "Database", "Character", "latin1" );

	xgc_lpcstr lpParentNode = "database";
	MemMark( "init", lpParentNode );
	if( false == _init_library() )
	{
		SYS_ERROR( "���ݿ�ײ���ʼ��ʧ�ܡ�" );
		return false;
	}
	SYS_INFO( "���ݿ�ײ���ʼ���ɹ���" );

	MemMark( "async connect", lpParentNode );
	set_logger( DBLogger );
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

	xgc_lpcstr lpConf = ini.GetItemValue( "Database", "TableMaker", xgc_nullptr );
	if( lpConf )
	{
		xgc_char szTableConf[_MAX_PATH];
		GetNormalPath( szTableConf, sizeof( szTableConf ), "%s", lpConf );
		if( false == MakeTables( g_sync_h, szTableConf ) )
			return false;
	}

	//if( ini.IsExistSection( "UUID" ) )
	//{
	//	xgc_lpcstr lpTableName	= ini.GetItemValue( "UUID", "TableName", xgc_nullptr );
	//	if( xgc_nullptr == lpTableName )
	//	{
	//		SYS_ERROR( "UUID��TableNameδ���á��޷�ȷ�����ݱ����ơ�" );
	//		return false;
	//	}
	//	xgc_uint32 nOnceAlloc	= ini.GetItemValue( "UUID", "OnceAlloc", 100U );

	//	xgc_char szSql[1024];
	//	sprintf_s( szSql, "create table `%s` ("
	//		"`type` tinyint( 3 ) unsigned not null default '0',"
	//		"`usid` bigint( 20 ) unsigned not null default '0',"
	//		"primary key( `id`)"
	//		"); ", lpTableName );
	//}
	return true;
	FUNCTION_END;

	return false;
}

xgc_void FiniServerDatabase()
{
	FUNCTION_BEGIN;
	disconnect( g_sync_h );
	async_close( g_async_h );
	_fini_library();
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
xgc_void SyncDBExecute( const std::function< respcall( sql_connection ) > &fnPost )
{
	auto resp = fnPost( g_sync_h );
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
xgc_void AsyncDBExecutePost( const DB::postcall &post)
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
