#pragma once
#ifndef _DB_MYSQL_H_
#define _DB_MYSQL_H_

#ifdef _WINDOWS
#	include <WinSock2.h>
#	pragma comment( lib, "ws2_32.lib")
#	include <mysql.h>
#	include <errmsg.h>
#endif

#ifdef _LINUX
#	include <mysql/mysql.h>
#	include <mysql/errmsg.h>
#endif

namespace xgc
{
	namespace sql
	{
		class CMySQLRecordSet
		{
			friend class CMySQLConnection;
		public:
			///构造函数
			CMySQLRecordSet() 
				: m_res( xgc_nullptr )
				, m_row( xgc_nullptr )
			{
			}

			///析构函数
			~CMySQLRecordSet()
			{
				mysql_free_result( m_res );
			}

			///获取记录个数
			xgc_uint64 GetRecordCount() 
			{ 
				return ( m_res ? (xgc_uint64)mysql_num_rows( m_res ) : 0 ); 
			}

			///获取字段个数
			xgc_uint32 GetFieldCount()
			{
				return ( m_res ? (xgc_uint32)mysql_num_fields( m_res ) : 0 ); 
			}

			///移动到记录集顶端
			xgc_bool MoveFirst()
			{
				if( m_res == xgc_nullptr )
					return false;

				mysql_data_seek( m_res, 0 );
				m_row = mysql_fetch_row( m_res );

				return ( m_row != xgc_nullptr );
			}

			///获取下一个记录行
			xgc_bool MoveNext()
			{
				if( m_res == xgc_nullptr )
					return false;
				m_row = mysql_fetch_row( m_res );
				return ( m_row != xgc_nullptr );
			}

			xgc_bool MoveLast()
			{
				if( m_res == xgc_nullptr )
					return false;
				mysql_data_seek( m_res, mysql_num_rows(m_res) - 1 );
				m_row = mysql_fetch_row( m_res );

				return ( m_row != xgc_nullptr );
			}

			///是否记录底端
			xgc_bool IsEOF()
			{
				return mysql_eof( m_res ) != 0;
			}

			xgc_uint32 GetFieldByName( xgc_lpcstr pszName )
			{
				if( m_res == xgc_nullptr )
				{
					return -1;
				}

				if( m_fields == xgc_nullptr )
				{
					m_fields = mysql_fetch_fields( m_res );
				}

				if( m_fields )
				{
					xgc_size name_length = strlen( pszName );
					for( xgc_uint32 i = 0; i < mysql_num_fields( m_res ); ++i )
					{
						MYSQL_FIELD &field = m_fields[i];
						if( name_length == field.name_length && strncmp( field.name, pszName, field.name_length ) == 0 )
							return i;
					}
				}

				return -1;
			}

			xgc_uint16 GetFieldType( xgc_uint32 nIndex )
			{
				if( nIndex < GetFieldCount() )
				{
					MYSQL_FIELD* field = mysql_fetch_field_direct( m_res, nIndex );
					if( field )
					{
						return field->type;
					}
				}

				return -1;
			}

			xgc_uint32 TestFieldFlags( xgc_uint32 nIndex, xgc_uint32 uTest )
			{
				if( nIndex < GetFieldCount() )
				{
					MYSQL_FIELD* field = mysql_fetch_field_direct( m_res, nIndex );
					if( field )
					{
						return field->flags & uTest;
					}
				}

				return 0;
			}

			///获取当前记录行中某一个字段的值
			xgc_lpcstr GetFieldValue( xgc_uint32 nIndex )
			{
				if( m_res == xgc_nullptr )
					return xgc_nullptr;
				if( m_row == xgc_nullptr )
					return xgc_nullptr;
				if( nIndex >= GetFieldCount() )
					return xgc_nullptr;
				return m_row[nIndex];
			}

			xgc_size GetFieldLength( xgc_uint32 nIndex )
			{
				if( nIndex < GetFieldCount() )
				{
					xgc_ulong * lengths = mysql_fetch_lengths( m_res );
					if( lengths )
					{
						return lengths[nIndex];
					}
				}

				return 0U;
			}
			
		private:
			MYSQL_RES*		m_res;
			MYSQL_ROW		m_row;
			MYSQL_FIELD*	m_fields;
		};

		//**
		//@brief 封装MySQL的连接
		//*
		//*	封装MySQL的连接，不可由多个线程同时操作本对象实例
		//*/

		class CMySQLConnection
		{
		public:
			///构造函数
			CMySQLConnection( void );

			///析构函数
			~CMySQLConnection( void );

			///设置连接信息
			xgc_void SetConnectionString( connection_cfg cfg );

			///打开连接
			xgc_bool Connect( connection_cfg cfg );

			///打开连接，在设置连接信息后调用
			xgc_bool Connect();

			///关闭连接
			xgc_void Close();

			///重置连接
			xgc_bool ResetConnection();

			///是否已连接
			xgc_bool IsConnected();

			///验证SQL字符串是否合法
			xgc_bool CheckSqlString( xgc_lpcstr lpSql );

			///转义字符串，使之可用于MYSQL查询
			xgc_size EscapeString( xgc_lpcstr pszSrc, xgc_size nSize, char* pszDest )
			{
				if( mConnection )
				{
					return mysql_real_escape_string( mConnection, pszDest, pszSrc, (xgc_ulong)nSize );
				}

				return 0U;
			}

			///返回受到最后一个UPDATE、DELETE或INSERT查询影响(变化)的行数
			xgc_size GetAffectedRows()
			{
				return (xgc_size)mysql_affected_rows( mConnection );
			}

			///转义字符串，使之可用于MYSQL查询，主要用在转意2进制数据
			size_t EscapeStringEx( xgc_lpcstr pszSrc, xgc_ulong nSize, xgc_lpstr pszDest )
			{
				if( mConnection )
				{
					return mysql_real_escape_string( mConnection, pszDest, pszSrc, nSize );
				}
				else
				{
					return 0;
				}
			}

			///执行一条SQL语句，不返回记录集
			xgc_bool Execute( xgc_lpcstr pszSQL );

			///执行一条SQL语句，返回记录集
			xgc_bool MoreResult();

			///执行一条SQL语句，返回记录集
			sql_result Execute( xgc_lpcstr pszSQL, CMySQLRecordSet &rcdSet );

			///无差别执行SQL语句，通过返回值判断具体效果(支持二进制代码数据)
			sql_result Execute( xgc_lpcstr pszSQL, xgc_ulong nLength, CMySQLRecordSet &rcdSet );

			///执行一条SQL语句，返回记录集
			sql_result MoreResult( CMySQLRecordSet& rcdSet );

			///获取上一次错误号
			xgc_uint32 GetLastError()const
			{
				if( mConnection != xgc_nullptr )
				{
					return mysql_errno( mConnection );
				}
				return CR_UNKNOWN_ERROR;
			}

			xgc_lpcstr GetLastStatus()const
			{
				if( mConnection != xgc_nullptr )
				{
					return mysql_sqlstate( mConnection );
				}
				return "UNKNOWN";
			}

			///获取上一次错误描述
			xgc_lpcstr GetLastErrorInfo()const
			{
				if( mConnection != xgc_nullptr )
				{
					return mysql_error( mConnection );
				}
				return "UNKNOWN";
			}

			xgc_void CheckOwnerThread( xgc_bool bCheck = true )
			{
				if( gettid() != mOwnerThread )
				{
					if( bCheck && mCheckThread )
					{
						XGC_ASSERT_MESSAGE( false, "与之前不同的线程被发现。" );
						SYS_ERROR( "[DB] database connection run at different thread.old = %d, new = %d", mOwnerThread, gettid() );
					}

					mOwnerThread = gettid();
				}
			}
		private:
			xgc_char    mServerAddr[32];
			xgc_char    mSchema[32];
			xgc_char    mUsername[64];
			xgc_char    mPassword[128];
			xgc_char	mCharacterSet[32];

			xgc_uint16	mServerPort;	// 服务器端口号
			xgc_uint16	mRetryCnt;		// 每次查询失败后重试的次数
			MYSQL*		mConnection;	// 数据库连接句柄
			xgc_bool	mCheckThread;	// 是否检查线程

			xgc_uint32	mOwnerThread;	// 最后一次的线程ID
		};
	};
};
#endif // _DB_MYSQL_H_