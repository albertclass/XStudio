#pragma once
#ifndef _GAME_SESSION_H_
#define _GAME_SESSION_H_
#include "BaseSession.h"

class CGameSession : public CBaseSession
{
	friend class CGameSessionFactory;
protected:
	CGameSession();
	virtual ~CGameSession();

protected:
	virtual xgc_void SDAPI SetConnection( SGDP::ISDConnection* poConnection );
	virtual xgc_void SDAPI OnEstablish( xgc_void );
	virtual xgc_void SDAPI OnTerminate( xgc_void );
	virtual xgc_bool SDAPI OnError( xgc_int32 nModuleErr, xgc_int32 nSysErr );
	virtual xgc_void SDAPI OnRecv( xgc_lpcstr pBuf, xgc_uint32 dwLen );
	virtual xgc_void SDAPI Release( xgc_void );

public:
	virtual xgc_bool SDAPI Send( const xgc_char* pData, xgc_uint32 nLen );

	///
	/// ���ûỰ��ʶ
	/// [7/4/2015] create by albert.xu
	///
	xgc_void SetTransID( xgc_uint32 nTransID );

	///
	/// ��ȡ�Ự��ʶ
	/// [7/4/2015] create by albert.xu
	///
	xgc_uint32 GetTransID()const
	{
		return mTransID;
	}

	///
	/// ����Gate��������ʶ
	/// [7/4/2015] create by albert.xu
	///
	xgc_void SetGateID( xgc_uint32 nGateID );

	///
	/// ��ȡGate��������ʶ
	/// [7/4/2015] create by albert.xu
	///
	xgc_uint32 GetGateID()const
	{
		return mGateID;
	}

protected:
	/// @var ����ID
	xgc_uint32				mTransID;
	/// @var GateID
	xgc_uint32				mGateID;
};

class CGameSessionFactory : public SGDP::ISDSessionFactory
{
	friend CGameSessionFactory& getGameSessionFactory();
	friend class CGameSession;
protected:
	CGameSessionFactory();
	virtual ~CGameSessionFactory();

public:
	//
	// Name     : CreateSession
	// Function : Create an user implemented ISDSession object.
	//
	virtual SGDP::ISDSession* SDAPI CreateSession( SGDP::ISDConnection* poConnection );
};

CGameSessionFactory& getGameSessionFactory();
#endif // _CLIENT_SESSION_H_