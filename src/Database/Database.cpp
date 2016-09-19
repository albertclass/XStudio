// Database.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Database.h"
#include "db_mysql.h"
using namespace xgc::common;

#include <mutex>

namespace xgc
{
	namespace sql
	{
		const xgc_uint16 DB_FIELD_TYPE_DECIMAL = MYSQL_TYPE_DECIMAL;
		const xgc_uint16 DB_FIELD_TYPE_NEWDECIMAL = MYSQL_TYPE_NEWDECIMAL;
		const xgc_uint16 DB_FIELD_TYPE_TINY = MYSQL_TYPE_TINY;
		const xgc_uint16 DB_FIELD_TYPE_SHORT = MYSQL_TYPE_SHORT;
		const xgc_uint16 DB_FIELD_TYPE_LONG = MYSQL_TYPE_LONG;
		const xgc_uint16 DB_FIELD_TYPE_FLOAT = MYSQL_TYPE_FLOAT;
		const xgc_uint16 DB_FIELD_TYPE_DOUBLE = MYSQL_TYPE_DOUBLE;
		const xgc_uint16 DB_FIELD_TYPE_NULL = MYSQL_TYPE_NULL;
		const xgc_uint16 DB_FIELD_TYPE_TIMESTAMP = MYSQL_TYPE_TIMESTAMP;
		const xgc_uint16 DB_FIELD_TYPE_LONGLONG = MYSQL_TYPE_LONGLONG;
		const xgc_uint16 DB_FIELD_TYPE_INT24 = MYSQL_TYPE_INT24;
		const xgc_uint16 DB_FIELD_TYPE_DATE = MYSQL_TYPE_DATE;
		const xgc_uint16 DB_FIELD_TYPE_TIME = MYSQL_TYPE_TIME;
		const xgc_uint16 DB_FIELD_TYPE_DATETIME = MYSQL_TYPE_DATETIME;
		const xgc_uint16 DB_FIELD_TYPE_YEAR = MYSQL_TYPE_YEAR;
		const xgc_uint16 DB_FIELD_TYPE_NEWDATE = MYSQL_TYPE_NEWDATE;
		const xgc_uint16 DB_FIELD_TYPE_ENUM = MYSQL_TYPE_ENUM;
		const xgc_uint16 DB_FIELD_TYPE_SET = MYSQL_TYPE_SET;
		const xgc_uint16 DB_FIELD_TYPE_TINY_BLOB = MYSQL_TYPE_TINY_BLOB;
		const xgc_uint16 DB_FIELD_TYPE_MEDIUM_BLOB = MYSQL_TYPE_MEDIUM_BLOB;
		const xgc_uint16 DB_FIELD_TYPE_LONG_BLOB = MYSQL_TYPE_LONG_BLOB;
		const xgc_uint16 DB_FIELD_TYPE_BLOB = MYSQL_TYPE_BLOB;
		const xgc_uint16 DB_FIELD_TYPE_VAR_STRING = MYSQL_TYPE_VAR_STRING;
		const xgc_uint16 DB_FIELD_TYPE_STRING = MYSQL_TYPE_STRING;
		const xgc_uint16 DB_FIELD_TYPE_CHAR = MYSQL_TYPE_TINY;
		const xgc_uint16 DB_FIELD_TYPE_INTERVAL = MYSQL_TYPE_ENUM;
		const xgc_uint16 DB_FIELD_TYPE_GEOMETRY = MYSQL_TYPE_GEOMETRY;
		const xgc_uint16 DB_FIELD_TYPE_BIT = MYSQL_TYPE_BIT;

		//////////////////////////////////////////////////////////////////////////
		// field flags
		const xgc_uint32 DB_FIELD_NOT_NULL_FLAG = NOT_NULL_FLAG;
		const xgc_uint32 DB_FIELD_PRI_KEY_FLAG = PRI_KEY_FLAG;
		const xgc_uint32 DB_FIELD_UNIQUE_KEY_FLAG = UNIQUE_KEY_FLAG;
		const xgc_uint32 DB_FIELD_MULTIPLE_KEY_FLAG = MULTIPLE_KEY_FLAG;
		const xgc_uint32 DB_FIELD_BLOB_FLAG = BLOB_FLAG;
		const xgc_uint32 DB_FIELD_UNSIGNED_FLAG = UNSIGNED_FLAG;
		const xgc_uint32 DB_FIELD_ZEROFILL_FLAG = ZEROFILL_FLAG;
		const xgc_uint32 DB_FIELD_BINARY_FLAG = BINARY_FLAG;

		/* The following are only sent to new clients */
		const xgc_uint32 DB_FIELD_ENUM_FLAG = ENUM_FLAG;
		const xgc_uint32 DB_FIELD_AUTO_INCREMENT_FLAG = AUTO_INCREMENT_FLAG;
		const xgc_uint32 DB_FIELD_TIMESTAMP_FLAG = TIMESTAMP_FLAG;
		const xgc_uint32 DB_FIELD_SET_FLAG = SET_FLAG;
		const xgc_uint32 DB_FIELD_NO_DEFAULT_VALUE_FLAG = NO_DEFAULT_VALUE_FLAG;
		const xgc_uint32 DB_FIELD_ON_UPDATE_NOW_FLAG = ON_UPDATE_NOW_FLAG;
		const xgc_uint32 DB_FIELD_NUM_FLAG = NUM_FLAG;
		const xgc_uint32 DB_FIELD_PART_KEY_FLAG = PART_KEY_FLAG;
		const xgc_uint32 DB_FIELD_GROUP_FLAG = GROUP_FLAG;
		const xgc_uint32 DB_FIELD_UNIQUE_FLAG = UNIQUE_FLAG;
		const xgc_uint32 DB_FIELD_BINCMP_FLAG = BINCMP_FLAG;

		void sql_recordset_deleter::operator()( CMySQLRecordSet *_Ptr ) const _NOEXCEPT
		{	// delete a pointer
			static_assert(0 < sizeof( CMySQLRecordSet ),
						   "can't delete an incomplete type");
			delete _Ptr;
		}

		using std::thread;
		using std::condition_variable;
		using std::mutex;

		using autolock = std::unique_lock< std::mutex >;
		class CMySqlDatabase
		{
		private:
			thread              m_thread;
			mutex				m_singel;
			condition_variable	m_event;
			xgc_bool			m_working;
			sql_connection		m_connection;

			template< typename _Queue >
			struct pop_when_return
			{
				typedef typename _Queue::value_type		value_type;
				typedef typename _Queue::container_type container_type;
				pop_when_return( std::queue< value_type, container_type > &q )
					: _q( q )
				{

				}

				~pop_when_return()
				{
					if( !_q.empty() )
						_q.pop();
				}

				_Queue &_q;
			};
		public:
			CMySqlDatabase( void )
			{
			}

			~CMySqlDatabase( void )
			{
			}

			sql_connection get_connection()
			{
				return m_connection;
			}

			///
			/// \brief 启动数据库异步线程
			///
			/// \author albert.xu
			/// \date 2015/12/16 19:02
			///
			xgc_bool start( connection_cfg cfg, xgc_bool sync = false )
			{
				m_connection = connect( cfg, sync );
				if( m_connection == 0 )
				{
					return false;
				}
				if( sync )
				{
					m_working = true;
					return true;
				}
				m_thread = std::thread( &CMySqlDatabase::svc, this );
				m_event.wait( autolock( m_singel ) );
				return m_working;
			}

			xgc_void stop()
			{
				m_working = false;
				if( m_thread.joinable() )
					m_thread.join();
			}

			xgc_void put_request( const postcall &post )
			{
				autolock lock( m_section_req );
				m_transreq.push( std::move( post ) );
			}

			postcall get_request()
			{
				autolock lock( m_section_req );
				if( m_transreq.empty() )
					return xgc_nullptr;

				pop_when_return< trans_req > pop( m_transreq );
				return std::move( m_transreq.front() );
			}

			void put_response( const respcall &&resp )
			{
				if( xgc_nullptr == resp )
					return;

				autolock lock( m_section_rpn );
				m_transrpn.push( resp );
			}

			respcall get_response()
			{
				autolock lock( m_section_rpn );

				if( m_transrpn.empty() )
					return xgc_nullptr;

				pop_when_return< trans_rpn > pop( m_transrpn );
				return std::move( m_transrpn.front() );
			}

		protected:
			// 数据库服务线程
			static xgc_uint32 __stdcall svc( xgc_lpvoid pParam )
			{
				CMySqlDatabase *pService = (CMySqlDatabase*) pParam;

				sql_connection conn = pService->m_connection;
				if( conn == xgc_nullptr )
				{
					pService->m_working = false;
					pService->m_event.notify_one();
					return -1;
				}

				if( false == reconnect( conn ) )
				{
					SYS_ERROR( "异步数据库连接失败! %d - %s", get_error_code( conn ), get_error_info( conn ) );
				}

				pService->m_working = true;
				pService->m_event.notify_one();

				while( pService->m_working )
				{
					postcall post = pService->get_request();
					// 处理数据
					if( post )
					{
						pService->put_response( post( conn ) );
					}
					else
					{
						std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
					}
				}

				disconnect( conn );
				return 0;
			}

		private:
			typedef std::queue< postcall > trans_req;
			typedef std::queue< respcall > trans_rpn;

			mutex  m_section_req;
			mutex  m_section_rpn;

			trans_req	m_transreq;
			trans_rpn	m_transrpn;
		};

		//////////////////////////////////////////////////////////////////////////
		// service
		//////////////////////////////////////////////////////////////////////////
		// 创建数据库服务
		sql_database async_connect( connection_cfg cfg )
		{
			CMySqlDatabase *s = XGC_NEW CMySqlDatabase();
			if( s && s->start( cfg ) )
			{
				return reinterpret_cast<sql_database>(s);
			}

			SAFE_DELETE( s );
			return xgc_nullptr;
		}

		sql_database sync_connect( connection_cfg cfg )
		{
			CMySqlDatabase *s = XGC_NEW CMySqlDatabase();
			if( s && s->start( cfg, true ) )
			{
				return reinterpret_cast<sql_database>(s);
			}

			SAFE_DELETE( s );
			return xgc_nullptr;
		}

		xgc_size async_escape_string( sql_database s, xgc_lpcstr data, xgc_int32 data_size, xgc_lpstr buffer, xgc_size buffer_size )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( !d )
				return -1;

			mutex m;
			condition_variable cv;

			xgc_size ret = -1;
			d->put_request( [&]( sql_connection conn )->respcall{
				ret = escape_string( conn, data, data_size, buffer, buffer_size );
				cv.notify_all();
				return xgc_nullptr;
			} );

			cv.wait( autolock( m ) );

			return ret;
		}

		// 销毁数据库服务
		xgc_void async_close( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				d->stop();
				delete d;
			}
		}

		xgc_void sync_close( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				d->stop();
				delete d;
			}
		}

		// 投递数据库请求
		xgc_void async_post( sql_database s, const postcall &call )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				d->put_request( call );
			}
		}

		// 获取数据库回应
		respcall async_resp( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				return std::move( d->get_response() );
			}
			return xgc_nullptr;
		}

		xgc_bool sync_exec( sql_database s, xgc_lpcstr sql )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				return execute_sql( d->get_connection(), sql );
			}
			return false;
		}

		sql_result sync_exec_rc( sql_database s, xgc_lpcstr sql, sql_recordset &rs )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				return execute_sql_rc( d->get_connection(), sql, rs );
			}
			return sql_failed;
		}

		xgc_void sync_post( sql_database s, postcall && call )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				auto resp = call( d->get_connection() );
				if( resp )
				{
					resp();
				}
			}
		}
		xgc_uint32 get_error_code( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				return get_error_code( d );
			}
			return 9998;
		}
		xgc_lpcstr get_error_info( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( d )
			{
				return get_error_info( d );
			}
			return "error database.cpp";
		}
	}
}
