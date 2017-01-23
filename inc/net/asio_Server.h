///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file asio_Server.h
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 网络服务器对象
///
///////////////////////////////////////////////////////////////
#pragma once
#ifndef _ASIO_SERVER_H_
#define _ASIO_SERVER_H_

namespace xgc
{
	namespace net
	{
		using namespace asio;

		///
		/// \brief 服务器对象基类
		///
		/// \author albert.xu
		/// \date 2016/02/26 15:49
		///
		class asio_ServerBase
		{
		public:
			///
			/// \brief 构造
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			asio_ServerBase( io_service& service_, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 time_invent, xgc_uint16 timeout );

			///
			/// \brief 析构
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			virtual ~asio_ServerBase();

			///
			/// \brief 开启服务器
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			xgc_bool StartServer( const char *address, xgc_int16 port );

			///
			/// \brief 停止服务器
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			xgc_void StopServer( xgc_bool bCloseAllLink = true );

		protected:
			///
			/// \brief 接受连接
			///
			/// \author albert.xu
			/// \date 2016/02/26 16:01
			///
			xgc_void handle_accept( asio_SocketPtr pSocket, const asio::error_code& error);

			///
			/// \brief 投递连接请求
			///
			/// \author albert.xu
			/// \date 2016/02/26 16:01
			///
			xgc_void post_accept();

		private:
			///
			/// \brief 创建网络会话
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:48
			///
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal ) = 0;
		protected:
			io_service&			service_;
			ip::tcp::acceptor	acceptor_;
			xgc_uint16			time_invent_;
			xgc_uint16			timeout_;
			xgc_uint16			acceptor_count_;
			PacketProtocal*		protocal_;
		};

		///
		/// \brief 一般服务器对象
		///
		/// \author albert.xu
		/// \date 2016/02/26 15:49
		///
		class asio_Server : public asio_ServerBase
		{
		public:
			asio_Server( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, MessageQueuePtr queue_ptr );
			~asio_Server();

		protected:
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal );

		private:
			MessageQueuePtr	queue_ptr_;
		};

		///
		/// \brief 高级服务器对象
		///
		/// \author albert.xu
		/// \date 2016/02/26 15:50
		///
		class asio_ServerEx	:	public asio_ServerBase
		{
		public:
			asio_ServerEx( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, create_handler_func call, xgc_uintptr param );
			~asio_ServerEx();

		protected:
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal );

		private:
			create_handler_func call_;
			xgc_uintptr param_;
		};

	}
}
#endif // _ASIO_SERVER_H_