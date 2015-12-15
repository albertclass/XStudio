// Database.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Database.h"
#include "db_mysql.h"
#include "synchronous.h"
using namespace XGC::common;
namespace XGC
{
	namespace DB
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
			static_assert( 0 < sizeof( CMySQLRecordSet ),
				"can't delete an incomplete type" );
			delete _Ptr;
		}

		class CMySqlDatabase
		{
		private:
			uintptr_t		m_thread_h;
			bool			m_work_b;
			event			m_event;

			sql_connection	mConnection;

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
			CMySqlDatabase(void)
				: m_event( false, true )
			{
			}

			~CMySqlDatabase(void)
			{
			}

			sql_connection get_connection()
			{
				return mConnection;
			}

			//--------------------------------------------------------//
			//	created:	29:12:2009   16:48
			//	filename: 	DatabaseService
			//	author:		Albert.xu
			//
			//	purpose:	创建数据库服务
			//--------------------------------------------------------//
			xgc_bool start( connection cfg )
			{
				mConnection = connect( cfg, false );
				if( mConnection == 0 )
				{
					return false;
				}

				m_thread_h = _beginthreadex( xgc_nullptr, 0, CMySqlDatabase::svc, (void*)this, 0, xgc_nullptr );
				if( m_thread_h == -1 )
					return false;

				m_event.wait();
				return m_work_b;
			}

			void stop()
			{
				m_work_b = false;
				WaitForSingleObject( (HANDLE)m_thread_h, INFINITE );
			}

			void put_request( const postcall &post )
			{
				autolock lock( m_section_req );
				m_transreq.push( std::move( post ) );
			}

			postcall get_request()
			{
				autolock lock( m_section_req );

				pop_when_return< trans_req > pop( m_transreq );
				if( m_transreq.empty() )
				{
					return xgc_nullptr;
				}
				else
				{
					return std::move( m_transreq.front() );
				}
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

				pop_when_return< trans_rpn > pop( m_transrpn );
				if( m_transrpn.empty() )
					return xgc_nullptr;
				else
					return std::move( m_transrpn.front() );
			}

		protected:
			// 数据库服务线程
			static xgc_uint32 __stdcall svc( xgc_lpvoid pParam )
			{
				CMySqlDatabase *pService = (CMySqlDatabase*)pParam;

				DB::sql_connection conn = pService->mConnection;
				if( conn )
				{
					if( false == reconnect( conn ) )
					{
						SYS_ERROR( "异步数据库连接失败! %d - %s", get_error_code( conn ), get_error_info( conn ) );
					}

					pService->m_work_b = true;
					pService->m_event.set();
					while( pService->m_work_b )
					{
						postcall post = pService->get_request();
						// 处理数据
						if( post )
						{
							pService->put_response( post( conn ) );
						}
						else
						{
							Sleep( 1 );
						}
					}

					disconnect( conn );
				}
				else
				{
					pService->m_work_b = false;
					pService->m_event.set();
				}
				return 0;
			}

		private:
			typedef std::queue< postcall > trans_req;
			typedef std::queue< respcall > trans_rpn;

			sync_section m_section_req, m_section_rpn;
			trans_req	m_transreq;
			trans_rpn	m_transrpn;
		};

		//////////////////////////////////////////////////////////////////////////
		// service
		//////////////////////////////////////////////////////////////////////////
		// 创建数据库服务
		sql_database async_connect( connection cfg )
		{
			CMySqlDatabase *s = XGC_NEW CMySqlDatabase();
			if( s && s->start( cfg ) )
			{
				return reinterpret_cast<sql_database>( s );
			}

			SAFE_DELETE( s );
			return xgc_nullptr;
		}

		sql_connection get_database_conn( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*) s;
			if( !d )
				return 0;

			return d->get_connection();
		}

		// 销毁数据库服务
		xgc_void async_close( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*)s;
			if( d )
			{
				d->stop();
				delete d;
			}
		}

		// 投递数据库请求
		xgc_void async_post( sql_database s, const postcall &call )
		{
			CMySqlDatabase *d = (CMySqlDatabase*)s;
			if( d )
			{
				d->put_request( call );
			}
		}

		// 获取数据库回应
		respcall async_resp( sql_database s )
		{
			CMySqlDatabase *d = (CMySqlDatabase*)s;
			if( d )
			{
				return std::move( d->get_response() );
			}
			return xgc_nullptr;
		}

		xgc_bool initial()
		{
			return true;
		}

		xgc_void final()
		{
		}
	}
}
