#pragma once

#ifndef _CLIENT_SESSION_H_ 
#define _CLIENT_SESSION_H_ 

class CClientSession : public net::INetworkSession
{
protected:
	/// 网络句柄
	net::network_t handle_;
	/// 网络延迟
	xgc_ulong pingpong_;
	/// 最后一次接收ping消息的时间
	xgc_time64 pinglast_;
	/// 用户数据
	xgc_uint32 chat_id_;
public:
	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	CClientSession();

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual ~CClientSession();

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
	/// \brief 用户认证
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserAuth( xgc_lpvoid ptr, int len );

	///
	/// \brief 请求用户信息
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserInfoReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 请求进入频道
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onEnterChannelReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 请求离开频道
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onLeaveChannelReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 用户私聊请求
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onUserChatReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 用户频道发言请求
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void onChannelChatReq( xgc_lpvoid ptr, int len );

	///
	/// \brief 发送数据到客户端
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2Client( xgc_uint16 msgid, ::google::protobuf::Message& msg );

};

#endif // _NETSESSION_H_ 

