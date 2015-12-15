#include "stdafx.h"
#include "ServerDefines.h"
#include "GameSession.h"
#include "ServerConsole.h"
#include "ServerBase.h"

using namespace XGC;
using namespace XGC::common;
using namespace SGDP;

static GameMsgHandler MsgHandler = xgc_nullptr;
static GameEvtHandler EvtHandler = xgc_nullptr;

//////////////////////////////////////////////////////////////////////////
CGameSession::CGameSession()
	: CBaseSession()
	, mTransID( -1 )
	, mGateID( -1 )
{
}

CGameSession::~CGameSession()
{
}

xgc_void SDAPI CGameSession::SetConnection( ISDConnection* poConnection )
{
	mpConnection = poConnection;
}

xgc_void SDAPI CGameSession::OnEstablish( xgc_void )
{
	USR_WARNING( "GG session Connect %p", this );
	mConnected = true;
	EvtHandler( this, SESSION_EVENT_CONNECTED, 0 );
}

xgc_void SDAPI CGameSession::OnTerminate( xgc_void )
{
	USR_WARNING( "GG session Disconnect GGID - %08x, %p - %u", mGateID, this, mTransID );
	mConnected = false;
	EvtHandler( this, SESSION_EVENT_DISCONNECT, 0 );
}

xgc_bool SDAPI CGameSession::OnError( xgc_int32 nModuleErr, xgc_int32 nSysErr )
{
	USR_WARNING( "GG session Error. GGID - %08x, %p - %u nModuleErr=%d nSysErr=%d"
		, mGateID
		, this
		, mTransID
		, nModuleErr
		, nSysErr );

	EvtHandler( this, SESSION_EVENT_ERROR, nModuleErr );
	return true;
}

xgc_void SDAPI CGameSession::OnRecv( xgc_lpcstr pBuf, xgc_uint32 dwLen )
{
	FUNCTION_BEGIN;

	xgc_lpcvoid	pCursor = (xgc_lpcvoid)pBuf;
	xgc_size	nRelease = dwLen;

	// 获取数据包长度
	PkgHeader pkgHeader = buffer_cast<PkgHeader>( pCursor, nRelease, &pCursor, &nRelease );
	pkgHeader.uLength   = ntohs( pkgHeader.uLength );

	XGC_ASSERT_RETURN( pkgHeader.uLength == dwLen - TOTAL_HEADER_LEN, xgc_void(0), "消息接收长度与数据包描述不符。" );

	// 获取数据包报头
	MsgHeader msgHeader = buffer_cast<MsgHeader>( pCursor, nRelease, &pCursor, &nRelease );
	msgHeader.uClass    = ntohs( msgHeader.uClass );
	msgHeader.uMessage  = ntohs( msgHeader.uMessage );
	msgHeader.uReserved = ntohl( msgHeader.uReserved );

	MsgHandler( this, msgHeader.uMessage, (xgc_lpcstr) pCursor, nRelease, msgHeader.uReserved );
	FUNCTION_END;
}

xgc_void SDAPI CGameSession::Release( xgc_void )
{
	delete this;
}

xgc_bool SDAPI CGameSession::Send( const xgc_char* pData, xgc_uint32 nLen )
{
	FUNCTION_BEGIN;
	if( mConnected == false )
		return false;

	if( mpConnection == xgc_nullptr )
		return false;

	mpConnection->Send( (xgc_lpcstr) pData, (UINT32) nLen );
	return true;
	FUNCTION_END;

	return false;
}

xgc_void CGameSession::SetGateID( xgc_uint32 nGateID )
{
	FUNCTION_BEGIN;
	xgc_char szGateID[64] = { 0 };
	USR_INFO( "GG session %p set TransID - %s", this, ServerCode2String( nGateID, szGateID, sizeof( szGateID ) ) );
	mGateID = nGateID;
	FUNCTION_END;
}

xgc_void CGameSession::SetTransID( xgc_uint32 nTransID )
{
	FUNCTION_BEGIN;
	USR_INFO( "GG session %p set TransID - %u", this, nTransID );
	mTransID = nTransID;
	FUNCTION_END;
}

//////////////////////////////////////////////////////////////////////////
// GateSessionFactory
//////////////////////////////////////////////////////////////////////////
CGameSessionFactory::CGameSessionFactory()
{
}

CGameSessionFactory::~CGameSessionFactory()
{
}

ISDSession* SDAPI CGameSessionFactory::CreateSession( ISDConnection* poConnection )
{
	FUNCTION_BEGIN;
	ISDSession* pSession = XGC_NEW CGameSession();
	XGC_ASSERT_MESSAGE( pSession, "Session分配失败" );

	return pSession;

	FUNCTION_END;
	return xgc_nullptr;
}

CGameSessionFactory& getGameSessionFactory()
{
	static CGameSessionFactory Instance;
	return Instance;
}


xgc_void RegistGameHandler( GameMsgHandler fnMsgHandler, GameEvtHandler fnEvtHandler )
{
	MsgHandler = fnMsgHandler;
	EvtHandler = fnEvtHandler;
}

///
/// 断开连接
/// [12/1/2014] create by albert.xu
///
xgc_void Disconnect( CGameSession* pSession )
{
	if( pSession )
		pSession->Disconnect();
}

///
/// 设置用户数据
/// [12/1/2014] create by albert.xu
///
xgc_void SetUserdata( CGameSession* pSession, xgc_lpvoid pUserdata )
{
	pSession->SetUserdata( pUserdata );
}

///
/// 获取用户数据
/// [12/1/2014] create by albert.xu
///
xgc_lpvoid GetUserdata( CGameSession* pSession )
{
	return pSession ? pSession->GetUserdata() : xgc_nullptr;
}

xgc_uint32 GetRemoteAddr( CGameSession* pSession )
{
	return pSession->GetRemoteAddr();
}

xgc_uint16 GetRemotePort( CGameSession* pSession )
{
	return pSession->GetRemotePort();
}

xgc_uint32 GetLocalAddr( CGameSession* pSession )
{
	return pSession->GetLocalAddr();
}

xgc_uint16 GetLocalPort( CGameSession* pSession )
{
	return pSession->GetLocalPort();
}

xgc_uint32 GetGateID( CGameSession* pSession )
{
	return pSession->GetGateID();
}

xgc_uint32 GetTransID( CGameSession* pSession )
{
	return pSession->GetTransID();
}

xgc_void SetTransID( CGameSession* pSession, xgc_uint32 nTransID )
{
	pSession->SetTransID( nTransID );
}

xgc_void SetGateID( CGameSession* pSession, xgc_uint32 nGateID )
{
	pSession->SetGateID( nGateID );
}