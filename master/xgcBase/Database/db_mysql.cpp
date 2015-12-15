#include "stdafx.h"
#include "Database.h"
#include "db_mysql.h"

namespace XGC
{
	namespace DB
	{
		CMySQLConnection::CMySQLConnection( void )
			: mConnection( xgc_nullptr )
			, mServerPort( 0 )
			, mRetryCnt( 1 )
			, mOwnerThread( 0 )
		{
			memset( mServerAddr, 0, sizeof( mServerAddr ) );
			memset( mSchema, 0, sizeof( mSchema ) );
			memset( mUsername, 0, sizeof( mUsername ) );
			memset( mPassword, 0, sizeof( mPassword ) );
		}

		CMySQLConnection::~CMySQLConnection( void )
		{
			if( mConnection )
			{
				mysql_close( mConnection );
			}
		}

		xgc_void CMySQLConnection::SetConnectionString( connection cfg )
		{
			strcpy_s( mServerAddr, sizeof(mServerAddr)-1, cfg.host );
			strcpy_s( mSchema, sizeof(mSchema)-1, cfg.schema );
			strcpy_s( mUsername, sizeof(mUsername)-1, cfg.username );
			strcpy_s( mPassword, sizeof(mPassword)-1, cfg.password );
			strcpy_s( mCharacterSet, sizeof(mCharacterSet)-1, cfg.character );
			mServerPort = cfg.port;
			mRetryCnt = cfg.retry;
		}

		xgc_bool CMySQLConnection::Connect( connection cfg )
		{
			SetConnectionString( cfg );
			return Connect();
		}

		xgc_bool CMySQLConnection::Connect()
		{
			if( mConnection == xgc_nullptr )
			{
				mConnection = mysql_init( xgc_nullptr );
			}

			if( mConnection == xgc_nullptr )
			{
				SYS_ERROR( "mysql_init failed." );
				return false;
			}

			CheckOwnerThread( false );
			if( '\0' != mCharacterSet[0] )
			{
				if( mysql_options( mConnection, MYSQL_SET_CHARSET_NAME, mCharacterSet ) )
				{
					SYS_ERROR( "mysql_options failed. %u - %s, %s",
						mysql_errno( mConnection ),
						mysql_error( mConnection ),
						mysql_sqlstate( mConnection ) );
					return false;
				}
			}

			MYSQL* myConn = mysql_real_connect( 
				mConnection, 
				mServerAddr,
				mUsername, 
				mPassword, 
				mSchema, 
				mServerPort, 
				xgc_nullptr, 
				CLIENT_MULTI_STATEMENTS );

			if( !myConn )
			{
				SYS_ERROR( "mysql_real_connect failed. %u - %s, %s", 
					mysql_errno( mConnection ), 
					mysql_error( mConnection ),
					mysql_sqlstate( mConnection ) );
				Close();
				return false;
			}

			xgc_char szSql[1024];
			sprintf_s( szSql, "set names %s;", mCharacterSet );
			if( Execute( szSql ) == false )
			{
				SYS_ERROR( "run sql '%s' failed. %u - %s, %s",
					szSql,
					mysql_errno( mConnection ),
					mysql_error( mConnection ),
					mysql_sqlstate( mConnection ) );
				Close();
				return false;
			}

			return true;
		}

		xgc_void CMySQLConnection::Close()
		{
			if( mConnection )
			{
				mysql_close( mConnection );
				mConnection = xgc_nullptr;
			}
		}

		xgc_bool CMySQLConnection::ResetConnection()
		{
			Close();
			return Connect();
		}

		xgc_bool CMySQLConnection::IsConnected()
		{
			XGC_ASSERT_RETURN( mConnection, false );

			return mysql_ping( mConnection ) == 0;
		}

		///验证SQL字符串是否合法
		xgc_bool CMySQLConnection::CheckSqlString( xgc_lpcstr lpSql )
		{
			xgc_char szEnd = 0;
			while( *lpSql )
			{
				if( !isblank( *lpSql ) )
					szEnd = *lpSql;

				++lpSql;
			}

			return szEnd == ';';
		}

		xgc_bool CMySQLConnection::Execute( xgc_lpcstr pszSQL )
		{
			CheckOwnerThread();
			// XGC_ASSERT( CheckSqlString( pszSQL ) );
			if( xgc_nullptr == mConnection )
			{
				if( ResetConnection() == false )
				{
					USR_WARNING( "[DB] server lost. connection = %p", mConnection );
					return false;
				}
			}
			
			// 吸收未处理的数据库操作
			while( mysql_more_results( mConnection ) )
			{
				if( mysql_next_result( mConnection ) <= 0 )
				{
					MYSQL_RES* pRes = mysql_store_result( mConnection );
					mysql_free_result( pRes );
				}
			}

			xgc_uint16 nRetry = mRetryCnt;
			do
			{
				if( 0 == mysql_query( mConnection, pszSQL ) )
					return true;

				USR_WARNING( "[DB] execute sql '%s' failed", pszSQL );
				USR_WARNING( "[DB] errno = %d, state = %s, errmsg = %s, retry = %d"
					, GetLastError()
					, GetLastStatus()
					, GetLastErrorInfo()
					, nRetry );

				auto err = mysql_errno( mConnection );
				
				if( err != CR_SERVER_GONE_ERROR &&
					err != CR_SERVER_LOST )
				{
					return false;
				}

				ResetConnection();
			} while( --nRetry > 0 );

			return nRetry > 0;
		}

		sql_result CMySQLConnection::Execute( xgc_lpcstr pszSQL, CMySQLRecordSet &rcdSet )
		{
			CheckOwnerThread();
			// XGC_ASSERT( CheckSqlString( pszSQL ) );
			if( xgc_nullptr == mConnection )
			{
				if( ResetConnection() == false )
				{
					USR_WARNING( "[DB] server lost. connection = %p, %u - %s, %s", 
						mConnection
						, GetLastError()
						, GetLastStatus()
						, GetLastErrorInfo() );
					return sql_failed;
				}
			}

			// 吸收未处理的数据库操作
			while( mysql_more_results( mConnection ) )
			{
				if( mysql_next_result( mConnection ) <= 0 )
				{
					MYSQL_RES* pRes = mysql_store_result( mConnection );
					mysql_free_result( pRes );
				}
			}

			xgc_uint16 nRetry = mRetryCnt;
			do
			{
				if( 0 == mysql_query( mConnection, pszSQL ) )
					break;

				USR_WARNING( "[DB] execute sql '%s' failed. errno = %d, state = %s, errmsg = %s, retry = %d"
					, pszSQL ? pszSQL : "nullptr"
					, GetLastError()
					, GetLastStatus()
					, GetLastErrorInfo()
					, nRetry );

				auto err = mysql_errno( mConnection );

				if( err != CR_SERVER_GONE_ERROR &&
					err != CR_SERVER_LOST )
				{
					return sql_failed;
				}

				ResetConnection();
			} while( --nRetry > 0 );

			if( nRetry == 0 )
				return sql_failed;

			if( rcdSet.m_res )
			{
				mysql_free_result( rcdSet.m_res );
				rcdSet.m_res = xgc_nullptr;
			}
			
			rcdSet.m_res = mysql_store_result( mConnection );
			if( rcdSet.m_res == xgc_nullptr )
			{
				USR_WARNING( "[DB] store result failed. sql = '%s', errno = %d, state = %s, errmsg = %s, retry=%u"
					, pszSQL ? pszSQL : "nullptr"
					, GetLastError()
					, GetLastStatus()
					, GetLastErrorInfo()
					, nRetry );
				return sql_failed; ///record set
			}

			if( rcdSet.GetFieldCount() > 0 && rcdSet.GetRecordCount() > 0 )
			{
				return sql_success;
			}

			return  sql_empty;       ///NO Rescord Set
		}

		sql_result CMySQLConnection::Execute( xgc_lpcstr pszSQL, xgc_ulong nLength, CMySQLRecordSet &rcdSet )
		{
			CheckOwnerThread();
			// XGC_ASSERT( CheckSqlString( pszSQL ) );
			if( xgc_nullptr == mConnection )
			{
				if( ResetConnection() == false )
				{
					USR_WARNING( "[DB] server lost. connection = %p", mConnection );
					return sql_failed;
				}
			}

			// 吸收未处理的数据库操作
			while( mysql_more_results( mConnection ) )
			{
				if( mysql_next_result( mConnection ) <= 0 )
				{
					MYSQL_RES* pRes = mysql_store_result( mConnection );
					mysql_free_result( pRes );
				}
			}

			xgc_uint16 nRetry = mRetryCnt;
			do
			{
				if( 0 == mysql_real_query( mConnection, pszSQL, nLength ) )
					break;

				USR_WARNING( "[DB] execute sql '%s' failed. errno = %d, state = %s, errmsg = %s, retry = %d"
					, pszSQL ? pszSQL : "nullptr"
					, GetLastError()
					, GetLastStatus()
					, GetLastError()
					, nRetry );

				auto err = mysql_errno( mConnection );

				if( err != CR_SERVER_GONE_ERROR &&
					err != CR_SERVER_LOST )
				{
					return sql_failed;
				}

				ResetConnection();
			} while( --nRetry > 0 );

			if( nRetry == 0 )
				return sql_failed;

			if( rcdSet.m_res )
			{
				mysql_free_result( rcdSet.m_res );
				rcdSet.m_res = xgc_nullptr;
			}
			
			rcdSet.m_res = mysql_store_result( mConnection );

			if( rcdSet.m_res == xgc_nullptr )
			{
				USR_WARNING( "[DB] store result failed. sql = '%s', errno = %d, state = %s, errmsg = %s"
					, pszSQL ? pszSQL : "nullptr"
					, GetLastError()
					, GetLastStatus()
					, GetLastError()
					, nRetry );
				return sql_failed; ///record set
			}

			if( rcdSet.GetFieldCount() > 0 && rcdSet.GetRecordCount() > 0 )
			{
				return sql_success;
			}

			return  sql_empty;       ///NO Rescord Set
		}

		///执行下一条SQL语句
		xgc_bool CMySQLConnection::MoreResult()
		{
			CheckOwnerThread();
			if( xgc_nullptr == mConnection )
			{
				if( ResetConnection() == false )
				{
					USR_WARNING( "[DB] server lost. connection = %p", mConnection );
					return false;
				}
			}

			return mysql_next_result( mConnection ) == 0;
		}

		///执行下一条SQL语句，返回记录集
		sql_result CMySQLConnection::MoreResult( CMySQLRecordSet& rcdSet )
		{
			CheckOwnerThread();
			if( xgc_nullptr == mConnection )
			{
				if( ResetConnection() == false )
				{
					USR_WARNING( "[DB] server lost. connection = %p", mConnection );
					return sql_failed;
				}
			}

			if( rcdSet.m_res )
			{
				mysql_free_result( rcdSet.m_res );
				rcdSet.m_res = xgc_nullptr;
			}

			int result = mysql_next_result( mConnection );
			if( result == 0 )
			{
				rcdSet.m_res = mysql_store_result( mConnection );
				if( rcdSet.m_res && rcdSet.GetFieldCount() > 0 && rcdSet.GetRecordCount() > 0 )
				{
					return sql_success;
				}
			}
			else if( result > 0 )
			{
				return sql_failed;
			}

			return  sql_empty;       ///NO Rescord Set
		}
	}
}
