#pragma once
#ifndef _ASIO_ASYNC_CONNECTOR_H_
#define _ASIO_ASYNC_CONNECTOR_H_
#include "Netbase.h"
#include "asio_Header.h"

using namespace asio;

namespace xgc
{
	namespace net
	{
		/*!
		 *
		 * \brief ͬ������
		 *
		 * \author albert.xu
		 * \date ʮһ�� 2015
		 */
		xgc_bool Connect( io_service& service, xgc_lpcstr addr, xgc_int16 port, PacketProtocal *protocal, MessageQueuePtr queue_ptr, xgc_uint16 timeout );

		/*!
		*
		* \brief �첽����
		*
		* \author albert.xu
		* \date ʮһ�� 2015
		*/
		xgc_bool ConnectAsync( io_service& service, xgc_lpcstr addr, xgc_int16 port, PacketProtocal *protocal, MessageQueuePtr queue_ptr, xgc_uint16 timeout );
	};
};

#endif //_ASIO_ASYNC_CONNECTOR_H_