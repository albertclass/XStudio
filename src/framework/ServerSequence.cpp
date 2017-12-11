#include "ServerDefines.h"
#include "ServerBase.h"
#include "ServerDatabase.h"
#include "ServerSequence.h"

xgc_uint32 gKey = 0;          ///< ���ݿ��Ψһ���
xgc_uint8  gIndex = 0;        ///< ���̶�Ӧ�ı��λֵ

NETWORK_REGION gServerCode = 0;   ///< �������λ
xgc_uint32 gSIDSize = 1000;   ///< SID�Ĵ�С
xgc_uint64 gSIDPos = 0;       ///< ��ǰ��ֵ
xgc_uint64 gSIDMax = 0;       ///< ����ֵ

xgc_bool InitServerSequence()
{
	FUNCTION_BEGIN;

	NETWORK_ID nNetworkId = GetNetworkId();

	xgc_char szNetworkId[NETWORK_REGION_S];
	NetworkId2Str( nNetworkId, szNetworkId );

	gServerCode = NETWORK_INDEX( nNetworkId );

	{
		// pipe2indexȡ��һ��index
		xgc_char sql[1024] = { 0 };
		sprintf_s( sql, sizeof( sql ), "call procedure_index_from_pipe2index ('%s', @_index);", szNetworkId );
		sql_recordset rs = 0;
		if( SyncDBExecuteRc( sql, rs ) == sql_result::sql_failed )
		{
			SYS_ERROR( "[%s]��ѯIndex��ǳ���", szNetworkId );
			return false;
		}

		if( movenext( rs ) )
		{
			gIndex = (xgc_uint8) field_unsigned( rs, 0, 0 );
			if( gIndex == 0 )
			{
				SYS_ERROR( "[%s]��ѯIndex��ǳ���", szNetworkId );
				return false;
			}
		}
	}

	{
		xgc_uint32 nId = 0;
		xgc_uint64 nSid = 0;
		xgc_char sql[1024] = { 0 };
		sprintf_s( sql, sizeof( sql ), "select id, sid from sequenceid where pipeindex = %u", xgc_uint32( gIndex ) );
		xgc::sql::sql_recordset rs = 0;
		auto result = SyncDBExecuteRc( sql, rs );
		if( result == xgc::sql::sql_result::sql_failed )
		{
			SYS_ERROR( "����Ѵ�����ʧ��:%s", SyncDBErrorInfo() );
			return false;
		}
		else if( result == xgc::sql::sql_result::sql_empty )
		{
			// ����һ��
			sprintf_s( sql, sizeof( sql ), "insert into sequenceid (pipeindex, sid) values (%u, %llu)", xgc_uint32( gIndex ), xgc_uint64( gSIDSize ) );
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
