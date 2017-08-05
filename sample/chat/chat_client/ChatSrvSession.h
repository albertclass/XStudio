#pragma once

#ifndef _CHAT_SRV_SESSION_H_ 
#define _CHAT_SRV_SESSION_H_ 

class CChatSrvSession : public net::INetworkSession
{
protected:
	/// 网络句柄
	net::network_t handle_;
	/// 网络延迟
	xgc_ulong pingpong_;
	/// 最后一次接收ping消息的时间
	xgc_time64 pinglast_;
	/// 用户标识
	xgc_uint64 user_id_;
	/// 聊天标识
	xgc_uint32 chat_id_;
	/// 登陆令牌
	xgc_string token_;
	/// 游戏服务器连接句柄
	network_t gate_handle_;
public:
	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	CChatSrvSession( network_t gate_handle, xgc_uint64 user_id, xgc_uint32 chat_id, const xgc_string &token );

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual ~CChatSrvSession();

	///
	/// \brief 获取网络句柄
	///
	/// \author albert.xu
	/// \date 2017/08/5
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
	/// \date 2017/08/5
	///
	virtual xgc_void OnAccept( net::network_t handle ) override;

	///
	/// \brief 连接建立
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnConnect( net::network_t handle ) override;

	///
	/// \brief 连接错误
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnError( xgc_uint32 error_code ) override;

	///
	/// \brief 连接关闭
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnClose() override;

	///
	/// \brief 网络保活事件
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnAlive() override;

	///
	/// \brief 接收数据
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) override;

	///
	/// \brief 聊天服务器认证
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void ChatUserAuth();

	///
	/// \brief 密语
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void ChatTo( xgc_uint32 nChatID, xgc_lpcstr lpMessage );

	///
	/// \brief 发言
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Say( xgc_uint32 nChannelID, xgc_lpcstr lpMessage );

	///
	/// \brief 用户认证回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onUserAuthAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 已进入频道回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChannelEnterNtf( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 用户信息回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onUserInfoAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 进入频道回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChannelEnterAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 离开频道回应
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onchannelLeaveAck( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 用户聊天内容通知
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onUserChatNtf( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 频道聊天通知
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChannelChatNtf( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 聊天错误通知
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void CChatSrvSession::onChatErr( xgc_lpcstr ptr, xgc_size len );

	///
	/// \brief 聊天服务器连接断开
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void ChatSrvClosed();

	///
	/// \brief 游戏服务器连接断开
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void GateSrvClosed();

	///
	/// \brief 发送消息到Chat服务器
	///
	/// \author albert.xu
	/// \date 2017/08/05
	///
	xgc_void Send2ChatSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _CHAT_SRV_SESSION_H_ 

