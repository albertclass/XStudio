#include "stdafx.h"
#include "GateSession.h"
#include "ServerDefines.h"
#include "ServerConsole.h"
#include "ServerMonitor.h"
#include "ServerBase.h"
using namespace XGC;
using namespace XGC::common;
using namespace SGDP;

///
/// Gate消息分发函数
/// [11/27/2014] create by albert.xu
///
static GateMsgHandler MsgHandler = xgc_nullptr;
static GateEvtHandler EvtHandler = xgc_nullptr;

//////////////////////////////////////////////////////////////////////////
CGateSession::CGateSession()
: CBaseSession()
{
}

CGateSession::~CGateSession()
{
	getGateSessionFactory().CleanSession( mGateID );
}

xgc_void SDAPI CGateSession::SetConnection( ISDConnection* poConnection )
{
	mpConnection = poConnection;
}

xgc_void SDAPI CGateSession::OnEstablish( xgc_void )
{
	SYS_INFO( "GameGate Socket Established %s:%d:%d",
		mpConnection->GetRemoteIPStr(), 
		mpConnection->GetRemotePort(),
		mpConnection->GetLocalPort() );

	mConnected = true;
	EvtHandler( this, SESSION_EVENT_CONNECTED, 0 );

	// Monitor
	MonitorConsoleIPConnect( mpConnection->GetRemoteIPStr(), mpConnection->GetRemotePort(), mConnected );
}

xgc_void SDAPI CGateSession::OnTerminate( xgc_void )
{
	SYS_INFO( "GameGate disconnected. " );
	mConnected = false;
	EvtHandler( this, SESSION_EVENT_DISCONNECT, 0 );

	// Monitor
	MonitorConsoleIPConnect( mpConnection->GetRemoteIPStr(), mpConnection->GetRemotePort(), mConnected );
}

xgc_bool SDAPI CGateSession::OnError( xgc_int32 nModuleErr, xgc_int32 nSysErr )
{
	USR_WARNING( "GG session Error.nModuleErr = %d nSysErr = %d", nModuleErr, nSysErr );
	EvtHandler( this, SESSION_EVENT_ERROR, nModuleErr );
	return true;
}

xgc_void SDAPI CGateSession::OnRecv( xgc_lpcstr pBuf, xgc_uint32 dwLen )
{
	FUNCTION_BEGIN;

	xgc_lpcvoid	pCursor = (xgc_lpcvoid)pBuf;
	xgc_size	nRelease = dwLen;

	// 获取数据包长度
	PkgHeader pkgHeader = buffer_cast<PkgHeader>( pCursor, nRelease, &pCursor, &nRelease );
	pkgHeader.uLength	= ntohs( pkgHeader.uLength );
	XGC_ASSERT_RETURN( pkgHeader.uLength == dwLen - TOTAL_HEADER_LEN, xgc_void( 0 ), "消息接收长度与数据包描述不符。" );
	// 获取数据包报头
	MsgHeader msgHeader = buffer_cast< MsgHeader >( pCursor, nRelease, &pCursor, &nRelease );
	msgHeader.uClass	= ntohs( msgHeader.uClass );
	msgHeader.uMessage	= ntohs( msgHeader.uMessage );
	msgHeader.uReserved = ntohl( msgHeader.uReserved );

	MsgHandler( this, msgHeader.uMessage, (xgc_lpcstr) pCursor, nRelease, msgHeader.uReserved );
	FUNCTION_END;
}

xgc_void SDAPI CGateSession::Release( xgc_void )
{
	delete this;
}

xgc_void CGateSession::SetIndex( xgc_uint32 nIndex )
{
	mGateID = nIndex;
	ServerCode2String( mGateID, mPipeID, sizeof( mPipeID ) );
	MonitorConsoleIPPortToID( mpConnection->GetRemoteIPStr(), mpConnection->GetRemotePort(), mPipeID );
}

xgc_uint32 CGateSession::GetIndex() const
{
	return mGateID;
}

xgc_bool CGateSession::Send( xgc_lpcvoid pData, xgc_size uLength )
{
	FUNCTION_BEGIN;
	if( mConnected == false )
		return false;

	if( mpConnection == xgc_nullptr )
		return false;

	mpConnection->Send( (xgc_lpcstr)pData, (UINT32)uLength );
	FUNCTION_END;

	return false;
}

//////////////////////////////////////////////////////////////////////////
// GateSessionFactory
//////////////////////////////////////////////////////////////////////////
CGateSessionFactory::CGateSessionFactory()
{
}

CGateSessionFactory::~CGateSessionFactory()
{
	FUNCTION_BEGIN;
	while( !mGateMgr.empty() )
	{
		// 因为CGateSession的析构会CleanSession，所以才这么写，这里不能用SAFE_DELETE
		auto iter = mGateMgr.begin();
		if(iter->second)
			delete iter->second;
	}
	FUNCTION_END;
}

ISDSession* SDAPI CGateSessionFactory::CreateSession( ISDConnection* poConnection )
{
	// FUNCTION_BEGIN;
	ISDSession* pSession = XGC_NEW CGateSession();
	XGC_ASSERT_MESSAGE( pSession, "Session分配失败" );
	return pSession;
	// FUNCTION_END;
	return xgc_nullptr;
}

CGateSession* CGateSessionFactory::FetchSession( xgc_uint32 nIndex )
{
	FUNCTION_BEGIN;
	auto iter = mGateMgr.find( nIndex );
	if( iter != mGateMgr.end() )
		return iter->second;
	FUNCTION_END;

	return xgc_nullptr;
}

CGateSession* CGateSessionFactory::CleanSession( xgc_uint32 nIndex )
{
	FUNCTION_BEGIN;
	auto iter = mGateMgr.find( nIndex );
	if( iter != mGateMgr.end() )
	{
		CGateSession* pSession = iter->second;
		mGateMgr.erase( iter );

		return pSession;
	}
	FUNCTION_END;

	return xgc_nullptr;
}

xgc_void CGateSessionFactory::SetGateIndex( xgc_uint32 nIndex, CGateSession* pSession )
{
	auto iter = mGateMgr.find( nIndex );
	XGC_ASSERT_MESSAGE( iter == mGateMgr.end(), "重复的Gate索引被设置%08X", nIndex );

	mGateMgr[nIndex] = pSession;
	pSession->SetIndex( nIndex );
}

CGateSessionFactory& getGateSessionFactory()
{
	static CGateSessionFactory FactoryInst;
	return FactoryInst;
}

xgc_void RegistGateHandler( GateMsgHandler fnMsgHandler, GateEvtHandler fnEvtHandler )
{
	MsgHandler = fnMsgHandler;
	EvtHandler = fnEvtHandler;
}

///
/// 设置Gate关联关系
/// [12/1/2014] create by albert.xu
///
xgc_void RegistGateSession( xgc_uint32 nIndex, CGateSession* pSession )
{
	getGateSessionFactory().SetGateIndex( nIndex, pSession );
}

///
/// 获取GateSession
/// [12/1/2014] create by albert.xu
///
CGateSession* GetGateSession( xgc_uint32 nIndex )
{
	return getGateSessionFactory().FetchSession( nIndex );
}

///
/// 断开连接
/// [12/1/2014] create by albert.xu
///
xgc_void Disconnect( CGateSession* pSession )
{
	if( pSession )
		pSession->Disconnect();
}

///
/// 设置用户数据
/// [12/1/2014] create by albert.xu
///
xgc_void SetUserdata( CGateSession* pSession, xgc_lpvoid pUserdata )
{
	pSession->SetUserdata( pUserdata );
}

///
/// 获取用户数据
/// [12/1/2014] create by albert.xu
///
xgc_lpvoid GetUserdata( CGateSession* pSession )
{
	return pSession->GetUserdata();
}

xgc_uint32 GetRemoteAddr( CGateSession* pSession )
{
	return pSession->GetRemoteAddr();
}

xgc_uint16 GetRemotePort( CGateSession* pSession )
{
	return pSession->GetRemotePort();
}

xgc_uint32 GetLocalAddr( CGateSession* pSession )
{
	return pSession->GetLocalAddr();
}

xgc_uint16 GetLocalPort( CGateSession* pSession )
{
	return pSession->GetLocalPort();
}

///
/// 获取Gate的索引编号
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetGateIndex( CGateSession* pSession )
{
	return pSession->GetIndex();
}
