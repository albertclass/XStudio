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
	/// 公共频道
	xgc_map< xgc_uint32, xgc_string > channels_;
	/// 服务器上的用户
	xgc_map< xgc_uint64, xgc_uint32 > users_;
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
	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code ) override;

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
	/// \brief 创建频道回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onCreateChannelAck( xgc_lpvoid ptr, int len );

	///
	/// \brief 角色登陆聊天服务器回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onLoginAck( xgc_lpvoid ptr, int len );

	///
	/// \brief 用户登陆
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_long UserLogin( xgc_uint64 user_id, const xgc_string &nickname, xgc_uint32 session_id );

	///
	/// \brief 用户登出
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void UserLogout( xgc_uint64 user_id );

	///
	/// \brief 发送系统通知
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void SendSystemNote( xgc_uint32 channel_id, xgc_string note, xgc_uint32 flags );

	///
	/// \brief 发送数据
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _SERVER_SESSION_H_ 

