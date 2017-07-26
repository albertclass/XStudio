#pragma once
#ifndef _NET_MODULE_CONFIG_H_
#define _NET_MODULE_CONFIG_H_
// crt
#include <stdio.h>
#include <stdlib.h>

// stl
#include <thread>
#include <atomic>

// common
#include "defines.h"
#include "logger.h"
#include "pugixml.hpp"
#include "exception.h"
#include "destructor.h"
#include "xutility.h"

// net
#include "net.h"

using namespace xgc;
using namespace xgc::common;

#if defined(_LINUX)
#	include <arpa/inet.h>
#	include <endian.h>
#endif

#if defined(_WINDOWS)
#	include <WinSock2.h>
#	pragma comment( lib, "ws2_32.lib" )
#endif

#include "net_defines.h"

namespace net_module
{
	///
	/// \brief ����ģʽ
	///
	/// \author albert.xu
	/// \date 2017/02/23 11:07
	///
	enum enListenMode
	{
		/// ��ͨģʽ����ת��
		eMode_Normal = 0,
		/// Gateģʽ�����ת��
		eMode_Gate = 1,
		/// Pipeģʽ
		eMode_Pipe = 2,
		/// Relayģʽ���м�
		eMode_Relay = 3,
	};

	///
	/// \brief ���������ʶ
	///
	/// \author albert.xu
	/// \date 2017/03/20 11:48
	///
	NETWORK_ID _SetNetworkID( xgc_lpcstr pNetworkId );

	///
	/// \brief ��ȡ�����ʶ
	///
	/// \author albert.xu
	/// \date 2017/03/20 10:52
	///
	NETWORK_ID _GetNetworkID();

	///
	/// \brief ת�������ʶΪ��ֵ
	///
	/// \author albert.xu
	/// \date 2017/03/20 10:52
	///
	NETWORK_ID _Str2NetworkID( xgc_lpcstr pNetworkId );

	///
	/// \brief ת����ֵΪ�����ʶ
	///
	/// \author albert.xu
	/// \date 2017/03/20 10:53
	///
	xgc_lpcstr _NetworkID2Str( NETWORK_ID nNetworkId, xgc_char szOutput[NETWORK_REGION_S] );
}

#endif // _NET_MODULE_CONFIG_H_