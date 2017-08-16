#pragma once
#ifndef _PIPE_MANAGER_H_ 
#define _PIPE_MANAGER_H_ 
namespace net_module
{
	///
	/// \brief ��ʼ���ܵ�������
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:48
	///
	xgc_bool InitPipeManager();

	///
	/// \brief ˽������
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:48
	///
	xgc_void FiniPipeManager();

	///
	/// \brief ��ӹܵ���Ϣ
	///
	/// \author albert.xu
	/// \date 2017/03/20 14:14
	///
	xgc_bool OnPipeConnect( NETWORK_ID nID, CPipeSession* pPipe );

	///
	/// \brief �����ܵ�����
	///
	/// \author albert.xu
	/// \date 2017/07/17 18:19
	///
	xgc_bool RegistPipeHandler( xgc_lpcstr lpNetworkId, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );

	///
	/// \brief �����ܵ�����
	///
	/// \author albert.xu
	/// \date 2017/07/17 18:19
	///
	xgc_void RegistSockHandler( xgc_lpcstr lpNetworkId, SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler );

	///
	/// \brief �����ܵ�����
	///
	/// \author albert.xu
	/// \date 2017/02/28 16:19
	///
	xgc_bool PipeConnect( NETWORK_ID nID, xgc_lpcstr pAddr, xgc_uint16 nPort, net::connect_options &options );

	///
	/// \brief ��ȡ�ܵ��Ự
	///
	/// \author albert.xu
	/// \date 2017/02/28 16:20
	///
	CPipeSession* GetPipe( NETWORK_ID nID );
}
#endif // _PIPE_MANAGER_H_ 
