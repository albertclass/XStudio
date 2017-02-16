// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DATABASE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#pragma once
#ifndef __DATABASE_H
#define __DATABASE_H

#include "config.h"
#include <functional>
#include <memory>

namespace xgc
{
	namespace sql
	{
		using std::function;

		extern const xgc_uint16 DB_FIELD_TYPE_DECIMAL;
		extern const xgc_uint16 DB_FIELD_TYPE_NEWDECIMAL;
		extern const xgc_uint16 DB_FIELD_TYPE_TINY;
		extern const xgc_uint16 DB_FIELD_TYPE_SHORT;
		extern const xgc_uint16 DB_FIELD_TYPE_LONG;
		extern const xgc_uint16 DB_FIELD_TYPE_FLOAT;
		extern const xgc_uint16 DB_FIELD_TYPE_DOUBLE;
		extern const xgc_uint16 DB_FIELD_TYPE_NULL;
		extern const xgc_uint16 DB_FIELD_TYPE_TIMESTAMP;
		extern const xgc_uint16 DB_FIELD_TYPE_LONGLONG;
		extern const xgc_uint16 DB_FIELD_TYPE_INT24;
		extern const xgc_uint16 DB_FIELD_TYPE_DATE;
		extern const xgc_uint16 DB_FIELD_TYPE_TIME;
		extern const xgc_uint16 DB_FIELD_TYPE_DATETIME;
		extern const xgc_uint16 DB_FIELD_TYPE_YEAR;
		extern const xgc_uint16 DB_FIELD_TYPE_NEWDATE;
		extern const xgc_uint16 DB_FIELD_TYPE_ENUM;
		extern const xgc_uint16 DB_FIELD_TYPE_SET;
		extern const xgc_uint16 DB_FIELD_TYPE_TINY_BLOB;
		extern const xgc_uint16 DB_FIELD_TYPE_MEDIUM_BLOB;
		extern const xgc_uint16 DB_FIELD_TYPE_LONG_BLOB;
		extern const xgc_uint16 DB_FIELD_TYPE_BLOB;
		extern const xgc_uint16 DB_FIELD_TYPE_VAR_STRING;
		extern const xgc_uint16 DB_FIELD_TYPE_STRING;
		extern const xgc_uint16 DB_FIELD_TYPE_CHAR;
		extern const xgc_uint16 DB_FIELD_TYPE_INTERVAL;
		extern const xgc_uint16 DB_FIELD_TYPE_GEOMETRY;
		extern const xgc_uint16 DB_FIELD_TYPE_BIT;

		extern const xgc_uint32 DB_FIELD_NOT_NULL_FLAG;
		extern const xgc_uint32 DB_FIELD_PRI_KEY_FLAG;
		extern const xgc_uint32 DB_FIELD_UNIQUE_KEY_FLAG;
		extern const xgc_uint32 DB_FIELD_MULTIPLE_KEY_FLAG;
		extern const xgc_uint32 DB_FIELD_BLOB_FLAG;
		extern const xgc_uint32 DB_FIELD_UNSIGNED_FLAG;
		extern const xgc_uint32 DB_FIELD_ZEROFILL_FLAG;
		extern const xgc_uint32 DB_FIELD_BINARY_FLAG;

		/* The following are only sent to new clients */
		extern const xgc_uint32 DB_FIELD_ENUM_FLAG;
		extern const xgc_uint32 DB_FIELD_AUTO_INCREMENT_FLAG;
		extern const xgc_uint32 DB_FIELD_TIMESTAMP_FLAG;
		extern const xgc_uint32 DB_FIELD_SET_FLAG;
		extern const xgc_uint32 DB_FIELD_NO_DEFAULT_VALUE_FLAG;
		extern const xgc_uint32 DB_FIELD_ON_UPDATE_NOW_FLAG;
		extern const xgc_uint32 DB_FIELD_NUM_FLAG;
		extern const xgc_uint32 DB_FIELD_PART_KEY_FLAG;
		extern const xgc_uint32 DB_FIELD_GROUP_FLAG;
		extern const xgc_uint32 DB_FIELD_UNIQUE_FLAG;
		extern const xgc_uint32 DB_FIELD_BINCMP_FLAG;


		typedef class CMySQLRecordSet	CMySQLRecordSet;
		typedef class CMySQLConnection	CMySQLConnection;
		typedef class CMySqlDatabase	CMySqlDatabase;

		struct DATABASE_API sql_recordset_deleter
		{
			sql_recordset_deleter() _NOEXCEPT
			{	// default construct
			}

			sql_recordset_deleter( const sql_recordset_deleter& ) _NOEXCEPT
			{	// construct from another default_delete
			}

			void operator()( CMySQLRecordSet *_Ptr ) const _NOEXCEPT;
		};

		typedef std::unique_ptr< CMySQLRecordSet, sql_recordset_deleter > sql_recordset;

		typedef class CMySQLConnection	*sql_connection;
		typedef class CMySqlDatabase	*sql_database;

		typedef function< xgc_void() > respcall;
		typedef function< respcall( sql_connection conn ) > postcall;

		///定义ExecuteEx的返回值
		enum DATABASE_API sql_result
		{
			sql_failed = -1,
			sql_success = 0,
			sql_empty = 1,
		};

		struct DATABASE_API connection_cfg
		{
			xgc_lpcstr host;
			xgc_uint16 port;
			xgc_uint16 retry;
			xgc_lpcstr schema;
			xgc_lpcstr username;
			xgc_lpcstr password;
			xgc_lpcstr character;
		};

		///
		/// [1/17/2014 albert.xu]
		/// 初始化数据库模块
		///
		DATABASE_API xgc_bool sql_init_library();

		///
		/// [1/17/2014 albert.xu]
		/// 清理数据库模块
		///
		DATABASE_API xgc_void sql_fini_library();

		///
		/// [1/17/2014 albert.xu]
		/// 获取数据库连接的最后一次错误码
		///
		DATABASE_API xgc_uint32 get_error_code( sql_connection conn );

		///
		/// [1/17/2014 albert.xu]
		/// 获取数据库连接的最后一次错误信息
		///
		DATABASE_API xgc_lpcstr get_error_info( sql_connection conn );

		///
		/// [1/17/2014 albert.xu]
		/// 同步连接数据库,并取得数据库句柄,失败返回 0
		///
		DATABASE_API sql_connection connect( connection_cfg cfg, xgc_bool immediately = true );

		///
		/// 主动重连数据库
		/// [12/2/2014] create by albert.xu
		///
		DATABASE_API xgc_bool reconnect( sql_connection conn );

		///
		/// [1/17/2014 albert.xu]
		/// 主动断开数据库连接
		///
		DATABASE_API xgc_void disconnect( sql_connection conn );

		///
		/// [1/17/2014 albert.xu]
		/// 执行一个数据库查询
		///
		DATABASE_API xgc_bool execute_sql( sql_connection conn, xgc_lpcstr cmd );

		///
		/// [1/17/2014 albert.xu]
		/// 执行一个数据库查询,并返回结果集
		///
		DATABASE_API sql_result execute_sql_rc( sql_connection conn, xgc_lpcstr cmd, sql_recordset &rs );

		///
		/// [1/17/2014 albert.xu]
		/// 获取影响的记录条数
		///
		DATABASE_API xgc_size affect_rows( sql_connection conn );

		///
		/// [1/17/2014 albert.xu]
		/// 获取下一个结果集,并返回结果集
		///
		DATABASE_API xgc_bool next_sql( sql_connection conn );

		///
		/// [1/17/2014 albert.xu]
		/// 获取下一个结果集,并返回结果集
		///
		DATABASE_API sql_result next_sql_rc( sql_connection conn, sql_recordset &rs );

		///
		/// [1/17/2014 albert.xu]
		/// 查询数据库连接状态
		///
		DATABASE_API xgc_bool connected( sql_connection conn );

		/*!
		 * @brief 转义字符串，使之可用于MYSQL查询
		 * @param conn 数据库连接句柄
		 * @param pszSrc 源数据
		 * @param nSize 源数据大小
		 * @param pszDest 转义后数据
		 * @return 转义后的数据长度
		 * @author guqiwei.weir
		 * @date [6/16/2014]
		 */
		DATABASE_API xgc_size escape_string( sql_connection conn, xgc_lpcstr src, xgc_size src_size, xgc_lpstr dst, xgc_size dst_size );

		template< xgc_size S >
		xgc_size escape_string( sql_connection conn, xgc_lpcstr src, xgc_size src_size, xgc_char( &dst )[S] )
		{
			return escape_string( conn, src, src_size, dst, S );
		}

		template< xgc_size S1, xgc_size S2 >
		xgc_size escape_string( sql_connection conn, const xgc_char( &src )[S1], xgc_char( &dst )[S2] )
		{
			return escape_string( conn, src, S1, dst, S2 );
		}

		template< class T, xgc_size S >
		xgc_size escape_string( sql_connection conn, const T &src, xgc_char( &dst )[S] )
		{
			return escape_string( conn, (xgc_lpcstr) src, sizeof( T ), dst, S );
		}

		//////////////////////////////////////////////////////////////////////////
		// recordset_t
		//////////////////////////////////////////////////////////////////////////
		DATABASE_API xgc_void	release( sql_recordset &rs );
		DATABASE_API xgc_bool	movefirst( sql_recordset &rs );
		DATABASE_API xgc_bool	movenext( sql_recordset &rs );
		DATABASE_API xgc_bool	movelast( sql_recordset &rs );
		DATABASE_API xgc_bool	eof( sql_recordset &rs );
		DATABASE_API xgc_uint64	recordcount( sql_recordset &rs );
		DATABASE_API xgc_uint32	fieldcount( sql_recordset &rs );
		DATABASE_API xgc_uint16	fieldtype( sql_recordset &rs, xgc_uint32 index );
		DATABASE_API xgc_uint16	fieldtype( sql_recordset &rs, xgc_lpcstr name );
		DATABASE_API xgc_uint32	fieldindex( sql_recordset &rs, xgc_lpcstr name );
		DATABASE_API xgc_uint32	fieldflags( sql_recordset &rs, xgc_uint32 index, xgc_uint32 test );
		DATABASE_API xgc_uint32	fieldflags( sql_recordset &rs, xgc_lpcstr name, xgc_uint32 test );

		// 字段长度
		DATABASE_API xgc_size	field_length( sql_recordset &rs, xgc_uint32 index );
		DATABASE_API xgc_size	field_length( sql_recordset &rs, xgc_lpcstr name );

		// 所有字段都可以返回为string
		DATABASE_API xgc_lpcstr	field_string( sql_recordset &rs, xgc_uint32 index );

		///
		/// [1/20/2014 albert.xu]
		/// 数值类型字段读取
		///
		DATABASE_API xgc_long	field_integer( sql_recordset &rs, xgc_uint32 index, xgc_long _default );
		DATABASE_API xgc_ulong	field_unsigned( sql_recordset &rs, xgc_uint32 index, xgc_ulong _default );

		///
		/// [1/20/2014 albert.xu]
		/// 超大数值类型字段读取
		///
		DATABASE_API xgc_int64	field_longlong( sql_recordset &rs, xgc_uint32 index, xgc_int64 _default );
		DATABASE_API xgc_uint64	field_ulonglong( sql_recordset &rs, xgc_uint32 index, xgc_uint64 _default );

		///
		/// [1/20/2014 albert.xu]
		/// bit类型字段读取
		///
		DATABASE_API xgc_bool	field_bool( sql_recordset &rs, xgc_uint32 index, xgc_bool _default );

		///
		/// [1/20/2014 albert.xu]
		/// 浮点类型字段读取
		///
		DATABASE_API xgc_real32	field_float( sql_recordset &rs, xgc_uint32 index, xgc_real32 _default );
		DATABASE_API xgc_real64	field_double( sql_recordset &rs, xgc_uint32 index, xgc_real64 _default );
		DATABASE_API xgc_real64	field_decimal( sql_recordset &rs, xgc_uint32 index, xgc_real64 _default );

		///
		/// [1/20/2014 albert.xu]
		/// 二进制类型字段读取
		///
		DATABASE_API xgc_lpvoid	field_binary( sql_recordset &rs, xgc_uint32 index, xgc_lpvoid data, xgc_size length );

		///
		/// timestamp 类型
		/// [12/17/2014] create by jianglei.kinly
		///
		DATABASE_API xgc_lpcstr field_timestamp( sql_recordset &rs, xgc_uint32 index );

		///
		/// 转了unix_timestamp的类型
		/// [12/17/2014] create by jianglei.kinly
		///
		DATABASE_API xgc_time64 field_unix_timestamp( sql_recordset &rs, xgc_uint32 index, xgc_time64 _default );

		//////////////////////////////////////////////////////////////////////////
		// service
		//////////////////////////////////////////////////////////////////////////
		///
		/// \brief 创建数据库服务
		///
		/// \author albert.xu
		/// \date 2015/12/18 17:31
		///
		DATABASE_API sql_database async_connect( connection_cfg cfg );

		DATABASE_API sql_database sync_connect( connection_cfg cfg );

		///
		/// \brief 获取连接对象
		///
		/// \author albert.xu
		/// \date 2015/12/18 17:34
		///
		DATABASE_API xgc_size async_escape_string( sql_database s, xgc_lpcstr data, xgc_int32 data_size, xgc_lpstr buffer, xgc_size buffer_size );

		///
		/// \brief 销毁数据库服务
		///
		/// \author albert.xu
		/// \date 2015/12/18 17:31
		///
		DATABASE_API xgc_void async_close( sql_database s );

		DATABASE_API xgc_void sync_close( sql_database s );

		///
		/// \brief 投递数据库请求
		///
		/// \author albert.xu
		/// \date 2015/12/18 17:31
		///
		DATABASE_API xgc_void async_post( sql_database s, const postcall &call );

		///
		/// \brief 获取数据库回应
		///
		/// \author albert.xu
		/// \date 2015/12/18 17:31
		///
		DATABASE_API respcall async_resp( sql_database s );

		DATABASE_API xgc_bool sync_exec( sql_database s, xgc_lpcstr sql );
		DATABASE_API sql_result sync_exec_rc( sql_database s, xgc_lpcstr sql, sql_recordset &rs );
		DATABASE_API xgc_void sync_post( sql_database s, postcall && call );

		DATABASE_API xgc_uint32 get_error_code(sql_database s);
		DATABASE_API xgc_lpcstr get_error_info(sql_database s);
	}
}
#endif //__DATABASE_H
