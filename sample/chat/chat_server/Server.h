#pragma once
#ifndef _SERVER_H_
#define _SERVER_H_
#include "ServerSession.h"

class CServer
{
private:
	/// �Ƿ�������
	xgc_bool mRunning = true;

	/// ��Ϸ���������
	xgc_lpvoid mGameListener = xgc_nullptr;

	/// �ͻ����������
	xgc_lpvoid mChatListener = xgc_nullptr;

	/// ��Ϸ������������ַ
	xgc_char mGameBind[64];
	/// ��Ϸ�����������˿�
	xgc_uint16 mGamePort;

	/// ���������������ַ
	xgc_char mChatBind[64];
	/// ��������������˿�
	xgc_uint16 mChatPort;

public:
	CServer()
	{

	}

	~CServer()
	{

	}

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
	xgc_long Run();
};

extern CServer theServer;

#endif // _SERVER_H_