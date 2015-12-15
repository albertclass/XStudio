#include "stdafx.h"
#include "ServerSequence.h"
#include "ServerBase.h"

namespace ServerSequence
{
	xgc_uint32 gKey = 0;          ///< 数据库的唯一标记
	xgc_uint8  gIndex = 0;        ///< 进程对应的标记位值
	xgc_uint16 gServerCode = 0;   ///< 区服标记位
	xgc_uint32 gSIDSize = 1000;   ///< SID的大小
	xgc_uint64 gSIDPos = 0;       ///< 当前的值
	xgc_uint64 gSIDMax = 0;       ///< 最大的值

	xgc_bool InitServerSequence()
	{
		FUNCTION_BEGIN;

		xgc_lpcstr lpstrPipe = GetServerCode();
		xgc_uint32 nPipe = GetServerCode( xgc_nullptr );
		xgc_byte *pCode = (xgc_byte*) &nPipe;
		gServerCode = ( pCode[0] << 8 | pCode[1] );

		{
			// pipe2index取得一个index
			xgc_char sql[1024] = { 0 };
			sprintf_s( sql, sizeof( sql ), "CALL procedure_index_from_pipe2index ('%s', @_index);", lpstrPipe );
			XGC::DB::sql_recordset rs = 0;
			if( SyncDBExecuteRc( sql, rs ) == XGC::DB::sql_result::sql_failed )
			{
				SYS_ERROR( "[%s]查询Index标记出错", lpstrPipe );
				return false;
			}
			if( movenext( rs ) )
			{
				gIndex = (xgc_uint8) field_unsigned( rs, 0, 0 );
				if( gIndex == 0 )
				{
					SYS_ERROR( "[%s]查询Index标记出错", lpstrPipe );
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
				SYS_ERROR( "检查已创建表失败:%s", SyncDBErrorInfo() );
				return false;
			}
			else if( result == XGC::DB::sql_result::sql_empty )
			{
				// 插入一条
				sprintf_s( sql, sizeof( sql ), "INSERT INTO sequenceid (pipeindex, sid) VALUES (%u, %I64u)", xgc_uint32( gIndex ), xgc_uint64( gSIDSize ) );
				if( false == SyncDBExecute( sql ) )
				{
					SYS_ERROR( "Insert sid 失败:%s", SyncDBErrorInfo() );
					return false;
				}
			}
			else
			{
				if( movenext( rs ) )
				{
					nId = field_unsigned( rs, 0, 0 );
					nSid = field_ulonglong( rs, 1, 0 );

					// 新的sid是 nSid + gSidSize				
					sprintf_s( sql, sizeof( sql ), "UPDATE sequenceid set sid = sid + %u WHERE id = %u", gSIDSize, nId );
					if( false == SyncDBExecute( sql ) )
					{
						SYS_ERROR( "Update sid 失败:%s", SyncDBErrorInfo() );
						return false;
					}
				}
				else
				{
					SYS_ERROR( "获取 sid 失败" );
					return false;
				}
			}
			// 赋值起始位置
			gSIDPos = nSid;
			gSIDMax = gSIDPos + gSIDSize;
			gKey = nId;
		}

		if( ( gServerCode > 0xFFFF ) || ( gIndex > 0xFF ) )
		{
			SYS_ERROR( "gServerCode [%u], gIndex [%u] 某个值超过最大值", gServerCode, gIndex );
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
			SYS_ERROR( "还没有初始化Sequence吧" );
			return 0;
		}

		auto FnCalcSID = []()->xgc_uint64
		{
			// 结构：16位区服 8位进程ID 40位SequenceID
			xgc_uint64 _sid = 0;
			_sid = ( ( ( xgc_uint64( gServerCode ) & 0xFFFF ) << 48 ) | ( ( xgc_uint64( gIndex ) & 0xFF ) << 40 ) );

			if( gSIDPos > 0xFFFFFFFFFF )
			{
				SYS_ERROR( "SID-sequence出错了，超过了最大值" );
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
			// 重新获取一下 gSIDPos, gSIDMax

			// 新的sid是 nSid + gSidSize		
			xgc_char sql[1024] = { 0 };
			sprintf_s( sql, sizeof( sql ), "UPDATE sequenceid set sid = sid + %u WHERE id = %u", gSIDSize, gKey );
			if( false == SyncDBExecute( sql ) )
			{
				SYS_ERROR( "Update sid 失败:%s", SyncDBErrorInfo() );
				return 0;
			}
			// 赋值起始位置
			gSIDPos = gSIDMax;
			gSIDMax = gSIDPos + gSIDSize;

			return FnCalcSID();
		}
		
		return 0;
		FUNCTION_END;
		return 0;
	}
};