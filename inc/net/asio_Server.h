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
		class asio_Server
		{
		public:
			///
			/// \brief 构造
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			asio_Server( io_service& service_, SessionCreator creator, server_options *options );

			///
			/// \brief 析构
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			virtual ~asio_Server();

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
			xgc_void StopServer();

			///
			/// \brief 创建连接会话
			///
			/// \author albert.xu
			/// \date 2017/03/02 15:01
			///
			INetworkSession* CreateSession()
			{
				return creator_();
			}
		protected:
			///
			/// \brief 是否在白名单中
			///
			/// \author albert.xu
			/// \date 2017/08/22
			///
			xgc_bool allow( xgc_uint32 addr );

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

		protected:
			/// 网络服务对象
			io_service&			service_;
			/// 连接
			ip::tcp::acceptor	acceptor_;
			/// 超时时间
			server_options		options_;
			/// 连接队列数
			std::atomic< xgc_uint16 > acceptor_count_;
			/// 服务器状态 0 - 服务器初始状态， 1 - 服务器已开启， 2 - 等待关闭
			std::atomic< xgc_uint16 > status_;
			/// 网络消息处理句柄
			SessionCreator		creator_;
		};
	}
}
#endif // _ASIO_SERVER_H_