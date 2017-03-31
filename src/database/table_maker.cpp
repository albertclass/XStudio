#include "database.h"
#include "xsystem.h"
#include "pugixml.hpp"

#include <list>
#include <string>
#include <sstream>
#include <unordered_set>

#ifdef _LINUX
#	include<dlfcn.h>
#endif
using namespace xgc::sql;

typedef xgc_lpstr( *sql_format )(xgc_lpcstr name, xgc_lpcstr sql, xgc_lpcstr params);

/// ��ǰ�Ļ�����
static xgc_lpstr _environment = xgc_nullptr;
/// ��������
static xgc_lpstr _environment_variables[32][2] = { 0 };
/// ����������
static xgc_ulong _environment_variable_count = 0;

/// ����������ֵ
#define variable_key 0
#define variable_val 1

///
/// \brief ������������
///
/// \author albert.xu
/// \date 2015/12/21 18:29
///
static xgc_void parse_environment( xgc_lpcstr environment )
{
	_environment = strdup( environment );

	xgc_lpcstr environment_delimiter = ";";
	xgc_lpstr environment_token = strtok( _environment, environment_delimiter );
	while( environment_token )
	{
		xgc_lpcstr variable_delimiter = " =\t\r\n";

		_environment_variables[_environment_variable_count][variable_key] = strtok( environment_token, variable_delimiter );
		_environment_variables[_environment_variable_count][variable_val] = strtok( xgc_nullptr, variable_delimiter );

		if( ++_environment_variable_count > XGC_COUNTOF( _environment_variables ) )
			break;

		environment_token = strtok( xgc_nullptr, environment_delimiter );
	}
}

///
/// \brief �ͷŻ�������
///
/// \author albert.xu
/// \date 2015/12/22 14:29
///
static xgc_void erase_environment()
{
	_environment_variable_count = 0;
	memset( _environment_variables, 0, sizeof( _environment_variables ) );
	free( _environment );
	_environment = xgc_nullptr;
}

///
/// \brief �滻һ����������
///
/// \author albert.xu
/// \date 2015/12/21 19:45
///
static xgc_lpstr replace_environment_variable( xgc_lpstr buffer, xgc_lpcstr search, xgc_lpcstr replace )
{
	xgc_size size = memsize( buffer );

	xgc_size len_b = strlen( buffer );
	xgc_size len_s = strlen( search );
	xgc_size len_r = strlen( replace );

	xgc_lpstr str = strstr( buffer, search );

	while( str )
	{
		void* new_buffer = buffer;

		// ���������㣬�����·��仺����
		while( len_b - len_s + len_r > size )
		{
			new_buffer = realloc( buffer, size + 1024 );
			if( new_buffer == xgc_nullptr )
			{
				free( buffer );
				return xgc_nullptr;
			}
			// ��ʼ���·�����ڴ�
			memset( (xgc_lpstr)new_buffer + size, 0, 1024 );
			// ���¼����ѯָ��
			str = (xgc_lpstr) new_buffer + (str - buffer);
			// ���»��帳��buffer
			buffer = (xgc_lpstr)new_buffer;

			size = memsize( buffer );
		}

		// ִ���滻����
		xgc_size size_1 = (buffer + size) - (str + len_r);
		xgc_size size_2 = ( buffer + len_b ) - ( str + len_s );
		memmove( str + len_r, str + len_s, XGC_MIN( size_1, size_2 ) );
		xgc_size size_0 = buffer + size - str;
		memcpy( str, replace, XGC_MIN( size_0, len_r) );

		// ������һ��
		str = strstr( str + len_r, search );
	}

	return buffer;
}

static xgc_lpstr replace_environment( xgc_lpcstr sql )
{
	// ���䷵�صĻ�����
	xgc_lpstr buf = strdup( sql );

	// �����滻��������
	for( xgc_ulong i = 0; i < _environment_variable_count; ++i )
	{
		buf = replace_environment_variable( buf, _environment_variables[i][variable_key], _environment_variables[i][variable_val] );
		if( xgc_nullptr == buf )
			return xgc_nullptr;
	}

	return buf;
}

xgc_lpstr get_environment( xgc_lpcstr key )
{
	for( xgc_ulong i = 0; i < _environment_variable_count; ++i )
	{
		if( strcasecmp( _environment_variables[i][variable_key], key ) == 0 )
			return _environment_variables[i][variable_val];
	}

	return xgc_nullptr;
}

xgc_bool execute_schemas( sql_connection conn, pugi::xml_node& node, xgc_lpcstr environment )
{
	// �������ݿ����������ݿ�
	xgc_lpcstr schema = node.attribute( "name" ).as_string( xgc_nullptr );

	if( xgc_nullptr == schema )
	{
		schema = get_environment( "$(schema)" );
		XGC_ASSERT_RETURN( schema, false, "���ݿ�ڵ�ͻ���������δָ�����ݿ�����%s", node.path().c_str() );
	}

	// ���ݿⲻ�����򴴽����ݿ�
	xgc_char sql[128] = { 0 };

	sprintf_s( sql, "create database if not exists %s;", schema );
	if( false == execute_sql( conn, sql ) )
	{
		SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
		return false;
	}

	sprintf_s( sql, "use %s;", schema );
	if( false == execute_sql( conn, sql ) )
	{
		SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
		return false;
	}

	// �����ݿ��д������¼�¼��
	sql_recordset rs = xgc_nullptr;
	sql_result res = execute_sql_rc( conn, "show tables like '__tables';", rs );
	if( sql_empty == res )
	{
		xgc_lpcstr sql_create_tables = "create table `__tables` ("
			"	`name` char(64) not null default '' comment '����',"
			"	`version` int(11) unsigned not null comment '�汾��',"
			"	`sql` text comment '�˴ΰ汾�����ݿ�������',"
			"	`create_time` datetime comment '��¼����ʱ��',"
			"	primary key (`name`,`version`)"
			") engine=innodb comment='���';";

		if( false == execute_sql( conn, sql_create_tables ) )
		{
			SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
			return false;
		}
	}
	else if( res == sql_failed )
	{
		SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
		return false;
	}

	return true;
}

///
/// \brief ���ɱ��
///
/// \author albert.xu
/// \date 2015/12/21 17:20
///
xgc_bool execute_tables( sql_connection conn, pugi::xml_node& node, xgc_lpcstr environment )
{
	#ifdef _WINDOWS
	HMODULE _M = LoadLibraryA( xgc_nullptr );

	sql_format rule = (sql_format) GetProcAddress( _M, node.attribute( "rule" ).as_string() );
	#endif

	#ifdef _LINUX
	xgc_lpvoid _M = dlopen( xgc_nullptr, RTLD_LAZY );

	sql_format rule = (sql_format) dlsym( _M, node.attribute( "rule" ).as_string() );
	#endif
	xgc_string name = node.attribute( "name" ).as_string();
	xgc_string params = node.attribute( "params" ).as_string();
	xgc_lpcstr next = xgc_nullptr;

	xgc_uint32 version = node.attribute( "version" ).as_uint( 1 );
	xgc_uint32 version_sql = 0;

	sql_result res = sql_failed;
	sql_recordset rs = xgc_nullptr;

	xgc_string chk = "select max(version) as version from __tables where name='" + name + "';";
	res = execute_sql_rc( conn, chk.c_str(), rs );

	if( res == sql_failed )
	{
		SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
		#ifdef _WINDOWS
		FreeLibrary( _M );
		#endif

		#ifdef _LINUX
		dlclose(_M);
		#endif
		return false;
	}

	if( res == sql_success )
	{
		movefirst( rs );
		version_sql = field_unsigned( rs, 0, 0 );
	}

	if( version > version_sql )
	{
		SYS_INFO( "���ݿ���ָ��°汾[%010u ==> %010u], %s", version_sql, version, name.c_str() );

		xgc_lpstr sql = replace_environment( node.text().get() );
		XGC_ASSERT_RETURN( sql, false );
		if( rule )
		{
			xgc_lpstr src = sql;
			// �鿴ÿ�������޸Ľű��Ƿ�ִ�й�
			sql = rule( name.c_str(), src, params.c_str() );
			free( src );
		}

		if( false == execute_sql( conn, sql ) )
		{
			SYS_INFO( "%d - %s: %s", get_error_code( conn ), get_error_info( conn ), sql );
		}
		else
		{
			xgc_lpstr esc = (xgc_lpstr) malloc( strlen( sql ) * 2 + 2 );
			escape_string( conn, sql, strlen( sql ), esc, memsize( (void*) esc ) );
			std::stringstream ver;
			ver << "insert __tables( `name`, `version`, `sql`, `create_time` ) values( \n"
				<< "'" << name << "',\n"
				<< version << ",\n"
				<< "'" << esc << "', now() );"
				;

			if( false == execute_sql( conn, ver.str().c_str() ) )
			{
				SYS_INFO( "%d - %s", get_error_code( conn ), get_error_info( conn ) );
			}

			free( esc );
		}

		free( sql );
	}

	#ifdef _WINDOWS
	FreeLibrary( _M );
	#endif

	#ifdef _LINUX
	dlclose(_M);
	#endif

	return true;
}

xgc_bool make_tables( connection_cfg cfg, xgc_lpcstr fconfig, xgc_lpcstr environment )
{
	char fullpath[XGC_MAX_PATH] = { 0 };
	xgc::get_absolute_path( fullpath, fconfig );

	// ��ȡ�������
	pugi::xml_document doc;
	pugi::xml_parse_result resload = doc.load_file( fullpath );
	if( !resload )
	{
		SYS_ERROR( "read table config file %s \n", fullpath );
		return false;
	}

	cfg.schema = "";
	sql_connection conn = connect( cfg );
	if( xgc_nullptr == conn )
	{
		SYS_ERROR( "connect database server failed. host = %s:%u schema = %s", cfg.host, cfg.port, cfg.schema );
		return false;
	}

	// ������������
	if( environment )
		parse_environment( environment );

	xgc_bool ret = true;
	for( auto node = doc.child( "schema" ); node; node = node.next_sibling( "schema" ) )
	{
		if( false == execute_schemas( conn, node, environment ) )
			continue;

		for( auto node2 = node.child( "execute" ); node2; node2 = node2.next_sibling( "execute" ) )
		{
			if( false == execute_tables( conn, node2, environment ) )
			{
				SYS_ERROR( "generate table failed. path = %s\n", node2.path().c_str() );
				ret = true;
				break;
			}
		}

		if( false == ret )
			break;
	}

	// ����������
	erase_environment();

	// �Ͽ����ݿ�����
	disconnect( conn );
	return ret;
}

extern "C"
{
	xgc_lpstr DATABASE_API hash_namerule( xgc_lpcstr name, xgc_lpcstr sql, xgc_lpcstr params )
	{
		xgc_lpstr ret = (xgc_lpstr) malloc( 4096 );
		xgc_size  cur = 0;

		// ʹ�ú�׺����','�ָ����
		xgc_lpcstr delim = ", ";
		xgc_char *token = xgc_nullptr;

		xgc_lpstr dup = strdup( params );
		token = strtok( dup, delim );

		while( token )
		{
			xgc_string tmp = sql;
			xgc_string::size_type pos = 0;
			while( (pos = tmp.find( name, pos )) != xgc_string::npos )
			{
				pos += strlen( name );
				tmp.insert( pos, token );
			}

			if( memsize( ret ) - cur < tmp.length() + 1 )
			{
				xgc_lpstr nnn = (xgc_lpstr) realloc( ret, memsize( ret ) + 4096 );
				if( nnn == xgc_nullptr )
				{
					free( dup );
					free( ret );
					return xgc_nullptr;
				}

				ret = nnn;
			}

			strcpy_s( ret + cur, memsize( ret ) - cur, tmp.c_str() );
			cur += tmp.length();

			token = strtok( xgc_nullptr, delim );
		}

		free( dup );
		return ret;
	}
}