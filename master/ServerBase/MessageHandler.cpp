#include "stdafx.h"
#include "MessageHandler.h"
#include "PipeSession.h"
#include "GameSession.h"
#include "GateSession.h"

namespace Network
{
	char g_BeginSafeBufer[SAFE_BUFFER_SIZE];	// 安全隔离空间
	HEAP_ALLOC( wrkmem, LZO1X_1_MEM_COMPRESS );
	char g_EndSafeBufer[SAFE_BUFFER_SIZE];	// 安全隔离空间

	xgc_bool SortMessageHandler( MessageHandler* pFirst, MessageHandler* pLast )
	{
		std::sort( pFirst, pLast );	return true;
	}

	xgc_void FreeMessageBuffer()
	{
		FUNCTION_BEGIN;
		free( gpNetBuffer );
		FUNCTION_END;
	}

	xgc_bool SendPackage( CPipeSession* pSession, xgc_lpcstr pData )
	{
		if( !pSession )
		{
			USR_WARNING( "Send msg failed, session is null" );
			return false;
		}
		XGC_SINGLE_THREAD_CHECKER( "PipeSession 0x%08x", pSession->GetPipeID() );
		PkgHeader* pHeader = (PkgHeader*) pData;
		return pSession->Send( pData, ntohs( ( (PkgHeader*) pData )->uLength ) + TOTAL_HEADER_LEN );
	}

	xgc_bool SendPackage( CGameSession* pSession, xgc_lpcstr pData )
	{
		if( !pSession )
		{
			USR_WARNING( "Send msg failed, session is null" );
			return false;
		}
		XGC_SINGLE_THREAD_CHECKER( "GameSession gate=%d, trans=%d", pSession->GetGateID(), pSession->GetTransID() );
		PkgHeader* pHeader = (PkgHeader*) pData;
		return pSession->Send( pData, ntohs( ( (PkgHeader*) pData )->uLength ) + TOTAL_HEADER_LEN );
	}

	xgc_bool SendPackage( CGateSession* pSession, xgc_lpcstr pData )
	{
		if( !pSession )
		{
			USR_WARNING( "Send msg failed, session is null" );
			return false;
		}
		XGC_SINGLE_THREAD_CHECKER( "GateSession gate=%d", pSession->GetIndex() );
		PkgHeader* pHeader = (PkgHeader*) pData;
		return pSession->Send( pData, ntohs( ( (PkgHeader*) pData )->uLength ) + TOTAL_HEADER_LEN );
	}

}