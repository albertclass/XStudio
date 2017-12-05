#pragma once
#ifndef _CHANNEL_H_
#define _CHANNEL_H_
#include "auto_handle.h"

class CChannel : public auto_handle< CChannel >
{
public:
	/// 用户房间信息
	struct user
	{
		xgc_uint32 chat_id;
		/// 最后一次发言的时间
		xgc_time64 last_timestamp;
		/// 是否被禁言
		xgc_bool forbid;
		/// 是否断线
		xgc_bool disconnect;
	};

private:
	/// 频道名字
	xgc::string mName;

	/// 是否自动删除
	xgc_bool mAutoDestory;

	/// 自动删除延迟
	xgc_long mAutoDestoryDelay;

	/// 自动踢出静默玩家
	xgc_long mAutoKickDelay;

	/// 房间密码
	xgc::string mPassword;

	/// 房间删除的时间
	xgc_time64 mDestoryTime;

	/// 用户列表
	xgc::map< xgc_uint32, user > mUserConfigMap;
public:
	///
	/// \brief 构造
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CChannel( const xgc::string &strName );

	///
	/// \brief 析构
	/// \author albert.xu
	/// \date 2017/08/03
	///
	~CChannel();

	///
	/// \brief 获取频道名
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_lpcstr getName()const
	{
		return mName.c_str();
	}

	///
	/// \brief 获取频道名
	/// \author albert.xu
	/// \date 2017/08/11
	///
	xgc_time64 getDestoryTime() const
	{
		return mDestoryTime;
	}

	///
	/// \brief 获取用户数量
	/// \author albert.xu
	/// \date 2017/08/11
	///
	xgc_size getUserCount() const
	{
		return mUserConfigMap.size();
	}

	///
	/// \brief 进入频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long Enter( xgc_uint32 nChatID, xgc_lpcstr lpPassword );

	///
	/// \brief 离开频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void Leave( xgc_uint32 nChatID );

	///
	/// \brief 检查活跃
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void eraseQuietUser();

	///
	/// \brief 是否有效用户
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_bool isValidUser( xgc_uint32 chat_id );

	///
	/// \brief 发送聊天
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long Chat( xgc_uint32 nChatID, xgc_lpcstr lpText, xgc_size nSize, xgc_uint32 nToken );

	///
	/// \brief 遍历频道中的对象
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void ForEachUser( std::function< xgc_void( user &user ) > &&invoke )
	{
		for( auto &pair : mUserConfigMap )
			invoke( pair.second );
	}
};

#endif // _CHANNEL_H_