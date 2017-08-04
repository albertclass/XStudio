#pragma once
#ifndef _USER_MGR_H_
#define _USER_MGR_H_
class CUser;
class CUserMgr
{
	friend CUserMgr& getUserMgr();

private:
	/// 游戏ID映射表
	xgc_unordered_map< xgc_uint64, xgc_uint32 > mUserMap;

	/// 昵称映射表
	xgc_unordered_map< xgc_string, xgc_uint32 > mNickMap;

private:
	///
	/// \brief 角色管理器构造
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CUserMgr();

	///
	/// \brief 角色管理器析构
	/// \author albert.xu
	/// \date 2017/08/03
	///
	~CUserMgr();

public:
	///
	/// \brief 角色登陆
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long UserLogin( xgc_uint64 nUserID, const xgc_string &strServer, const xgc_string &strNick, const xgc_string &strExtra );

	///
	/// \brief 角色登出
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void UserLogout( xgc_uint64 nUserID );

	///
	/// \brief 角色登出
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CUser* GetUserByUID( xgc_uint64 nUserID );
};

CUserMgr& getUserMgr();

#endif // _USER_MGR_H_