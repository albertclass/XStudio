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
			///���캯��
			CMySQLRecordSet() 
				: m_res( xgc_nullptr )
				, m_row( xgc_nullptr )
			{
			}

			///��������
			~CMySQLRecordSet()
			{
				mysql_free_result( m_res );
			}

			///��ȡ��¼����
			xgc_uint64 GetRecordCount() 
			{ 
				return ( m_res ? (xgc_uint64)mysql_num_rows( m_res ) : 0 ); 
			}

			///��ȡ�ֶθ���
			xgc_uint32 GetFieldCount()
			{
				return ( m_res ? (xgc_uint32)mysql_num_fields( m_res ) : 0 ); 
			}

			///�ƶ�����¼������
			xgc_bool MoveFirst()
			{
				if( m_res == xgc_nullptr )
					return false;

				mysql_data_seek( m_res, 0 );
				m_row = mysql_fetch_row( m_res );

				return ( m_row != xgc_nullptr );
			}

			///��ȡ��һ����¼��
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

			///�Ƿ��¼�׶�
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

			///��ȡ��ǰ��¼����ĳһ���ֶε�ֵ
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
		//@brief ��װMySQL������
		//*
		//*	��װMySQL�����ӣ������ɶ���߳�ͬʱ����������ʵ��
		//*/

		class CMySQLConnection
		{
		public:
			///���캯��
			CMySQLConnection( void );

			///��������
			~CMySQLConnection( void );

			///����������Ϣ
			xgc_void SetConnectionString( connection_cfg cfg );

			///������
			xgc_bool Connect( connection_cfg cfg );

			///�����ӣ�������������Ϣ�����
			xgc_bool Connect();

			///�ر�����
			xgc_void Close();

			///��������
			xgc_bool ResetConnection();

			///�Ƿ�������
			xgc_bool IsConnected();

			///��֤SQL�ַ����Ƿ�Ϸ�
			xgc_bool CheckSqlString( xgc_lpcstr lpSql );

			///ת���ַ�����ʹ֮������MYSQL��ѯ
			xgc_size EscapeString( xgc_lpcstr pszSrc, xgc_size nSize, char* pszDest )
			{
				if( mConnection )
				{
					return mysql_real_escape_string( mConnection, pszDest, pszSrc, (xgc_ulong)nSize );
				}

				return 0U;
			}

			///�����ܵ����һ��UPDATE��DELETE��INSERT��ѯӰ��(�仯)������
			xgc_size GetAffectedRows()
			{
				return (xgc_size)mysql_affected_rows( mConnection );
			}

			///ת���ַ�����ʹ֮������MYSQL��ѯ����Ҫ����ת��2��������
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

			///ִ��һ��SQL��䣬�����ؼ�¼��
			xgc_bool Execute( xgc_lpcstr pszSQL );

			///ִ��һ��SQL��䣬���ؼ�¼��
			xgc_bool MoreResult();

			///ִ��һ��SQL��䣬���ؼ�¼��
			sql_result Execute( xgc_lpcstr pszSQL, CMySQLRecordSet &rcdSet );

			///�޲��ִ��SQL��䣬ͨ������ֵ�жϾ���Ч��(֧�ֶ����ƴ�������)
			sql_result Execute( xgc_lpcstr pszSQL, xgc_ulong nLength, CMySQLRecordSet &rcdSet );

			///ִ��һ��SQL��䣬���ؼ�¼��
			sql_result MoreResult( CMySQLRecordSet& rcdSet );

			///��ȡ��һ�δ����
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

			///��ȡ��һ�δ�������
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
						XGC_ASSERT_MESSAGE( false, "��֮ǰ��ͬ���̱߳����֡�" );
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

			xgc_uint16	mServerPort;	// �������˿ں�
			xgc_uint16	mRetryCnt;		// ÿ�β�ѯʧ�ܺ����ԵĴ���
			MYSQL*		mConnection;	// ���ݿ����Ӿ��
			xgc_bool	mCheckThread;	// �Ƿ����߳�

			xgc_uint32	mOwnerThread;	// ���һ�ε��߳�ID
		};
	};
};
#endif // _DB_MYSQL_H_