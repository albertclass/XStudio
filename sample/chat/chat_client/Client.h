#pragma once
#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "GameSrvSession.h"
class CClient : public CGameSrvSession
{
private:
	/// �Ƿ�������
	xgc_bool mRunning = true;

	/// ��Ϸ���������Ӿ��
	network_t mGameLink = INVALID_NETWORK_HANDLE;

	/// ������������Ӿ��
	network_t mChatLink = INVALID_NETWORK_HANDLE;

	/// ��Ϸ���������ӵ�ַ
	xgc_char mGameAddr[64];
	/// ��Ϸ���������Ӷ˿�
	xgc_uint16 mGamePort;

	/// ������������ӵ�ַ
	xgc_char mChatAddr[64];
	/// ������������Ӷ˿�
	xgc_uint16 mChatPort;

public:
	CClient();

	~CClient();

	///
	/// \brief ���÷�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_bool Setup( xgc_lpcstr lpConfigFile );

	///
	/// \brief ���з�����
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run();

};

extern CClient theClient;
#endif // _CLIENT_H_