#pragma once
#ifndef _USER_H_
#define _USER_H_
#include "auto_handle.h"

class CChannel;
class CUser : public auto_handle< CUser >
{
private:
	/// 角色的游戏ID
	xgc_uint64 mUserID;
	/// 角色昵称
	xgc_string mNick;
	/// 角色所属的服务器
	xgc_string mServer;
	/// 角色的其他数据
	xgc_string mExtra;
	/// 角色Token
	xgc_char mToken[64];
	
	net::network_t mNetHandle;

	/// 加入的频道列表
	xgc_set< xgc_uint32 > mChannelSet;
public:
	CUser( xgc_uint64 nUserID );
	~CUser();

	///
	/// \brief 角色上线
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void online( net::network_t hNet )
	{
		mNetHandle = hNet;
	}

	///
	/// \brief 角色下线
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void offline()
	{
		mNetHandle = INVALID_NETWORK_HANDLE;
	}

	///
	/// \brief 设置角色服务器
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void setServer( const xgc_string &strServer )
	{
		mServer = strServer;
	}

	///
	/// \brief 设置角色服务器
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void setNickName( const xgc_string &strNick )
	{
		mNick = strNick;
	}

	///
	/// \brief 设置角色服务器
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void setExtra( const xgc_string &strExtra )
	{
		mExtra = strExtra;
	}

	///
	/// \brief 设置角色服务器
	/// \author albert.xu
	/// \date 2017/08/03
	///
	const xgc_string& getNickName()const
	{
		return mNick;
	}

	///
	/// \brief 设置角色服务器
	/// \author albert.xu
	/// \date 2017/08/03
	///
	const xgc_string& getExtra()const
	{
		return mExtra;
	}

	///
	/// \brief 重置角色Token
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_lpcstr genToken();

	///
	/// \brief 检查角色Token
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_bool checkToken( const xgc_string &strToken );


	///
	/// \brief 角色加入频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void onEnterChannel( CChannel* pChannel );

	///
	/// \brief 角色离开频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void onLeaveChannel( CChannel* pChannel );

	///
	/// \brief 遍历角色加入的频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void forEachChannel( const std::function< void( xgc_uint32 ) > &_Pred )
	{
		for( auto channel_id : mChannelSet )
			_Pred( channel_id );
	}

	///
	/// \brief 发送数据给角色
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_void Send( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _USER_H_