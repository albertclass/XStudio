#pragma once

#ifndef _GATE_SRV_SESSION_H_ 
#define _GATE_SRV_SESSION_H_ 

class CChatSrvSession;
class CGameSrvSession : public net::INetworkSession
{
protected:
	/// 网络句柄
	net::network_t handle_;
	/// 网络延迟
	xgc_ulong pingpong_;
	/// 最后一次接收ping消息的时间
	xgc_time64 pinglast_;
	/// 聊天服务器连接会话
	CChatSrvSession* mChatSrvSession;
	/// 用户名
	xgc_string mUsername;
	/// 密码
	xgc_string mPassword;
public:
	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	CGameSrvSession( xgc_lpcstr username, xgc_lpcstr password );

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual ~CGameSrvSession();

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
	/// \brief 接收聊天服务器数据
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_bool OnChatMsg( xgc_uint16 msgid, xgc_lpcstr ptr, xgc_long len )
	{
		return false;
	}

	///
	/// \brief 聊天服务器连接建立
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnChatConnect( network_t handle );

	///
	/// \brief 聊天服务器连接建立
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnChatError( xgc_int16 error_type, xgc_int16 error_code )
	{
		if( error_type == NET_ETYPE_CONNECT )
			CloseLink( handle_ );
	}

	///
	/// \brief 聊天服务器连接断开
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	virtual xgc_void OnChatClose();

	///
	/// \brief 发送数据到游戏服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2GameSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );

	///
	/// \brief 发送数据到游戏服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );

	///
	/// \brief 断开游戏服务器连接
	///
	/// \author albert.xu
	/// \date 2017/08/09
	///
	xgc_void Disconnect();
};

#endif // _GATE_SRV_SESSION_H_ 

