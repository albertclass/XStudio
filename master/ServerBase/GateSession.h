#pragma once
#ifndef _GATE_SESSION_H_
#define _GATE_SESSION_H_
#include "BaseSession.h"

class CGateSession : public CBaseSession
{
private:
	CGateSession( void );
	virtual ~CGateSession( void );
	
	friend class CGateSessionFactory;
protected:
	virtual xgc_void SDAPI SetConnection( SGDP::ISDConnection* poConnection );
	virtual xgc_void SDAPI OnEstablish( xgc_void );
	virtual xgc_void SDAPI OnTerminate( xgc_void );
	virtual xgc_bool SDAPI OnError( xgc_int32 nModuleErr, xgc_int32 nSysErr );
	virtual xgc_void SDAPI OnRecv( xgc_lpcstr pBuf, xgc_uint32 dwLen );
	virtual xgc_void SDAPI Release( xgc_void );

public:
	///
	/// 标记Gate的索引
	/// [2/18/2014 albert.xu]
	///
	xgc_void SetIndex( xgc_uint32 nIndex );

	///
	/// 获取Gate的索引
	/// [3/16/2015] create by albert.xu
	///
	xgc_uint32 GetIndex()const;

	///
	/// 发送消息
	/// [2/19/2014 albert.xu]
	///
	xgc_bool Send( xgc_lpcvoid pData, xgc_size uLength );

protected:
	/// @var GateID
	xgc_uint32				mGateID;
	/// @var pipeID
	xgc_char                mPipeID[32];
};

//////////////////////////////////////////////////////////////////////////
class CGateSessionFactory : SGDP::ISDSessionFactory
{
	friend CGateSessionFactory& getGateSessionFactory();
	xgc_unordered_map< xgc_uint32, CGateSession* > CGateSessionFactory::mGateMgr;

public:
	CGateSession* FetchSession( xgc_uint32 nIndex );
	///
	/// 遍历所有session 
	/// [5/26/2015] create by wuhailin.jerry
	///
	template<class T>
	xgc_void ForEachSession( T &fn )
	{
		for ( auto &iter : mGateMgr)
		{
			if (iter.second)
			{
				fn(iter.second);
			}
		}
	}

	CGateSession* CleanSession( xgc_uint32 nIndex );
	xgc_void SetGateIndex( xgc_uint32 nIndex, CGateSession* pSession );

protected:
	CGateSessionFactory();
	virtual ~CGateSessionFactory();

private:
	//
	// Name     : CreateSession
	// Function : Create an user implemented ISDSession object.
	//
	virtual SGDP::ISDSession* SDAPI CreateSession( SGDP::ISDConnection* poConnection );
};
#endif // _GATE_SESSION_H_