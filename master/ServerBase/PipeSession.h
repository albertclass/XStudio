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

	// 连接标志
	xgc_bool		mConnected;
	xgc_uint32		mBusinessID;

	xgc_char		mPipeID[32];
public:
	///
	/// [2/20/2014 albert.xu]
	/// 发送消息
	///
	xgc_bool Send( xgc_lpcvoid pData, xgc_size uLength );

	///
	/// [5/16/2014 guqiwei.weir]
	///  查询和设置 business ID 
	///
	xgc_uint32 GetBusinessID() 
	{ 
		return mBusinessID; 
	}

	///
	/// 获取管道ID
	/// [12/9/2014] create by albert.xu
	///
	xgc_uint32 GetPipeID()
	{
		return mPipe->GetID();
	}

	///
	/// 获取管道ID（字符串形式）
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
	/// 注册管道创建函数
	/// [11/26/2014] create by albert.xu
	///
	xgc_void RegistDispatcher( ServerType eServerType, const xgc_tuple< PipeMsgHandler, PipeEvtHandler > &Handler );

	///
	/// 根据服务器类型获取服务器连接对象
	/// [2/20/2014 albert.xu]
	///
	CPipeSession* GetSession( ServerType eServerType, xgc_byte byIndex = 1 );

	///
	/// 根据服务器类型获取服务器连接对象
	/// [2/20/2014 albert.xu]
	///
	CPipeSession* GetSession( xgc_uint32 nPipeID );
    void GetPipeSession(ServerType eType, vector<CPipeSession*>&);

	///
	/// 根据过滤条件获取服务器连接对象
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