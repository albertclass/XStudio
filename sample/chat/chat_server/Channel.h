#pragma once
#ifndef _CHANNEL_H_
#define _CHANNEL_H_
#include "auto_handle.h"

class CChannel : public auto_handle< CChannel >
{
private:
	/// 频道名字
	xgc_string mName;

	/// 频道属性

	/// 是否自动删除
	xgc_bool mAutoDestory;

	/// 自动删除延迟
	xgc_long mAutoDestoryDelay;

	/// 自动踢出静默玩家
	xgc_long mAutoKickDelay;

	/// 房间密码
	xgc_string mPassword;

	/// 用户房间信息
	struct UserConfig
	{
		/// 是否被禁言
		xgc_bool forbid;
		/// 是否断线
		xgc_bool disconnect;
		/// 最后一次发言的时间
		xgc_time64 last_timestamp;
	};

	/// 用户列表
	xgc_map< xgc_uint32, UserConfig > mUserConfigMap;
public:
	///
	/// \brief 构造
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CChannel( const xgc_string &strName );

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
	xgc_void AliveCheck();

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
	xgc_void ForEachUser( std::function< xgc_void( xgc_uint32, xgc_bool ) > &&invoke )
	{
		for( auto &pair : mUserConfigMap )
		{
			invoke( pair.first, pair.second.forbid );
		}
	}
};

#endif // _CHANNEL_H_