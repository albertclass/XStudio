#pragma once

#ifndef _SERVER_SESSION_H_ 
#define _SERVER_SESSION_H_ 

class CServerSession : public net::INetworkSession
{
protected:
	/// 网络句柄
	net::network_t handle_;
	/// 网络延迟
	xgc_ulong pingpong_;
	/// 最后一次接收ping消息的时间
	xgc_time64 pinglast_;
	/// 用户数据
	xgc_lpvoid userdata_;
public:
	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	CServerSession();

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual ~CServerSession();

	///
	/// \brief 获取网络句柄
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	net::network_t GetHandle()const
	{
		return handle_;
	}

	///
	/// \brief 数据包是否
	/// \return	0 ~ 成功, -1 ~ 失败
	///
	virtual int OnParsePacket( const void* data, xgc_size size ) override;

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnAccept( net::network_t handle ) override;

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnConnect( net::network_t handle ) override;

	///
	/// \brief 连接错误
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnError( xgc_uint32 error_code ) override;

	///
	/// \brief 连接关闭
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnClose() override;

	///
	/// \brief 网络保活事件
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnAlive() override;

	///
	/// \brief 接收数据
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

	///
	/// \brief 角色登陆游戏，通知聊天服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserLoginReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 角色登出游戏
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserLogoutReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 游戏服务器创建频道
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onCreateChannelReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 服务器发送系统消息
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onSystemNoteReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 设置用户数据
	///
	/// \author albert.xu
	/// \date 2017/03/27 11:35
	///
	xgc_void SetUserdata( xgc_lpvoid userdata )
	{
		userdata_ = userdata;
	}

	///
	/// \brief 获取用户数据
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_lpvoid GetUserdata()
	{
		return userdata_;
	}

	///
	/// \brief 发送数据到游戏服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2GameServer( xgc_uint16 msgid, ::google::protobuf::Message& msg );

};

#endif // _SERVER_SESSION_H_ 

