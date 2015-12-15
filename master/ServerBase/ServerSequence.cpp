#include "stdafx.h"
#include "ServerSequence.h"
#include "ServerBase.h"

namespace ServerSequence
{
	xgc_uint32 gKey = 0;          ///< ���ݿ��Ψһ���
	xgc_uint8  gIndex = 0;        ///< ���̶�Ӧ�ı��λֵ
	xgc_uint16 gServerCode = 0;   ///< �������λ
	xgc_uint32 gSIDSize = 1000;   ///< SID�Ĵ�С
	xgc_uint64 gSIDPos = 0;       ///< ��ǰ��ֵ
	xgc_uint64 gSIDMax = 0;       ///< ����ֵ

	xgc_bool InitServerSequence()
	{
		FUNCTION_BEGIN;

		xgc_lpcstr lpstrPipe = GetServerCode();
		xgc_uint32 nPipe = GetServerCode( xgc_nullptr );
		xgc_byte *pCode = (xgc_byte*) &nPipe;
		gServerCode = ( pCode[0] << 8 | pCode[1] );

		{
			// pipe2indexȡ��һ��index
			xgc_char sql[1024] = { 0 };
			sprintf_s( sql, sizeof( sql ), "CALL procedure_index_from_pipe2index ('%s', @_index);", lpstrPipe );
			XGC::DB::sql_recordset rs = 0;
			if( SyncDBExecuteRc( sql, rs ) == XGC::DB::sql_result::sql_failed )
			{
				SYS_ERROR( "[%s]��ѯIndex��ǳ���", lpstrPipe );
				return false;
			}
			if( movenext( rs ) )
			{
				gIndex = (xgc_uint8) field_unsigned( rs, 0, 0 );
				if( gIndex == 0 )
				{
					SYS_ERROR( "[%s]��ѯIndex��ǳ���", lpstrPipe );
					return false;
				}
			}
		}
		{
			xgc_uint32 nId = 0;
			xgc_uint64 nSid = 0;
			xgc_char sql[1024] = { 0 };
			sprintf_s( sql, sizeof( sql ), "SELECT id, sid FROM sequenceid WHERE pipeindex = %u", xgc_uint32( gIndex ) );
			XGC::DB::sql_recordset rs = 0;
			auto result = SyncDBExecuteRc( sql, rs );
			if( result == XGC::DB::sql_result::sql_failed )
			{
				SYS_ERROR( "����Ѵ�����ʧ��:%s", SyncDBErrorInfo() );
				return false;
			}
			else if( result == XGC::DB::sql_result::sql_empty )
			{
				// ����һ��
				sprintf_s( sql, sizeof( sql ), "INSERT INTO sequenceid (pipeindex, sid) VALUES (%u, %I64u)", xgc_uint32( gIndex ), xgc_uint64( gSIDSize ) );
				if( false == SyncDBExecute( sql ) )
				{
					SYS_ERROR( "Insert sid ʧ��:%s", SyncDBErrorInfo() );
					return false;
				}
			}
			else
			{
				if( movenext( rs ) )
				{
					nId = field_unsigned( rs, 0, 0 );
					nSid = field_ulonglong( rs, 1, 0 );

					// �µ�sid�� nSid + gSidSize				
					sprintf_s( sql, sizeof( sql ), "UPDATE sequenceid set sid = sid + %u WHERE id = %u", gSIDSize, nId );
					if( false == SyncDBExecute( sql ) )
					{
						SYS_ERROR( "Update sid ʧ��:%s", SyncDBErrorInfo() );
						return false;
					}
				}
				else
				{
					SYS_ERROR( "��ȡ sid ʧ��" );
					return false;
				}
			}
			// ��ֵ��ʼλ��
			gSIDPos = nSid;
			gSIDMax = gSIDPos + gSIDSize;
			gKey = nId;
		}

		if( ( gServerCode > 0xFFFF ) || ( gIndex > 0xFF ) )
		{
			SYS_ERROR( "gServerCode [%u], gIndex [%u] ĳ��ֵ�������ֵ", gServerCode, gIndex );
			return false;
		}
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_uint64 GetSID()
	{
		FUNCTION_BEGIN;
		
		if( gIndex == 0 )
		{
			SYS_ERROR( "��û�г�ʼ��Sequence��" );
			return 0;
		}

		auto FnCalcSID = []()->xgc_uint64
		{
			// �ṹ��16λ���� 8λ����ID 40λSequenceID
			xgc_uint64 _sid = 0;
			_sid = ( ( ( xgc_uint64( gServerCode ) & 0xFFFF ) << 48 ) | ( ( xgc_uint64( gIndex ) & 0xFF ) << 40 ) );

			if( gSIDPos > 0xFFFFFFFFFF )
			{
				SYS_ERROR( "SID-sequence�����ˣ����������ֵ" );
				return 0;
			}
			_sid += ( gSIDPos & 0xFFFFFFFFFF );

			gSIDPos += 1;
			return _sid;
		};

		if( gSIDPos < gSIDMax )
		{
			return FnCalcSID();
		}
		else
		{
			// ���»�ȡһ�� gSIDPos, gSIDMax

			// �µ�sid�� nSid + gSidSize		
			xgc_char sql[1024] = { 0 };
			sprintf_s( sql, sizeof( sql ), "UPDATE sequenceid set sid = sid + %u WHERE id = %u", gSIDSize, gKey );
			if( false == SyncDBExecute( sql ) )
			{
				SYS_ERROR( "Update sid ʧ��:%s", SyncDBErrorInfo() );
				return 0;
			}
			// ��ֵ��ʼλ��
			gSIDPos = gSIDMax;
			gSIDMax = gSIDPos + gSIDSize;

			return FnCalcSID();
		}
		
		return 0;
		FUNCTION_END;
		return 0;
	}
};