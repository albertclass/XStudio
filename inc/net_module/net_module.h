#pragma once
#ifndef _NET_MODULE_H_
#define _NET_MODULE_H_
#include "net_defines.h"

using net_module::CClientSession;
using net_module::CRelaySession;
using net_module::CPipeSession;

using net_module::ClientMsgHandler;
using net_module::PipeMsgHandler;
using net_module::SockMsgHandler;

using net_module::ClientEvtHandler;
using net_module::PipeEvtHandler;
using net_module::SockEvtHandler;

extern "C"
{
	///
	/// [12/30/2013 albert.xu]
	/// 初始化网络
	///
	NET_MODULE_API xgc_bool InitializeNetwork( xgc_lpcstr conf );

	///
	/// [2/17/2014 albert.xu]
	/// 处理网络数据
	///
	NET_MODULE_API xgc_bool ProcessNetwork();

	///
	/// [12/30/2013 albert.xu]
	/// 终止网络
	///
	NET_MODULE_API xgc_void FinializeNetwork();

	///
	/// \brief 获取服务器编号(字符串)
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:55
	///
	NET_MODULE_API NETWORK_ID GetNetworkId();

	///
	/// \brief 获取服务器编号
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:54
	///
	NET_MODULE_API NETWORK_ID GetNetworkRegion( NETWORK_REGION Id[NETWORK_REGION_N] );

	///
	/// 转换服务器ID到字符串
	/// [11/27/2014] create by albert.xu
	///
	NET_MODULE_API xgc_lpcstr NetworkId2Str( NETWORK_ID nNetworkId, xgc_char szOutput[NETWORK_REGION_S] );

	///
	/// 转换服务器ID为数值
	/// [11/27/2014] create by albert.xu
	///
	NET_MODULE_API NETWORK_ID Str2NetworkId( xgc_lpcstr pNetworkId );

	///
	/// \brief 建立虚拟连接
	///
	/// \author albert.xu
	/// \date 2017/03/08 12:09
	///
	NET_MODULE_API xgc_void MakeVirtualSock( CClientSession* pSession, NETWORK_ID nNetworkID, xgc_uint32 nToken );

	///
	/// \brief 断开虚拟连接
	///
	/// \author albert.xu
	/// \date 2017/03/08 12:09
	///
	NET_MODULE_API xgc_void KickVirtualSock( CClientSession* pSession, NETWORK_ID nNetworkID, xgc_uint32 nToken );

	///
	/// \brief 注册客户端连接处理函数
	///
	/// \author albert.xu
	/// \date 2017/03/08 15:09
	///
	NET_MODULE_API xgc_void RegistClientHandler( ClientMsgHandler fnMsgHandler, ClientEvtHandler fnEvtHandler );

	///
	/// \brief 注册管道处理函数
	///
	/// \author albert.xu
	/// \date 2017/03/08 15:10
	///
	NET_MODULE_API xgc_void RegistPipeHandler( xgc_lpcstr lpNetworkId, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );

	///
	/// \brief 注册中继处理函数
	///
	/// \author albert.xu
	/// \date 2017/03/08 15:11
	///
	NET_MODULE_API xgc_void RegistVirtualSockHandler( xgc_lpcstr lpNetworkId, SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler );
}

///
/// \brief 发送数据到客户端会话
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
NET_MODULE_API xgc_void SendPacket( CClientSession* pSession, xgc_lpvoid pData, xgc_size nSize );

///
/// \brief 发送数据到管道
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
NET_MODULE_API xgc_void SendPacket( CPipeSession* pSession, xgc_lpvoid pData, xgc_size nSize );

///
/// \brief 发送数据到中继
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
NET_MODULE_API xgc_void SendPacket( CRelaySession* pSession, xgc_lpvoid pData, xgc_size nSize );

#endif // _NET_MODULE_H_