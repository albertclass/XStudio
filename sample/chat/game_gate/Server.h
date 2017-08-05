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

	struct Account
	{
		xgc_uint64 user_id;
		xgc_string username;
		xgc_string password;
	};

	xgc_unordered_map< xgc_string, Account > mAccountMap;
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
	xgc_long VerificationUser( const xgc_string &username, const xgc_string &password, xgc_uint64 &user_id );

	///
	/// \brief ���з�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run();
};

extern CServer theServer;
#endif // _SERVER_H_