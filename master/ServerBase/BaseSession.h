#pragma once
#ifndef _BASE_SESSION_H_
#define _BASE_SESSION_H_
class CBaseSession : public SGDP::ISDSession
{
protected:
	/// @var ���Ӷ���
	SGDP::ISDConnection*	mpConnection;
	/// @var �Ƿ�����
	xgc_bool				mConnected;
	/// @var �û�����
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
	/// �Ͽ�����������
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
	/// ��ȡԶ�˵�ַ
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint32 GetRemoteAddr()
	{
		return mpConnection->GetRemoteIP();
	}

	///
	/// ��ȡԶ�˶˿�
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint16 GetRemotePort()
	{
		return mpConnection->GetRemotePort();
	}

	///
	/// ��ȡԶ�˵�ַ
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint32 GetLocalAddr()
	{
		return mpConnection->GetLocalIP();
	}

	///
	/// ��ȡԶ�˶˿�
	/// [6/28/2015] create by albert.xu
	///
	xgc_uint16 GetLocalPort()
	{
		return mpConnection->GetLocalPort();
	}

	///
	/// ���ø������ϵ��û�����
	/// [12/1/2014] create by albert.xu
	///
	xgc_void SetUserdata( xgc_lpvoid pUserdata )
	{
		mUserdata = pUserdata;
	}

	///
	/// ��ȡ�������ϵ��û�����
	/// [12/1/2014] create by albert.xu
	///
	xgc_lpvoid GetUserdata()const
	{
		return mUserdata;
	}
};
#endif //_BASE