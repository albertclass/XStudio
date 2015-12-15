#pragma once
#ifndef _BASE_SESSION_H_
#define _BASE_SESSION_H_
class CBaseSession : public SGDP::ISDSession
{
protected:
	/// @var 连接对象
	SGDP::ISDConnection*	mpConnection;
	/// @var 是否连接
	xgc_bool				mConnected;
	/// @var 用户数据
	xgc_lpvoid				mUserdata;

public:
	CBaseSession()
		: mpConnection( xgc_nullptr )
		, mConnected( false )
		, mUserdata( xgc_nullptr )
	{

	}

	~CBaseSession()
	{
		mpConnection = xgc_nullptr;
		mConnected   = false;
		mUserdata    = xgc_nullptr;
	}

	///
	/// 断开服务器连接
	/// [6/28/2015] create by albert.xu
	///
	xgc_bool Disconnect()
	{
		if( !mpConnection )
			return false;

		mpConnection->Disconnect();
		return true;
	}

	///
	/// 获取远端地址
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint32 GetRemoteAddr()
	{
		return mpConnection->GetRemoteIP();
	}

	///
	/// 获取远端端口
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint16 GetRemotePort()
	{
		return mpConnection->GetRemotePort();
	}

	///
	/// 获取远端地址
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint32 GetLocalAddr()
	{
		return mpConnection->GetLocalIP();
	}

	///
	/// 获取远端端口
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint16 GetLocalPort()
	{
		return mpConnection->GetLocalPort();
	}

	///
	/// 设置该连接上的用户数据
	/// [12/1/2014] create by albert.xu
	///
	xgc_void SetUserdata( xgc_lpvoid pUserdata )
	{
		mUserdata = pUserdata;
	}

	///
	/// 获取该连接上的用户数据
	/// [12/1/2014] create by albert.xu
	///
	xgc_lpvoid GetUserdata()const
	{
		return mUserdata;
	}
};
#endif //_BASE