#include "stdafx.h"
#include "PipeSession.h"

#include "ServerBase.h"
#include "ServerConsole.h"
#include "ServerMonitor.h"
#include "ServerBase.h"

using namespace SGDP;
using namespace XGC::common;

extern ISDPipeModule *gpPipeModule;

//-----------------------------------//
// [3/2/2014 albert.xu]
// 根据服务器类型，查表取事物索引号。
//-----------------------------------//
static xgc_uint32 ServerType2BusinessID( ServerType eRemoteType )
{
	ServerType eLocalType = (ServerType) GetPipeType( GetServerCode( xgc_nullptr ) );
	switch( eLocalType )
	{
		case ServerType::SD_GAME_SERVER:
		switch( eRemoteType )
		{
			case ServerType::SD_DB_SERVER:
			return DB_GS_BUSINESSID;
			case ServerType::SD_MSERVER:
			return MS_GS_BUSINESSID;
			case ServerType::SD_MONITOR_SERVER:
			return MT_GS_BUSINESSID;
			default:
			XGC_DEBUG_MESSAGE( "监测到非法的管道连接，请联系开发团队确认该问题。local %u - remote %u", eLocalType, eRemoteType );
			break;
		}
		break;
		case ServerType::SD_DB_SERVER:
		switch( eRemoteType )
		{
			case ServerType::SD_MSERVER:
			return DB_MS_BUSINESSID;
			case ServerType::SD_GATE_SERVER:
			return DB_GG_BUSINESSID;
			case ServerType::SD_GAME_SERVER:
			return DB_GS_BUSINESSID;
			case ServerType::SD_LOGIN_SERVER:
			return DB_LS_BUSINESSID;
			case ServerType::SD_MONITOR_SERVER:
			return MT_DB_BUSINESSID;
			default:
			XGC_DEBUG_MESSAGE( "监测到非法的管道连接，请联系开发团队确认该问题。local %u - remote %u", eLocalType, eRemoteType );
			break;
		}
		case ServerType::SD_MSERVER:
		switch( eRemoteType )
		{
			case ServerType::SD_DB_SERVER:
			return DB_MS_BUSINESSID;
			case ServerType::SD_GAME_SERVER:
			return MS_GS_BUSINESSID;
			case ServerType::SD_MONITOR_SERVER:
			return MT_MS_BUSINESSID;
			default:
			XGC_DEBUG_MESSAGE( "监测到非法的管道连接，请联系开发团队确认该问题。local %u - remote %u", eLocalType, eRemoteType );
			break;
		}
		break;
	}

	return ( ~0 );
}

CPipeSession::CPipeSession( ISDPipe *pPipe, xgc_uint32 nBusinessID, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler )
: mPipe( pPipe )
, mBusinessID( nBusinessID )
, mMsgHandler( fnMsgHandler )
, mEvtHandler( fnEvtHandler )
{
	FUNCTION_BEGIN;

	mConnected = false;

	FUNCTION_END;
}

CPipeSession::~CPipeSession()
{
	FUNCTION_BEGIN;

	mConnected = false;
	mMsgHandler = xgc_nullptr;
	mEvtHandler = xgc_nullptr;
	FUNCTION_END;
}

void SDAPI CPipeSession::OnRecv( UINT16 wBusinessID, const char* pData, xgc_uint32 dwLen )
{
	FUNCTION_BEGIN;
	XGC_ASSERT_RETURN( mMsgHandler, xgc_void( 0 ) );
	xgc_lpcvoid	pCursor = (xgc_lpcvoid) pData;
	xgc_size	nRelease = dwLen;

	// 获取数据包长度
	PkgHeader pkgHeader = buffer_cast<PkgHeader>( pCursor, nRelease, &pCursor, &nRelease );
	pkgHeader.uLength = ntohs( pkgHeader.uLength );
	XGC_ASSERT_RETURN( pkgHeader.uLength == dwLen - TOTAL_HEADER_LEN, xgc_void( 0 ), "消息接收长度与数据包描述不符。" );

	// 获取数据包报头
	MsgHeader msgHeader = buffer_cast<MsgHeader>( pCursor, nRelease, &pCursor, &nRelease );
	msgHeader.uClass	= ntohs( msgHeader.uClass );
	msgHeader.uMessage	= ntohs( msgHeader.uMessage );
	msgHeader.uReserved = ntohl( msgHeader.uReserved );

	mMsgHandler( this, msgHeader.uMessage, (xgc_lpstr) pCursor, nRelease, msgHeader.uReserved );
	FUNCTION_END;
}

void SDAPI CPipeSession::OnReport( UINT16 wBusinessID, INT32 nErrCode )
{
	FUNCTION_BEGIN;
	if( nErrCode == PIPE_DISCONNECT )
	{
		mConnected = false;
		mEvtHandler( this, SESSION_EVENT_DISCONNECT, nErrCode );
		
		// Monitor
		MonitorConsoleConnect( mPipeID, mConnected );
	}
	else if( nErrCode == PIPE_SUCCESS )
	{
		mConnected = true;
		ServerCode2String( mPipe->GetID(), mPipeID, sizeof( mPipeID ) );
		mEvtHandler( this, SESSION_EVENT_CONNECTED, nErrCode );

		// Monitor
		MonitorConsoleConnect( mPipeID, mConnected );
	}
	else
	{
		mEvtHandler( this, SESSION_EVENT_ERROR, nErrCode );
	}
	FUNCTION_END;
}

xgc_bool CPipeSession::Send( xgc_lpcvoid pData, xgc_size uLength )
{
	FUNCTION_BEGIN;
	if( mConnected == false )
		return false;

	if( mPipe == xgc_nullptr )
		return false;

	return mPipe->Send( mBusinessID, (xgc_lpcstr) pData, (UINT32) uLength );

	FUNCTION_END;

	return false;
}

//////////////////////////////////////////////////////////////////////////
CPipeReporter::CPipeReporter()
{
}

CPipeReporter::~CPipeReporter()
{
	for( auto& it : mPipeIDMap )
	{
		if( it.second )
		{
			SAFE_DELETE( it.second );
		}
	}
	mPipeIDMap.clear();
	mDispatcherMap.clear();
}

CPipeSession* CPipeReporter::CreatePipeSession( ISDPipe* pPipe, ServerType nServerType )
{
	CPipeSession* pSession = xgc_nullptr;
	if( xgc_nullptr == pPipe )
	{
		SYS_WARNING( " Pipe is null" );
		return pSession;
	}

	auto it = mDispatcherMap.find( (ServerType) nServerType );
	if( it != mDispatcherMap.end() )
	{
		pSession = XGC_NEW CPipeSession( pPipe, 
			ServerType2BusinessID( nServerType ), 
			std::get< 0 >( it->second ), 
			std::get< 1 >( it->second ) );

		pPipe->SetSink( pSession->GetBusinessID(), pSession );

		return pSession;
	}

	return xgc_nullptr;
}

void SDAPI CPipeReporter::OnReport( INT32 nErrCode, xgc_uint32 dwServerID )
{
	FUNCTION_BEGIN;
	SSDPipeId &ServerID = reinterpret_cast<SSDPipeId&>( dwServerID );

	ISDPipe * pPipe = xgc_nullptr;
	CPipeSession * pSession = xgc_nullptr;
	xgc_char szCode[32];
	ServerCode2String( dwServerID, szCode, sizeof( szCode ) );

	switch( nErrCode )
	{
		case PIPE_SUCCESS:
		{
			pPipe = gpPipeModule->GetPipe( dwServerID );
			if( xgc_nullptr == pPipe )
			{
				SYS_WARNING( "说连接成功了，又取不到管道。难道连上就断？" );
				return;
			}

			pSession = CreatePipeSession( pPipe, (ServerType) ServerID.byType );
			if( xgc_nullptr == pSession )
			{
				SYS_WARNING( "服务器类型未知？从哪儿连过来的？ ServerType = %d", ServerID.byType );
				return;
			}

			mPipeIDMap.insert( std::make_pair( dwServerID, pSession ) );
			SYS_INFO( "PIPE 0X%08X(%s) Report Code=%d connected successful.", dwServerID, szCode, nErrCode );
		}
		break;
		case PIPE_DISCONNECT:
		{
			pPipe = gpPipeModule->GetPipe( dwServerID );
			if( xgc_nullptr == pPipe )
			{
				SYS_WARNING( "说连接断开了，又取不到管道。 ServerType = %d", ServerID.byType );
				return;
			}
			xgc_uint32 busnessid = ServerType2BusinessID( (ServerType) ServerID.byType );
			pSession = static_cast<CPipeSession*>( pPipe->GetSink( busnessid ) );
			if( xgc_nullptr == pSession )
			{
				SYS_WARNING( "Pipe上找不到断开的Session？ ServerType = %d", ServerID.byType );
				return;
			}

			pPipe->SetSink( busnessid, xgc_nullptr );
			mPipeIDMap.erase( dwServerID );
			SAFE_DELETE( pSession );

			SYS_INFO( "PIPE 0X%08X(%s) Report Code=%d connected failed.", dwServerID, szCode, nErrCode );
		}
		break;
	}

	FUNCTION_END;
}

CPipeSession* CPipeReporter::GetSession( ServerType eServerType, xgc_byte byIndex )
{
	FUNCTION_BEGIN;
	xgc_uint32 nPipeID = GetServerCode( &byIndex, (xgc_byte*) &eServerType );

	if( gpPipeModule )
	{
		ISDPipe* pPipe = gpPipeModule->GetPipe( nPipeID );
		if( xgc_nullptr != pPipe )
			return static_cast<CPipeSession*>( pPipe->GetSink( ServerType2BusinessID( eServerType ) ) );
		else
			SYS_WARNING( "找不到请求的管道[0x%08x]", nPipeID );
	}

	FUNCTION_END;
	return xgc_nullptr;
}

void CPipeReporter::GetPipeSession(ServerType eType, vector<CPipeSession*>& vec)
{
    for (auto pipeItem: mPipeIDMap)
    {
        if (pipeItem.second)
        {
            if (eType == static_cast<ServerType>(GetPipeType(pipeItem.second->GetPipeID())))
            {
                vec.push_back(pipeItem.second);
            }
        }
    }
}

///
/// [2/20/2014 albert.xu]
/// 根据服务器类型获取服务器连接对象
///
CPipeSession* CPipeReporter::GetSession( xgc_uint32 nPipeID )
{
	FUNCTION_BEGIN;
	if( gpPipeModule )
	{
		ISDPipe* pPipe = gpPipeModule->GetPipe( nPipeID );
		if( xgc_nullptr != pPipe )
			return static_cast<CPipeSession*>( 
				pPipe->GetSink( ServerType2BusinessID( (ServerType) GetPipeType( nPipeID ) ) ) );
		else
			SYS_WARNING( "找不到请求的管道[0x%08x]", nPipeID );
	}

	FUNCTION_END;
	return xgc_nullptr;
}

///
/// 根据过滤条件获取服务器连接对象
/// [2/20/2014 albert.xu]
///
CPipeSession* CPipeReporter::GetSession( const std::function< xgc_bool( xgc_uint32 ) > &fnFilter )
{
	FUNCTION_BEGIN;
	for each( auto& it in mPipeIDMap )
	{
		if( fnFilter( it.first ) )
			return GetSession( it.first );
	}

	FUNCTION_END;
	return xgc_nullptr;
}

xgc_void CPipeReporter::RegistDispatcher( ServerType eServerType, const xgc_tuple< PipeMsgHandler, PipeEvtHandler > &Handler )
{
	mDispatcherMap[eServerType] = Handler;
}

CPipeReporter& getPipeReporter()
{
	static CPipeReporter Inst;
	return Inst;
}

///
/// 注册管道消息处理函数
/// [11/27/2014] create by albert.xu
///
xgc_void RegistPipeHandler( ServerType eType, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler )
{
	getPipeReporter().RegistDispatcher( eType, std::make_tuple( fnMsgHandler, fnEvtHandler ) );
}

///
/// 获取PipeSession
/// [12/1/2014] create by albert.xu
///
CPipeSession* GetPipeSession( ServerType eType, xgc_byte byIndex /*= 1*/ )
{
	return getPipeReporter().GetSession( eType, byIndex );
}

void GetPipeSession(ServerType eType, vector<CPipeSession*>& vec)
{
    return getPipeReporter().GetPipeSession(eType, vec);
}

///
/// 获取PipeSession
/// [12/9/2014] create by albert.xu
///
CPipeSession* GetPipeSession( xgc_uint32 nPipeID )
{
	return getPipeReporter().GetSession( nPipeID );
}

///
/// 根据过滤条件获取服务器连接对象
/// [2/20/2014 albert.xu]
///
CPipeSession* GetPipeSession( const std::function< xgc_bool( xgc_uint32 ) > &fnFilter )
{
	return getPipeReporter().GetSession( fnFilter );
}

///
/// 获取PipeSession的PipeID
/// [12/9/2014] create by albert.xu
///
xgc_uint32 GetPipeID( CPipeSession* pSession )
{
	return pSession->GetPipeID();
}

///
/// 获取PipeSession的PipeID（字符串形式）
/// [12/11/2014] create by albert.xu
///
xgc_lpcstr GetPipeID( CPipeSession* pSession, xgc_lpstr lpOutput, xgc_size nSize )
{
	return pSession->GetPipeID( lpOutput, nSize );
}
