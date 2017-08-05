#pragma once
#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "GameSrvSession.h"
class CClient : public CGameSrvSession
{
private:
	/// 是否运行中
	xgc_bool mRunning = true;

	/// 游戏服务器连接句柄
	network_t mGameLink = INVALID_NETWORK_HANDLE;

	/// 聊天服务器连接句柄
	network_t mChatLink = INVALID_NETWORK_HANDLE;

	/// 游戏服务器连接地址
	xgc_char mGameAddr[64];
	/// 游戏服务器连接端口
	xgc_uint16 mGamePort;

	/// 聊天服务器连接地址
	xgc_char mChatAddr[64];
	/// 聊天服务器连接端口
	xgc_uint16 mChatPort;

public:
	CClient();

	~CClient();

	///
	/// \brief 配置服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_bool Setup( xgc_lpcstr lpConfigFile );

	///
	/// \brief 运行服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Run();

};

extern CClient theClient;
#endif // _CLIENT_H_