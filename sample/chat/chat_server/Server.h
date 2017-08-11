#pragma once
#ifndef _SERVER_H_
#define _SERVER_H_
#include "ServerSession.h"

class CServer
{
private:
	/// 是否运行中
	xgc_bool mRunning = true;

	/// 游戏服务器句柄
	xgc_lpvoid mGameListener = xgc_nullptr;

	/// 客户服务器句柄
	xgc_lpvoid mChatListener = xgc_nullptr;

	/// 游戏服务器监听地址
	xgc_char mGameBind[64];
	/// 游戏服务器监听端口
	xgc_uint16 mGamePort;

	/// 聊天服务器监听地址
	xgc_char mChatBind[64];
	/// 聊天服务器监听端口
	xgc_uint16 mChatPort;

public:
	CServer()
	{

	}

	~CServer()
	{

	}

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
	xgc_long Run();
};

extern CServer theServer;

#endif // _SERVER_H_