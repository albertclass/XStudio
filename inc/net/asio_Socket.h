#pragma once
#ifndef _ASIO_NETWORK_HANDLER_H_
#define _ASIO_NETWORK_HANDLER_H_

namespace xgc
{
	namespace net
	{
		using namespace asio;
		// 设置发送缓冲大小
		xgc_void set_send_buffer_size( xgc_size size );
		// 设置接收缓冲大小
		xgc_void set_recv_buffer_size( xgc_size size );

		class asio_ServerBase;
		class asio_NetworkHandler;

		class asio_Socket : public std::enable_shared_from_this< asio_Socket >
		{
		public:
			asio_Socket( io_service& s, INetworkSession* holder, xgc_uint16 timeout );

			~asio_Socket();

			/*!
			 *
			 * \brief 连接到服务器
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_bool connect( xgc_lpcstr address, xgc_int16 port, xgc_bool async = false, xgc_uint16 timeout = 1 );

			/*!
			 *
			 * \brief 接收连接
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void accept( xgc_lpvoid from );

			/*!
			 *
			 * \brief 发送消息
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void send( xgc_lpvoid data, xgc_size size, xgc_bool last = false );

			/*!
			 *
			 * \brief 关闭连接
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void close();

			/*!
			*
			* \brief 关闭连接
			*
			* \author albert.xu
			* \date 十一月 2015
			*/
			xgc_ulong get_socket_info( xgc_int16 mask, xgc_byte* data );
		private:
			/*!
			 *
			 * \brief 连接成功
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void handle_connect( const asio::error_code& error );

			/*!
			 *
			 * \brief 连接超时
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void handle_timeout( const asio::error_code& error );

			/*!
			 *
			 * \brief 数据接收处理
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void handle_recv( const asio::error_code& error, size_t translate );

			/*!
			 *
			 * \brief 数据发送处理
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void handle_send( const asio::error_code& error, size_t translate );

			/*!
			 *
			 * \brief 连接保活定时器
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void handle_timer();

		public:
			ip::tcp::socket& socket()
			{
				return socket_;
			}

			xgc_lpvoid	get_userdata()const
			{
				return holder_->GetUserdata();
			}

			network_t	get_handler()const
			{
				return handle_;
			}

			xgc_void set_handler( network_t handler )
			{
				handle_ = handler;
			}

			xgc_void set_userdata( xgc_lpvoid userdata )
			{
				holder_->SetUserdata( userdata );
			}

			xgc_bool is_connected()const
			{
				return socket_.is_open();
			}

			xgc_bool belong( xgc_lpvoid token )
			{
				return token == from_;
			}

		private:
			ip::tcp::socket		socket_;            // 套接字
			std::mutex			send_buffer_lock;   // 发送缓冲区锁
			asio_NetBuffer		send_buffer_;       // 发送缓冲
			asio_NetBuffer		recv_buffer_;       // 接收缓冲

			std::atomic< xgc_uint16	> timeout_; // 心跳发送间隔
			std::atomic< xgc_uint16 > connect_status_;   // 当前连接状态 0 - 未连接， 1 - 已连接， 2 - 等待关闭
			network_t			handle_;            // 网络句柄
			xgc_lpvoid			from_;			    // 来自哪里
			asio::steady_timer	timer_;			    // 保活定时器
			INetworkSession		*holder_;           // 数据处理句柄
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
	};
};

#endif // _ASIO_NETWORK_HANDLER_H_