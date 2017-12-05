#pragma once
#ifndef _SERVER_H_
#define _SERVER_H_
#include "SrvSession.h"

class CServer : public CServerSession
{
private:
	/// �Ƿ�������
	xgc_bool mRunning = true;

	/// ���������
	xgc_lpvoid mListener = xgc_nullptr;

	/// ������������Ӿ��
	network_t mChatLink = INVALID_NETWORK_HANDLE;

	/// ������������ַ
	xgc_char mGateAddr[64];
	/// �����������˿�
	xgc_uint16 mGatePort;

	/// ������������ӵ�ַ
	xgc_char mChatAddr[64];
	/// ������������Ӷ˿�
	xgc_uint16 mChatPort;
	/// ͳ���ڷ������ϵ���������
	xgc_uint32 mConnectCount;

	/// �˺���Ϣ
	struct Account
	{
		xgc_uint64 user_id;
		xgc::string username;
		xgc::string password;
	};

	xgc::unordered_map< xgc::string, Account > mAccountMap;
public:
	CServer();

	~CServer();

	///
	/// \brief ���÷�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_bool Setup( xgc_lpcstr lpConfigFile );

	///
	/// \brief ��֤�û�
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_long VerificationUser( const xgc::string &username, const xgc::string &password, xgc_uint64 &user_id );

	///
	/// \brief ���з�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run();

	///
	/// \brief ���ӽ���
	///
	/// \author albert.xu
	/// \date 2017/08/16
	///
	xgc_void Connected()
	{
		++mConnectCount;
	}

	///
	/// \brief ���ӶϿ�
	///
	/// \author albert.xu
	/// \date 2017/08/16
	///
	xgc_void Disconnected()
	{
		--mConnectCount;
	}


};

extern CServer theServer;
#endif // _SERVER_H_