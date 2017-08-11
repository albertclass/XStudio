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
		// 设置发送数据大小
		xgc_void set_send_packet_size( xgc_size size );
		// 设置接收数据大小
		xgc_void set_recv_packet_size( xgc_size size );

		class asio_Server;
		class asio_NetworkHandler;

		class asio_Socket : public std::enable_shared_from_this< asio_Socket >
		{
		friend class asio_Server;
		friend class asio_SocketMgr;
		public:
			enum set_option { e_recv, e_send, e_both };

			///
			/// \brief 构造
			/// \author albert.xu
			/// \date 2015/11/13
			///
			asio_Socket( io_service& s, xgc_uint16 timeout, xgc_lpvoid userdata, xgc_lpvoid from );

			///
			/// \brief 析构
			/// \author albert.xu
			/// \date 2015/11/13
			///
			~asio_Socket();

			/*!
			 *
			 * \brief 设置缓冲区大小
			 *
			 * \author albert.xu
			 * \date 2017/08/10
			 */
			xgc_bool set_buffer_size( set_option option, xgc_size buffer_size );

			/*!
			*
			* \brief 设置缓冲区大小
			*
			* \author albert.xu
			* \date 2017/08/10
			*/
			xgc_void set_packet_max( set_option option, xgc_size packet_max );

			/*!
			 *
			 * \brief 等待连接到服务器
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void pending( xgc_lpvoid from );

			/*!
			 *
			 * \brief 连接到服务器
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_bool connect( xgc_lpcstr address, xgc_int16 port, connect_options *options );

			/*!
			 *
			 * \brief 接收连接
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void accept( int event );

			/*!
			 *
			 * \brief 发送消息
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void send( xgc_lpvoid data, xgc_size size );

			/*!
			*
			* \brief 合并消息并发送
			*
			* \author albert.xu
			* \date 十一月 2015
			*/
			xgc_void send( const std::list< std::tuple< xgc_lpvoid, xgc_size > >& buffers );

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
			xgc_ulong get_socket_info( xgc_uint32 addr[2], xgc_uint16 port[2] );

			/*!
			*
			* \brief 关闭连接
			*
			* \author albert.xu
			* \date 十一月 2015
			*/
			xgc_void make_event( xgc_uint32 event, xgc_uint32 error_code, xgc_lpvoid bring = xgc_nullptr );
		private:
			/*!
			*
			* \brief 连接到服务器
			*
			* \author albert.xu
			* \date 2017/07/18
			*/
			xgc_bool connect();

			/*!
			*
			* \brief 设置连接句柄
			*
			* \author albert.xu
			* \date 十一月 2015
			*/
			xgc_void set_handle( network_t handler )
			{
				handle_ = handler;
			}

			/*!
			 *
			 * \brief 处理连接建立事件
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
			xgc_void handle_timer( const asio::error_code& e );

		public:
			///
			/// \brief 获取来源信息
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:57
			///
			xgc_lpvoid get_from()const
			{
				return from_;
			}

			///
			/// \brief 获取连接句柄
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:58
			///
			network_t get_handle()const
			{
				return handle_;
			}

			///
			/// \brief 是否是连接状态
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:58
			///
			xgc_bool is_connected()const
			{
				return socket_.is_open() && connect_status_ == 1;
			}

			///
			/// \brief 判断从属关系
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:58
			///
			xgc_bool belong( xgc_lpvoid srv )
			{
				return srv == from_;
			}

			///
			/// \brief 设置用户数据
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:59
			///
			xgc_void set_userdata( xgc_lpvoid userdata )
			{
				userdata_ = userdata;
			}

			///
			/// \brief 获取用户数据
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:59
			///
			xgc_lpvoid get_userdata()const
			{
				return userdata_;
			}
		private:
			/// 连接信息
			struct connect_info
			{
				/// 连接等待时间（超时）
				xgc_uint16 timeout;

				/// 是否异步
				xgc_uint16 is_async : 1;
				/// 是否超时重连（连接不成功时重连）
				xgc_uint16 is_reconnect_timeout : 1;
				/// 是否被动重连（被动断开后重连）
				xgc_uint16 is_reconnect_passive : 1;
				// 连接地址
				asio::ip::tcp::endpoint address;
			};
			
			/// 连接信息
			connect_info		*connect_info_;

			/// 套接字
			ip::tcp::socket		socket_;
			/// 发送缓冲区锁
			std::mutex			send_buffer_lock;
			/// 发送缓冲
			asio_NetBuffer		send_buffer_;       
			/// 接收缓冲
			asio_NetBuffer		recv_buffer_;       

			/// 心跳发送间隔
			xgc_uint16			timeout_;
			
			/// 当前连接状态 0 - 未连接， 1 - 已连接， 2 - 等待关闭
			std::atomic_ushort  connect_status_;

			/// 当前投递的操作数
			std::atomic_ushort	operator_count_;

			/// 网络句柄
			volatile network_t	handle_;
			/// 来自哪里
			volatile xgc_lpvoid	from_;
			/// 用户数据
			volatile xgc_lpvoid	userdata_;
			
			/// 发送数据包最大尺寸
			volatile xgc_size send_packet_max_;
			/// 接收数据包最大尺寸
			volatile xgc_size recv_packet_max_;

			/// 保活定时器
			asio::steady_timer	timer_;
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
	};
};

#endif // _ASIO_NETWORK_HANDLER_H_