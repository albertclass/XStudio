#pragma once
#ifndef _ASIO_NETWORK_HANDLER_H_
#define _ASIO_NETWORK_HANDLER_H_
#include "asio_NetworkPacket.h"

struct IMessageQueue;
namespace XGC
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
		template< class > class CHandlerManager;
		struct asio_Socket : public std::enable_shared_from_this< asio_Socket >
		{
		public:
			asio_Socket( io_service& s, PacketProtocal* protocal, INetworkSession* holder, xgc_uint16 interval );

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
			xgc_void accept( xgc_uintptr from );

			/*!
			 *
			 * \brief 发送消息
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void send( const void* data, size_t size );

			/*!
			 *
			 * \brief 关闭连接
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_void close();

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

			/*!
			 *
			 * \brief 过滤包
			 *
			 * \author albert.xu
			 * \date 十一月 2015
			 */
			xgc_uint32 filter_packet( const xgc_byte *data, xgc_size size );

		public:
			ip::tcp::socket& socket()
			{
				return socket_;
			}

			PacketProtocal*	get_protocal()const
			{
				return protocal_;
			}
			xgc_lpvoid	get_userdata()const
			{
				return userdata_;
			}
			network_t	get_handler()const
			{
				return handler_;
			}

			xgc_void set_handler( network_t handler )
			{
				handler_ = handler;
			}
			xgc_void set_userdata( xgc_lpvoid userdata )
			{
				userdata_ = userdata; holder_->SetUserdata( userdata );
			}
			xgc_void set_disconnect()
			{
				disconnect_ = true;
			}

			xgc_bool is_connected()const
			{
				return network_ != 0;
			}
		private:
			ip::tcp::socket		socket_;           // 套接字
			std::mutex			send_buffer_lock;  // 发送缓冲区锁
			asio_NetworkPacket	send_buffer_;      // 发送缓冲
			asio_NetworkPacket	recv_buffer_;      // 接收缓冲
			asio_NetworkHandler	*network_;         // 句柄
			INetworkSession		*holder_;          // 数据处理句柄
			network_t			handler_;          // 网络句柄
			xgc_uint16			pingpang_inerval_; // 心跳发送间隔
			xgc_uint16			failed_count_;     // 失败次数
			xgc_bool			disconnect_;       // 是否连接断开
			xgc_lpvoid			userdata_;         // 用户数据
			deadline_timer		timer_;			   // 保活定时器
			PacketProtocal*		protocal_;		   // 协议句柄
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
		class asio_NetworkHandler
		{
			friend class CHandlerManager< asio_NetworkHandler >;
		public:
			enum Error_Enum
			{
				HANDLE_OPEN_ERROR,				//句柄打开错误
				HANDLE_READ_ERROR,				//读套接字错误
				HANDLE_WRITE_ERROR,				//写套接字错误
				HANDLE_PROCESS_PACKET_ERROR,	//数据包处理错误，这个多由逻辑返回错误，也可能是外挂攻击被捕获的错误
				HANDLE_SEND_COMPLETE_ERROR		//发送错误
			};
		public:
			asio_NetworkHandler( asio_SocketPtr& sock, xgc_uintptr from );

			~asio_NetworkHandler( xgc_void );

			//---------------------------------------------------//
			// [7/21/2009 Albert]
			// Description:	绑定用户数据
			//---------------------------------------------------//
			XGC_INLINE xgc_void SetUserdata( xgc_lpvoid pUserdata );

			//---------------------------------------------------//
			// [3/14/2012 Albert]
			// Description:	获取用户数据
			//---------------------------------------------------//
			XGC_INLINE xgc_lpvoid GetUserdata();

			//---------------------------------------------------//
			// [7/17/2009 Albert]
			// Description:	关闭网络
			//---------------------------------------------------//
			XGC_INLINE xgc_void CloseHandler();

			//////////////////////////////////////////////////////////////////////////
			// [7/17/2009 Albert]
			// 发送数据包
			//////////////////////////////////////////////////////////////////////////
			XGC_INLINE xgc_void SendPacket( const void *data, size_t size, xgc_bool lastone = false );

			//////////////////////////////////////////////////////////////////////////
			// [7/17/2009 Albert]
			// 获取连接信息
			//////////////////////////////////////////////////////////////////////////
			xgc_uintptr GetHandleInfo( xgc_int16 mask, xgc_byte* data );

			//---------------------------------------------------//
			// [11/11/2010 Albert]
			// Description:	设置ping值 
			//---------------------------------------------------//
			XGC_INLINE xgc_void SetHandlePing( xgc_uint32 nPingPong );

			//---------------------------------------------------//
			// [3/6/2012 Albert]
			// Description:	获取协议指针
			//---------------------------------------------------//
			XGC_INLINE PacketProtocal* GetProtocal() const;

			//---------------------------------------------------//
			// [3/14/2012 Albert]
			// Description:	检查附属
			//---------------------------------------------------//
			XGC_INLINE xgc_bool Belong( xgc_uintptr handle );

		protected:
			//---------------------------------------------------//
			// [7/17/2009 Albert]
			// Description:	锁定资源
			//---------------------------------------------------//
			xgc_void Fetch();

			//---------------------------------------------------//
			// [7/17/2009 Albert]
			// Description:	释放资源
			//---------------------------------------------------//
			xgc_void Free();

			///function	:	返回此连接句柄的编号
			///param	:	无
			///return	:	非0整数
			XGC_INLINE network_t GetHandler()const
			{
				if( m_pSocket )
					return m_pSocket->get_handler();

				return INVALID_NETWORK_HANDLE;
			}

			///function	:	设置连接句柄的编号，要求为非0整数
			///param	:	id	非0整数
			///return	:	无
			XGC_INLINE xgc_void SetHandler( network_t nHandler )
			{
				if( m_pSocket )
					m_pSocket->set_handler( nHandler );
			}

		private:
			asio_SocketPtr	m_pSocket;
			std::mutex		m_oCriResource;
			xgc_uint32		m_nPingPong;

			xgc_uintptr	_from;
		};

		XGC_INLINE xgc_void asio_NetworkHandler::SetUserdata( xgc_lpvoid pUserdata )
		{
			if( m_pSocket )
				m_pSocket->set_userdata( pUserdata );
		}

		XGC_INLINE xgc_lpvoid asio_NetworkHandler::GetUserdata()
		{
			if( m_pSocket )
				return m_pSocket->get_userdata();

			return xgc_nullptr;
		}

		//---------------------------------------------------//
		// [7/17/2009 Albert]
		// Description:	关闭句柄
		//---------------------------------------------------//
		XGC_INLINE xgc_void asio_NetworkHandler::CloseHandler()
		{
			m_pSocket->close();
		}

		XGC_INLINE xgc_void asio_NetworkHandler::SendPacket( const void *data, size_t size, xgc_bool lastone )
		{
			if( lastone )
				m_pSocket->set_disconnect();

			m_pSocket->send( data, size );
		}

		//---------------------------------------------------//
		// [7/17/2009 Albert]
		// Description:	锁定资源
		//---------------------------------------------------//
		XGC_INLINE xgc_void asio_NetworkHandler::Fetch()
		{
			m_oCriResource.lock();
		}

		//---------------------------------------------------//
		// [7/17/2009 Albert]
		// Description:	释放资源
		//---------------------------------------------------//
		XGC_INLINE xgc_void asio_NetworkHandler::Free()
		{
			m_oCriResource.unlock();
		}

		XGC_INLINE xgc_void asio_NetworkHandler::SetHandlePing( xgc_uint32 nPingPong )
		{
			m_nPingPong = nPingPong;
			// printf( "ping %d \n", nPingPong );
		}

		XGC_INLINE PacketProtocal* asio_NetworkHandler::GetProtocal() const
		{
			return m_pSocket->get_protocal();
		}

		//---------------------------------------------------//
		// [3/14/2012 Albert]
		// Description:	检查附属
		//---------------------------------------------------//
		XGC_INLINE xgc_bool asio_NetworkHandler::Belong( xgc_uintptr handle )
		{
			return handle == _from;
		}

	};
};

#endif // _ASIO_NETWORK_HANDLER_H_