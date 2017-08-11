#pragma once
#ifndef _APPLICATION_H_
#define _APPLICATION_H_
class CApplication
{
private:
	/// 是否运行中
	xgc_bool mRunning = true;

	/// 游戏服务器连接地址
	xgc_char mGameAddr[64];
	/// 游戏服务器连接端口
	xgc_uint16 mGamePort;

	/// 聊天服务器连接地址
	xgc_char mChatAddr[64];
	/// 聊天服务器连接端口
	xgc_uint16 mChatPort;

public:
	CApplication();

	~CApplication();

	///
	/// \brief 获取游戏服务器连接地址
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_lpcstr getGameAddr() const
	{
		return mGameAddr;
	}

	///
	/// \brief 获取游戏服务器连接地址
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_uint16 getGamePort() const
	{
		return mGamePort;
	}

	///
	/// \brief 获取聊天服务器连接地址
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_lpcstr getChatAddr() const
	{
		return mChatAddr;
	}

	///
	/// \brief 获取聊天服务器连接地址
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_uint16 getChatPort() const
	{
		return mChatPort;
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
	xgc_void Run( int argc, char *argv[] );

	///
	/// \brief 清理服务器
	///
	/// \author albert.xu
	/// \date 2017/08/10
	///
	xgc_void Clear();
};

extern CApplication theApp;
#endif // _APPLICATION_H_