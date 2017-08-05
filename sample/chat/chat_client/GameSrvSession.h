#pragma once

#ifndef _GATE_SRV_SESSION_H_ 
#define _GATE_SRV_SESSION_H_ 

class CGameSrvSession : public net::INetworkSession
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
	CGameSrvSession();

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	virtual ~CGameSrvSession();

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
	/// \brief 设置用户数据
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	xgc_void setUserdata( xgc_lpvoid userdata )
	{
		userdata_= userdata;
	}

	///
	/// \brief 获取用户数据
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	xgc_lpvoid getUserdata()
	{
		return userdata_;
	}

	///
	/// \brief 发送数据到游戏服务器
	///
	/// \author albert.xu
	/// \date 2017/08/5
	///
	xgc_void Send2GateSrv( xgc_uint16 msgid, ::google::protobuf::Message& msg );
};

#endif // _GATE_SRV_SESSION_H_ 

