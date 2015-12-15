#include "stdafx.h"
#include "Database.h"
#include "db_mysql.h"
namespace XGC
{
	namespace DB
	{
		//-----------------------------------//
		// [1/17/2014 albert.xu]
		// 初始化数据库模块
		//-----------------------------------//
		xgc_bool _init_library()
		{
			return true;
		}

		//-----------------------------------//
		// [1/17/2014 albert.xu]
		// 清理数据库模块
		//-----------------------------------//
		xgc_void _fini_library()
		{
		}

		// 日志的回调函数指针
		xgc_int32 ( *writelog )( xgc_lpcstr fmt, ... ) = printf;

		//-----------------------------------//
		// [1/17/2014 albert.xu]
		// 设置日志回调接口
		//-----------------------------------//
		xgc_void set_logger( xgc_int32 ( *pfn_logger )( xgc_lpcstr fmt, ... ) )
		{
			writelog = pfn_logger;
		}

		xgc_uint32 get_error_code( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				return pConn->GetLastError();
			}

			return 0;
		}

		xgc_lpcstr get_error_info( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				return pConn->GetLastErrorInfo();
			}

			return "";
		}

		sql_connection connect( connection cfg, xgc_bool immediately )
		{
			CMySQLConnection* pConn = XGC_NEW CMySQLConnection();
			if( pConn )
			{
				pConn->SetConnectionString( cfg );
				if( immediately )
					pConn->Connect();
				
				return pConn;
			}

			return xgc_nullptr;
		}

		xgc_bool reconnect( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;

			return pConn && pConn->ResetConnection();
		}

		xgc_void disconnect( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				pConn->Close();
				SAFE_DELETE( pConn );
			}
		}

		xgc_bool is_connected( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;
			if( !pConn )
				return false;
			
			return pConn->IsConnected();
		}

		xgc_bool execute_sql( sql_connection conn, xgc_lpcstr cmd )
		{
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				return pConn->Execute( cmd );
			}

			return false;
		}

		sql_result execute_sql_rc( sql_connection conn, xgc_lpcstr cmd, sql_recordset &rs )
		{
			if( conn )
			{
				if( rs == xgc_nullptr )
				{
					rs.reset( new CMySQLRecordSet() );
				}

				return conn->Execute( cmd, *rs );
			}

			return sql_failed;
		}

		xgc_size affect_rows( sql_connection conn )
		{
			if( conn )
			{
				return conn->GetAffectedRows();
			}

			return 0;
		}

		xgc_bool next_sql( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				return pConn->MoreResult();
			}

			return false;
		}

		sql_result next_sql_rc( sql_connection conn, sql_recordset &rs )
		{
			XGC_ASSERT_RETURN( rs, sql_failed );
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				return pConn->MoreResult( *rs );
			}

			rs.reset( xgc_nullptr );
			return sql_failed;
		}

		xgc_bool connected( sql_connection conn )
		{
			CMySQLConnection* pConn = conn;
			if( pConn )
			{
				return pConn->IsConnected();
			}

			return false;
		}

		xgc_size escape_string( sql_connection conn, xgc_lpcstr src, xgc_size src_size, xgc_lpstr dst, xgc_size dst_size )
		{
			XGC_ASSERT_RETURN( conn, 0, "%s", "无效的连接句柄。" );
			XGC_ASSERT_RETURN( dst_size >= src_size * 2 + 1, 0, "%s", "存储空间不足" );

			return conn->EscapeString( src, src_size, dst );
		}

		//////////////////////////////////////////////////////////////////////////
		// recordset_t
		//////////////////////////////////////////////////////////////////////////
		xgc_void release( sql_recordset &rs )
		{
			rs.reset( xgc_nullptr );
		}

		xgc_bool movefirst( sql_recordset &rs )
		{
			if( rs )
			{
				return rs->MoveFirst();
			}

			return false;
		}

		xgc_bool movenext( sql_recordset &rs )
		{
			if( rs )
			{
				return rs->MoveNext();
			}

			return false;
		}

		xgc_bool movelast( sql_recordset &rs )
		{
			if( rs )
			{
				return rs->MoveLast();
			}

			return false;
		}

		xgc_bool eof( sql_recordset &rs )
		{
			if( rs )
			{
				return rs->IsEOF();
			}

			return true;
		}

		xgc_uint64 recordcount( sql_recordset &rs )
		{
			if( rs )
			{
				return rs->GetRecordCount();
			}

			return 0;
		}

		xgc_uint32 fieldcount( sql_recordset &rs )
		{
			if( rs )
			{
				return rs->GetFieldCount();
			}

			return 0;
		}

		xgc_size field_length( sql_recordset &rs, xgc_lpcstr name )
		{
			if( rs )
			{
				return rs->GetFieldLength( rs->GetFieldByName( name ) );
			}

			return 0;
		}

		xgc_size field_length( sql_recordset &rs, xgc_uint32 index )
		{
			if( rs )
			{
				return rs->GetFieldLength( index );
			}

			return 0;
		}

		xgc_uint16 fieldtype( sql_recordset &rs, xgc_lpcstr name )
		{
			if( rs )
			{
				return rs->GetFieldType( rs->GetFieldByName( name ) );
			}

			return -1;
		}

		xgc_uint16 fieldtype( sql_recordset &rs, xgc_uint32 index )
		{
			if( rs )
			{
				return rs->GetFieldType( index );
			}

			return -1;
		}

		xgc_uint32 fieldindex( sql_recordset &rs, xgc_lpcstr name )
		{
			if( rs )
			{
				return rs->GetFieldByName( name );
			}

			return -1;
		}

		xgc_uint32 fieldflags( sql_recordset &rs, xgc_uint32 nIndex, xgc_uint32 test )
		{
			if( rs )
			{
				return rs->TestFieldFlags( nIndex, test );
			}

			return 0;
		}

		xgc_uint32	fieldflags( sql_recordset &rs, xgc_lpcstr name, xgc_uint32 test )
		{
			return fieldflags( rs, fieldindex( rs, name ), test );
		}

		xgc_lpcstr field_string( sql_recordset &rs, xgc_uint32 index )
		{
			if( rs )
			{
				return rs->GetFieldValue( index );
			}

			return xgc_nullptr;
		}

		xgc_long field_integer( sql_recordset &rs, xgc_uint32 index, xgc_long _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldflags( rs, index, UNSIGNED_FLAG ) != UNSIGNED_FLAG, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str == xgc_nullptr )
				return _default;

			switch( fieldtype(rs, index) )
			{
				case FIELD_TYPE_TINY:
				case FIELD_TYPE_SHORT:
				case FIELD_TYPE_LONG:
				case FIELD_TYPE_INT24:
				case FIELD_TYPE_LONGLONG:
					return std::strtol( str, (xgc_char**)&str, 10 );
			}

			return _default;
		}

		xgc_ulong	field_unsigned( sql_recordset &rs, xgc_uint32 index, xgc_ulong _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldflags( rs, index, UNSIGNED_FLAG ) == UNSIGNED_FLAG, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str == xgc_nullptr )
				return _default;

			switch( fieldtype( rs, index ) )
			{
				case FIELD_TYPE_TINY:
				case FIELD_TYPE_SHORT:
				case FIELD_TYPE_INT24:
				case FIELD_TYPE_LONG:
				case FIELD_TYPE_LONGLONG:
					return std::strtoul( str, (xgc_char**)&str, 10 );
			}

			return _default;
		}

		xgc_int64 field_longlong( sql_recordset &rs, xgc_uint32 index, xgc_int64 _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldflags( rs, index, UNSIGNED_FLAG ) != UNSIGNED_FLAG, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str == xgc_nullptr )
				return _default;

			XGC_ASSERT_RETURN( fieldtype( rs, index ) == FIELD_TYPE_LONGLONG, _default );
			return std::strtoll( str, (xgc_char**)&str, 10 );
		}

		xgc_uint64 field_ulonglong( sql_recordset &rs, xgc_uint32 index, xgc_uint64 _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldflags( rs, index, UNSIGNED_FLAG ) == UNSIGNED_FLAG, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str == xgc_nullptr )
				return _default;

			XGC_ASSERT_RETURN( fieldtype( rs, index ) == FIELD_TYPE_LONGLONG, _default );
			return std::strtoull( str, (xgc_char**)&str, 10 );
		}

		xgc_bool field_bool( sql_recordset &rs, xgc_uint32 index, xgc_bool _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldtype( rs, index ) == FIELD_TYPE_BIT, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str )
			{
				return str[0] == '1';
			}

			return _default;
		}

		xgc_real32 field_float( sql_recordset &rs, xgc_uint32 index, xgc_real32 _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldtype( rs, index ) == FIELD_TYPE_FLOAT, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str )
				return (xgc_real32)atof( str );

			return _default;
		}

		xgc_real64 field_double( sql_recordset &rs, xgc_uint32 index, xgc_real64 _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			XGC_ASSERT_RETURN( fieldtype( rs, index ) == FIELD_TYPE_DOUBLE, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str )
				return atof( str );

			return _default;
		}

		xgc_real64 field_decimal( sql_recordset &rs, xgc_uint32 index, xgc_real64 _default )
		{
			XGC_ASSERT_RETURN( index != -1, _default );
			xgc_uint16 type = fieldtype( rs, index );
			XGC_ASSERT_RETURN( type == FIELD_TYPE_DECIMAL || type == FIELD_TYPE_NEWDECIMAL, _default );

			xgc_lpcstr str = field_string( rs, index );
			if( str )
				return atof( str );

			return _default;
		}

		xgc_lpvoid field_binary( sql_recordset &rs, xgc_uint32 index, xgc_lpvoid data, xgc_size length )
		{
			XGC_ASSERT_RETURN( index != -1, xgc_nullptr );
			xgc_uint16	type = fieldtype( rs, index );
			xgc_lpcstr	blob = field_string( rs, index );

			xgc_size blob_length = field_length( rs, index );
			switch( type )
			{
				case FIELD_TYPE_BLOB:
				case FIELD_TYPE_TINY_BLOB:
				case FIELD_TYPE_MEDIUM_BLOB:
				case FIELD_TYPE_LONG_BLOB:
					if( data && length )
						memcpy_s( data, length, blob, blob_length );

					return (xgc_lpvoid)blob;
				default:
					XGC_ASSERT_RETURN( false, xgc_nullptr );
					break;
			}

			return xgc_nullptr;
		}

		xgc_lpcstr field_timestamp( sql_recordset &rs, xgc_uint32 index )
		{
			// 2014-12-15 17:40:53
			XGC_ASSERT_RETURN( index != -1, xgc_nullptr );
			XGC_ASSERT_RETURN( fieldtype( rs, index ) == FIELD_TYPE_TIMESTAMP, xgc_nullptr );

			xgc_lpcstr str = field_string( rs, index );
			if( str )
				return str;
			return xgc_nullptr;
		}
		xgc_time64 field_unix_timestamp( sql_recordset &rs, xgc_uint32 index, xgc_time64 _default )
		{
			// 1418636453
			return field_longlong( rs, index, _default );
		}
	}
}
