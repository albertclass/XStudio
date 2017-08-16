#pragma once
#ifndef _PIPE_MANAGER_H_ 
#define _PIPE_MANAGER_H_ 
namespace net_module
{
	///
	/// \brief 初始化管道管理器
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:48
	///
	xgc_bool InitPipeManager();

	///
	/// \brief 私有析构
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:48
	///
	xgc_void FiniPipeManager();

	///
	/// \brief 添加管道信息
	///
	/// \author albert.xu
	/// \date 2017/03/20 14:14
	///
	xgc_bool OnPipeConnect( NETWORK_ID nID, CPipeSession* pPipe );

	///
	/// \brief 建立管道连接
	///
	/// \author albert.xu
	/// \date 2017/07/17 18:19
	///
	xgc_bool RegistPipeHandler( xgc_lpcstr lpNetworkId, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );

	///
	/// \brief 建立管道连接
	///
	/// \author albert.xu
	/// \date 2017/07/17 18:19
	///
	xgc_void RegistSockHandler( xgc_lpcstr lpNetworkId, SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler );

	///
	/// \brief 建立管道连接
	///
	/// \author albert.xu
	/// \date 2017/02/28 16:19
	///
	xgc_bool PipeConnect( NETWORK_ID nID, xgc_lpcstr pAddr, xgc_uint16 nPort, net::connect_options &options );

	///
	/// \brief 获取管道会话
	///
	/// \author albert.xu
	/// \date 2017/02/28 16:20
	///
	CPipeSession* GetPipe( NETWORK_ID nID );
}
#endif // _PIPE_MANAGER_H_ 
