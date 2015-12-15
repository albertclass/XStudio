
#include "stdafx.h"
#include "TableMaker.h"
#include "markupstl.h"

#include <unordered_set>
#include <list>
#include <string>

namespace XGC
{
	namespace DB
	{
		using std::unordered_set;
		using std::list;
		using std::string;

		typedef xgc_lpstr ( *sql_format )( xgc_lpcstr name, xgc_lpcstr sql, xgc_lpcstr params );

		//-----------------------------------//
		// [3/6/2014 albert.xu]
		// 生成表格
		//-----------------------------------//
		xgc_bool ExecuteSql( DB::sql_connection conn, HMODULE mod, CMarkupSTL& xml )
		{
			sql_format rule    = (sql_format) GetProcAddress( mod, xml.GetAttrib( "rule" ).c_str() );
			xgc_string name    = xml.GetAttrib( "name" );
			xgc_string params  = xml.GetAttrib( "params" );
			xgc_lpcstr next    = xgc_nullptr;

			xgc_uint32 version = 1;
			xgc_uint32 version_sql = 0;
			GetXMLAttribInteger( xml, "version", version );

			sql_result res = sql_failed;
			sql_recordset rs = xgc_nullptr;

			xgc_string chk = "select max(version) as version from tables where name='" + name + "';";
			res = execute_sql_rc( conn, chk.c_str(), rs );

			if( res == sql_failed )
			{
				SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
				return false;
			}

			if( res == sql_success )
			{
				movefirst( rs );
				version_sql = field_unsigned( rs, 0, 0 );
			}

			if( version > version_sql )
			{
				SYS_INFO( "数据库表发现更新版本[%010u ==> %010u], %s", version_sql, version, name.c_str() );

				xgc_string sql = xml.GetData();
				xgc_lpstr  exe = xgc_nullptr;
				if( rule )
				{
					// 查看每个表中修改脚本是否执行过
					exe = rule( name.c_str(), sql.c_str(), params.c_str() );
				}

				if( false == execute_sql( conn, exe ? exe : sql.c_str() ) )
				{
					SYS_INFO( "%d - %s: %s", get_error_code( conn ), get_error_info( conn ), sql.c_str() );
					free( exe );
					return false;
				}

				xgc_lpstr esc = (xgc_lpstr) malloc( sql.length() * 2 + 2 );
				escape_string( conn, sql.c_str(), sql.length(), esc, sql.length() * 2 + 2 );
				std::stringstream ver;
				ver << "insert tables( `name`, `version`, `sql`, `create_time` ) values( \n"
					<< "'" << name << "',\n"
					<< version << ",\n"
					<< "'" << esc << "', now() );"
					;

				if( false == execute_sql( conn, ver.str().c_str() ) )
				{
					SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
				}

				free( esc );
				free( exe );
			}
			return true;
		}

		//-----------------------------------//
		// [3/6/2014 albert.xu]
		// 根据配置生成数据库表
		//-----------------------------------//
		xgc_bool MakeTables( XGC::DB::sql_connection conn, xgc_lpcstr fconfig )
		{
			sql_recordset rs = 0;
			sql_result res = execute_sql_rc( conn, "show tables like 'tables';", rs );
			if( res == sql_failed )
			{
				SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
				release( rs );
				rs = 0;
				return false;
			}
			else if( sql_empty == res )
			{
				xgc_bool ret = execute_sql( conn, "create table `tables` ("
					"`name` char(64) not null default '' comment '表名',"
					"`version` int(11) unsigned not null comment '版本号',"
					"`sql` text comment '此次版本的数据库操作语句',"
					"`create_time` datetime comment '记录创建时间',"
					"primary key (`name`,`version`)"
					") engine=innodb comment='表格';" );
				if( ret == false )
				{
					SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
					release( rs );
					rs = 0;
					return false;
				}
			}
			release( rs );
			rs = 0;

			CMarkupSTL doc;
			if( false == doc.Load( fconfig ) )
			{
				SYS_ERROR( "read table config file %s field.%s\n", fconfig, doc.GetError().c_str() );
				return false;
			}

			HMODULE mod = LoadLibrary( xgc_nullptr );

			while( doc.FindElem( "database" ) )
			{
				string schemas = doc.GetAttrib( "name" );
				SYS_INFO( "process database %s", schemas.c_str() );

				if( false == doc.IntoElem() )
				{
					puts( "into database error." );
					continue;
				}

				while( doc.FindElem( "execute" ) )
				{
					if( false == ExecuteSql( conn, mod, doc ) )
					{
						printf( "generate table failed.\n" );
						return false;
					}
				}

			}

			return true;
		}
	}
}

extern "C"
{
	xgc_lpstr hash_namerule( xgc_lpcstr name, xgc_lpcstr sql, xgc_lpcstr params )
	{
		xgc_lpstr ret = (xgc_lpstr) malloc( 4096 );
		xgc_size  cur = 0;

		// 使用后缀，用','分割符。
		xgc_char *token = xgc_nullptr;
		xgc_char *next = xgc_nullptr;

		xgc_lpstr dup = strdup( params );
		token = strtok_s( dup, ", ", &next );

		while( token )
		{
			xgc_string tmp = sql;
			xgc_string::size_type pos = 0;
			while( ( pos = tmp.find( name, pos ) ) != xgc_string::npos )
			{
				pos += strlen( name );
				tmp.insert( pos, token );
			}

			if( _msize( ret ) - cur < tmp.length() + 1 )
			{
				xgc_lpstr nnn = (xgc_lpstr) realloc( ret, _msize( ret ) + 4096 );
				if( nnn == xgc_nullptr )
				{
					free( dup );
					free( ret );
					return false;
				}

				ret = nnn;
			}

			strcpy_s( ret + cur, _msize( ret ) - cur, tmp.c_str() );
			cur += tmp.length();

			token = strtok_s( xgc_nullptr, ", ", &next );
		}

		free( dup );
		return ret;
	}
}