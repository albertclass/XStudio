#pragma once
#ifndef _PIPE_REPORTER_H_
#define _PIPE_REPORTER_H_

#include "ServerDefines.h"

class CPipeSession : public SGDP::ISDPipeSink
{
	friend class CPipeReporter;

private:
	SGDP::ISDPipe * mPipe;
	PipeMsgHandler	mMsgHandler;
	PipeEvtHandler	mEvtHandler;

	// ���ӱ�־
	xgc_bool		mConnected;
	xgc_uint32		mBusinessID;

	xgc_char		mPipeID[32];
public:
	///
	/// [2/20/2014 albert.xu]
	/// ������Ϣ
	///
	xgc_bool Send( xgc_lpcvoid pData, xgc_size uLength );

	///
	/// [5/16/2014 guqiwei.weir]
	///  ��ѯ������ business ID 
	///
	xgc_uint32 GetBusinessID() 
	{ 
		return mBusinessID; 
	}

	///
	/// ��ȡ�ܵ�ID
	/// [12/9/2014] create by albert.xu
	///
	xgc_uint32 GetPipeID()
	{
		return mPipe->GetID();
	}

	///
	/// ��ȡ�ܵ�ID���ַ�����ʽ��
	/// [12/9/2014] create by albert.xu
	///
	xgc_lpcstr GetPipeID( xgc_lpstr lpOutput, xgc_size nSize )
	{
		if( lpOutput && nSize > 1 )
		{
			strncpy_s( lpOutput, nSize - 1, mPipeID, sizeof( mPipeID ) );
			return lpOutput;
		}
		return mPipeID;
	}

protected:
	CPipeSession( SGDP::ISDPipe *pPipe, xgc_uint32 nBusinessID, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );
	~CPipeSession();

private:
	//
	// Name     : OnRecv
	// Function : Data arriving callback.
	//
	virtual void SDAPI OnRecv( UINT16 wBusinessID, const char* pData, xgc_uint32 dwLen );

	//
	// Name     : OnReport
	// Function : Pipe status changed callback.
	//
	virtual void SDAPI OnReport( UINT16 wBusinessID, INT32 nErrCode );
};

class CPipeReporter : public SGDP::ISDPipeReporter
{
	friend CPipeReporter& getPipeReporter();
protected:
	CPipeReporter();
	~CPipeReporter();

	CPipeSession* CreatePipeSession( SGDP::ISDPipe* pPipe, ServerType nServerType );
public:
	///
	/// ע��ܵ���������
	/// [11/26/2014] create by albert.xu
	///
	xgc_void RegistDispatcher( ServerType eServerType, const xgc_tuple< PipeMsgHandler, PipeEvtHandler > &Handler );

	///
	/// ���ݷ��������ͻ�ȡ���������Ӷ���
	/// [2/20/2014 albert.xu]
	///
	CPipeSession* GetSession( ServerType eServerType, xgc_byte byIndex = 1 );

	///
	/// ���ݷ��������ͻ�ȡ���������Ӷ���
	/// [2/20/2014 albert.xu]
	///
	CPipeSession* GetSession( xgc_uint32 nPipeID );
    void GetPipeSession(ServerType eType, vector<CPipeSession*>&);

	///
	/// ���ݹ���������ȡ���������Ӷ���
	/// [2/20/2014 albert.xu]
	///
	CPipeSession* GetSession( const std::function< xgc_bool( xgc_uint32 ) > &fnFilter );

private:
	//
	// Name     : OnReport
	// Function : Pipe status changed callback.
	//
	virtual void SDAPI OnReport( INT32 nErrCode, xgc_uint32 dwID );

private:
	xgc_unordered_map< ServerType, xgc_tuple< PipeMsgHandler, PipeEvtHandler > > mDispatcherMap;
	xgc_unordered_map< xgc_uint32, CPipeSession* > mPipeIDMap;
};

CPipeReporter& getPipeReporter();
#endif // _PIPE_REPORTER_H_