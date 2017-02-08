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
		 * \brief 同步连接
		 *
		 * \author albert.xu
		 * \date 十一月 2015
		 */
		xgc_bool asio_Connect( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, MessageQueuePtr queue );

		/*!
		*
		* \brief 异步连接
		*
		* \author albert.xu
		* \date 十一月 2015
		*/
		xgc_bool asio_ConnectAsync( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, MessageQueuePtr queue );

		/*!
		*
		* \brief 同步连接
		*
		* \author albert.xu
		* \date 十一月 2015
		*/
		xgc_bool asio_ConnectEx( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );

		/*!
		*
		* \brief 异步连接
		*
		* \author albert.xu
		* \date 十一月 2015
		*/
		xgc_bool asio_ConnectAsyncEx( io_service& service, xgc_lpcstr addr, xgc_int16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );
	};
};

#endif //_ASIO_ASYNC_CONNECTOR_H_