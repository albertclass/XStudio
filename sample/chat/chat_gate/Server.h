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
	/// 统计在服务器上的连接数量
	xgc_uint32 mConnectCount;

	/// 账号信息
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
	xgc_long VerificationUser( const xgc::string &username, const xgc::string &password, xgc_uint64 &user_id );

	///
	/// \brief 运行服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run();

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/08/16
	///
	xgc_void Connected()
	{
		++mConnectCount;
	}

	///
	/// \brief 连接断开
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