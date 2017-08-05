#pragma once
#ifndef _SERVER_H_
#define _SERVER_H_
#include "SrvSession.h"

class CServer : public CServerSession
{
private:
	/// 是否运行中
	xgc_bool mRunning = true;

	/// 服务器句柄
	xgc_lpvoid mListener = xgc_nullptr;

	/// 聊天服务器连接句柄
	network_t mChatLink = INVALID_NETWORK_HANDLE;

	/// 服务器监听地址
	xgc_char mGateAddr[64];
	/// 服务器监听端口
	xgc_uint16 mGatePort;

	/// 聊天服务器连接地址
	xgc_char mChatAddr[64];
	/// 聊天服务器连接端口
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
	/// \brief 配置服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_bool Setup( xgc_lpcstr lpConfigFile );

	///
	/// \brief 验证用户
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_long VerificationUser( const xgc_string &username, const xgc_string &password, xgc_uint64 &user_id );

	///
	/// \brief 运行服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run();
};

extern CServer theServer;
#endif // _SERVER_H_