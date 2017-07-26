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
	/// ��ʼ������
	///
	NET_MODULE_API xgc_bool InitializeNetwork( xgc_lpcstr conf );

	///
	/// [2/17/2014 albert.xu]
	/// ������������
	///
	NET_MODULE_API xgc_bool ProcessNetwork();

	///
	/// [12/30/2013 albert.xu]
	/// ��ֹ����
	///
	NET_MODULE_API xgc_void FinializeNetwork();

	///
	/// \brief ��ȡ���������(�ַ���)
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:55
	///
	NET_MODULE_API NETWORK_ID GetNetworkId();

	///
	/// \brief ��ȡ���������
	///
	/// \author albert.xu
	/// \date 2017/02/28 15:54
	///
	NET_MODULE_API NETWORK_ID GetNetworkRegion( NETWORK_REGION Id[NETWORK_REGION_N] );

	///
	/// ת��������ID���ַ���
	/// [11/27/2014] create by albert.xu
	///
	NET_MODULE_API xgc_lpcstr NetworkId2Str( NETWORK_ID nNetworkId, xgc_char szOutput[NETWORK_REGION_S] );

	///
	/// ת��������IDΪ��ֵ
	/// [11/27/2014] create by albert.xu
	///
	NET_MODULE_API NETWORK_ID Str2NetworkId( xgc_lpcstr pNetworkId );

	///
	/// \brief ������������
	///
	/// \author albert.xu
	/// \date 2017/03/08 12:09
	///
	NET_MODULE_API xgc_void MakeVirtualSock( CClientSession* pSession, NETWORK_ID nNetworkID, xgc_uint32 nToken );

	///
	/// \brief �Ͽ���������
	///
	/// \author albert.xu
	/// \date 2017/03/08 12:09
	///
	NET_MODULE_API xgc_void KickVirtualSock( CClientSession* pSession, NETWORK_ID nNetworkID, xgc_uint32 nToken );

	///
	/// \brief ע��ͻ������Ӵ�����
	///
	/// \author albert.xu
	/// \date 2017/03/08 15:09
	///
	NET_MODULE_API xgc_void RegistClientHandler( ClientMsgHandler fnMsgHandler, ClientEvtHandler fnEvtHandler );

	///
	/// \brief ע��ܵ�������
	///
	/// \author albert.xu
	/// \date 2017/03/08 15:10
	///
	NET_MODULE_API xgc_void RegistPipeHandler( xgc_lpcstr lpNetworkId, PipeMsgHandler fnMsgHandler, PipeEvtHandler fnEvtHandler );

	///
	/// \brief ע���м̴�����
	///
	/// \author albert.xu
	/// \date 2017/03/08 15:11
	///
	NET_MODULE_API xgc_void RegistVirtualSockHandler( xgc_lpcstr lpNetworkId, SockMsgHandler fnMsgHandler, SockEvtHandler fnEvtHandler );
}

///
/// \brief �������ݵ��ͻ��˻Ự
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
NET_MODULE_API xgc_void SendPacket( CClientSession* pSession, xgc_lpvoid pData, xgc_size nSize );

///
/// \brief �������ݵ��ܵ�
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
NET_MODULE_API xgc_void SendPacket( CPipeSession* pSession, xgc_lpvoid pData, xgc_size nSize );

///
/// \brief �������ݵ��м�
///
/// \author albert.xu
/// \date 2017/03/20 15:19
///
NET_MODULE_API xgc_void SendPacket( CRelaySession* pSession, xgc_lpvoid pData, xgc_size nSize );

#endif // _NET_MODULE_H_