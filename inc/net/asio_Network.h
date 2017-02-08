///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file asio_Network.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 网络API封装
///
///////////////////////////////////////////////////////////////
#ifndef _NETWORK_H_
#define _NETWORK_H_
#pragma once

namespace xgc
{
	namespace net
	{
		///
		/// \brief 网络库引擎封装类
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:58
		///
		class asio_Network
		{
			friend asio_Network& getNetwork();

		private:
			asio_Network(void);
			~asio_Network(void);

			xgc_void run();	
		public:
			xgc_bool insert_workthread( xgc_int32 c );
			xgc_void exit();

			asio::io_service& Ref(){ return *service_; }
		private:
			asio::io_service *service_;
			std::vector< std::shared_ptr< asio::thread > >	workthreads_;
		};

		///
		/// \brief get asio network
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		asio_Network& getNetwork();

		///
		/// \brief 初始化网络
		///
		/// \param workthreads 开启的网络线程数
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		xgc_bool asio_InitNetwork( int workthreads );


		///
		/// \brief 关闭网络
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		xgc_void asio_FiniNetwork();


		/// 
		/// \brief	开启服务器
		/// \param	address 服务器监听地址
		/// \param	port 服务器监听端口
		/// \param	msg_queue 服务器消息队列
		/// 
		/// \return server 句柄，关闭服务器时需要提供相应句柄
		/// 
		xgc_uintptr asio_StartServer( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );

		/// 
		/// 
		/// \brief 开启服务器
		/// 
		/// \author albert.xu
		/// \date 十一月 2015
		/// 
		xgc_uintptr asio_StartServerEx( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );

		///
		/// \brief 关闭服务器
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_void asio_CloseServer( xgc_uintptr server_h );

		///
		/// \brief 连接到服务器
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_Connect( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );

		///
		/// \brief 异步连接到服务器
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_ConnectAsync( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );

		///
		/// \brief 连接到服务器
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_ConnectEx( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );

		///
		/// \brief 异步连接到服务器
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_ConnectAsyncEx( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );
		///
		/// \brief 发送消息包
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendPacket( network_t handle, xgc_lpvoid data, xgc_size size );

		///
		/// \brief 发送消息包并断开网络连接
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendLastPacket( network_t handle, xgc_lpvoid data, xgc_size size );

		///
		/// \brief 发送一组消息包
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size );

		///
		/// \brief 发送到一个组
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendToGroup( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself );

		///
		/// \brief 关闭连接
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_CloseLink( network_t handle );

		///
		/// \brief 设置网络状态
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_uintptr asio_ExecuteState( xgc_uint32 operate_code, xgc_uintptr param );

	}
}
#endif // _NETWORK_H_