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
			asio_ServerBase( io_service& service_, xgc_uint16 acceptor_count, xgc_uint16 timeout, const pfnCreateHolder & creator );

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

		protected:
			io_service&			service_;
			ip::tcp::acceptor	acceptor_;
			xgc_uint16			timeout_;
			xgc_uint16			acceptor_count_;
			///
			/// \brief 创建网络会话
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:48
			///
			pfnCreateHolder		creator_;
		};
	}
}
#endif // _ASIO_SERVER_H_