#pragma  once
#ifndef _NETBASE_H
#define _NETBASE_H

#include "defines.h"
#include "exports.h"

#include <memory>
#include <functional>
#define NETBASE_API COMMON_API

//网络通讯包相关
#define NETBASE								_T("Net")
#define MAX_PACKET_SIZE						(64*1024)
#define INVALID_NETWORK_HANDLE				(~0)
namespace xgc
{
	namespace net
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// 套接字消息
		#define SOCKET_MESSAGE_TYPE	0xff
		/// 通知消息
		#define NOTIFY_MESSAGE_TYPE	0xfe

		/// 连接建立事件
		#define EVENT_PENDING	0
		/// 连接建立事件
		#define EVENT_ACCEPT	1
		/// 连接建立事件
		#define EVENT_CONNECT	2
		/// 连接关闭事件
		#define EVENT_CLOSE		3
		/// 连接错误事件
		#define EVENT_ERROR		4
		/// 数据事件
		#define EVENT_DATA		5
		/// PING消息
		#define EVENT_PING		6
		/// PONG消息
		#define EVENT_PONG		7
		/// 计时器消息
		#define EVENT_TIMER		8

		#define NET_ETYPE_ACCEPT	1
		#define NET_ETYPE_CONNECT	2
		#define NET_ETYPE_SEND		3
		#define NET_ETYPE_RECV		4
		#define NET_ETYPE_CLOSE		5
		#define NET_ETYPE_TIMER		6

		#define NET_ERROR_RECONNECT			-1L
		#define NET_ERROR_TIMEOUT			-2L
		#define NET_ERROR_MESSAGE_SIZE		-3L
		#define NET_ERROR_NO_SPACE			-4L
		#define NET_ERROR_NO_MEMORY			-5L

		#define FILTER_ERROR					0
		#define FILTER_PASS						1
		#define FILTER_REFUSE					2
		#define FILTER_SYSTEM_PING				3
		#define FILTER_SYSTEM_PONG				4
		#define FILTER_SYSTEM_ENCRYPTION		5
		#define FILTER_SYSTEM_DECRYPTION		6

		/// 使用异步连接
		#define NET_CONNECT_OPTION_ASYNC		(1U)
		/// 自动重连
		#define NET_CONNECT_OPTION_RECONNECT	(2U)
		/// 使用连接超时
		#define NET_CONNECT_OPTION_TIMEOUT		(4U)

		typedef xgc_uint32	network_t;
		typedef xgc_uint32	group_t;

		///
		/// \brief 连接会话接口
		///
		/// \author albert.xu
		/// \date 2017/03/01 10:41
		///
		struct NETBASE_API INetworkSession
		{
			///
			/// \brief 数据包是否
			/// \return	0 ~ 成功, -1 ~ 失败
			///
			virtual int OnParsePacket( const void* data, xgc_size size ) = 0;

			///
			/// \brief 连接建立
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:09
			///
			virtual xgc_void OnAccept( net::network_t handle ) = 0;

			///
			/// \brief 连接建立
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:09
			///
			virtual xgc_void OnConnect( net::network_t handle ) = 0;

			///
			/// \brief 连接错误
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:09
			///
			virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code ) = 0;

			///
			/// \brief 连接关闭
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:10
			///
			virtual xgc_void OnClose() = 0;

			///
			/// \brief 接收数据
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:10
			///
			virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) = 0;

			///
			/// \brief 网络保活事件
			///
			/// \author albert.xu
			/// \date 2017/03/03 10:41
			///
			virtual xgc_void OnAlive() = 0;
		};

		///
		/// \brief 获取用户数据
		///
		/// \return 0 成功 -1 失败
		/// \author albert.xu
		/// \date 2016/02/26 16:12
		///
		struct Param_GetSession
		{
			/// [in] network handle
			network_t	handle;
			/// [out] session
			INetworkSession* session;
		};
		#define Operator_GetSession	1

		///
		/// \brief 查询套接字信息
		///
		/// \param [in]	in = { xgc_uint32 handle, xgc_uint32 type }; type 0 - local_ip, 1 - local_port, 2 - remont_ip, 3 - remont_port, 4 - connect_time
		/// \return -1 未找到指定的连接， -2 获取信息时出错
		/// \author albert.xu
		/// \date 2016/02/26 16:11
		///
		struct Param_QueryHandleInfo
		{
			/// [in] network handle
			network_t	handle;

			/// [out] 0 - local addr, 1 - remote addr
			xgc_uint32	addr[2];
			/// [out] 0 - local port, 1 - remote port
			xgc_uint16	port[2];
		};
		#define Operator_QueryHandleInfo 2

		///
		/// \brief 缓冲设置，只影响更改后创建的套接字
		///
		/// \author albert.xu
		/// \date 2016/02/26 16:08
		///
		struct Param_SetBufferSize
		{
			/// 发送缓冲大小
			xgc_uint32	send_buffer_size;
			/// 接收缓冲大小
			xgc_uint32	recv_buffer_size;
		};
		#define Operator_SetBufferSize 3

		///
		/// \brief 缓冲设置，只影响更改后创建的套接字
		///
		/// \author albert.xu
		/// \date 2016/02/26 16:08
		///
		struct Param_SetPacketSize
		{
			/// 发送缓冲大小
			xgc_uint32	send_packet_size;
			/// 接收缓冲大小
			xgc_uint32	recv_packet_size;
		};
		#define Operator_SetPacketSize 4

		///
		/// \brief 服务器参数
		///
		/// \author albert.xu
		/// \date 2017/08/10
		///
		struct server_options
		{
			/// 发送缓冲大小
			xgc_size send_buffer_size;
			/// 接收缓冲大小
			xgc_size recv_buffer_size;
			/// 发送包最大值
			xgc_size send_packet_max;
			/// 接收包最大值
			xgc_size recv_packet_max;
			/// 连接投递个数
			xgc_uint16 acceptor_count;
			/// 是否智能调优投递数量
			xgc_uint16 acceptor_smart;
			/// 心跳间隔
			xgc_uint16 heartbeat_interval;
			/// 地址数量
			xgc_size allow_count;
			/// 连接白名单
			struct allow_info
			{
				/// 允许的地址
				xgc_uint64 from;
				/// 允许的区间
				xgc_uint64 to;
			} allow_addr[16];
		};

		///
		/// \brief 连接参数
		///
		/// \author albert.xu
		/// \date 2017/08/10
		///
		struct connect_options
		{
			/// 发送缓冲大小
			xgc_size send_buffer_size;
			/// 接收缓冲大小
			xgc_size recv_buffer_size;
			/// 发送包最大值
			xgc_size send_packet_max;
			/// 接收包最大值
			xgc_size recv_packet_max;
			/// 连接等待时间（超时）
			xgc_uint16 timeout;
			/// 是否异步
			xgc_bool is_async;
			/// 是否超时重连（连接不成功时重连）
			xgc_bool is_reconnect_timeout;
			/// 是否被动重连（被动断开后重连）
			xgc_bool is_reconnect_passive;
		};

		/// 创建连接会话
		typedef std::function< INetworkSession* () > SessionCreator;

		/// 缓冲链定义
		typedef xgc::list< xgc::tuple< xgc_lpvoid, xgc_size > > BufferChains;

		/// 创建缓冲
		template< class T >
		xgc::tuple< xgc_lpvoid, xgc_size > MakeBuffer( T &value ) { return { &value, sizeof( value ) }; }
		XGC_INLINE
		xgc::tuple< xgc_lpvoid, xgc_size > MakeBuffer( xgc_lpcvoid data, xgc_size size ) { return { ( xgc_lpvoid )data, size }; }

		extern "C"
		{
			/// 
			/// 
			/// \brief 开启服务器
			/// 
			/// \author albert.xu
			/// \date 十一月 2015
			/// 
			NETBASE_API xgc_lpvoid StartServer( xgc_lpcstr address, xgc_uint16 port, server_options *options, SessionCreator creator );

			///
			/// \brief 关闭服务器
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:00
			///
			NETBASE_API xgc_void CloseServer( xgc_lpvoid server );

			///
			/// \brief 连接到服务器
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:00
			///
			NETBASE_API network_t Connect( xgc_lpcstr address, xgc_uint16 port, INetworkSession* session, connect_options *options );

			///
			/// \brief 发送消息包
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPacket( network_t handle, xgc_lpvoid data, xgc_size size );

			///
			/// \brief 发送消息包
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPacketChains( network_t handle, const BufferChains& buffers );

			///
			/// \brief 发送一组消息包
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size );

			///
			/// \brief 关闭连接
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void CloseLink( network_t handle );

			///
			/// \brief 关闭连接
			///
			/// \author albert.xu
			/// \date 2017/05/24 15:06
			///
			NETBASE_API xgc_void NewTimer( xgc_uint32 id, xgc_real64 period, xgc_real64 after, const std::function< void() > &on_timer );

			///
			/// \brief 关闭连接
			///
			/// \author albert.xu
			/// \date 2017/05/24 15:06
			///
			NETBASE_API xgc_void DelTimer( xgc_uint32 id );

			///
			/// \brief 处理网络事件
			///
			/// \author albert.xu
			/// \date 2017/03/02 13:54
			///
			NETBASE_API xgc_long ProcessNetEvent( xgc_long step );

			///
			/// \brief 设置网络状态
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_ulong ExecuteState( xgc_uint32 operate_code, xgc_lpvoid param );

			///
			/// \brief 加载网络库
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:25
			///
			NETBASE_API xgc_bool CreateNetwork( int workthreads );

			///
			/// \brief 销毁网络库
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:25
			///
			NETBASE_API xgc_void DestroyNetwork();
		};
	}
};
#endif // _NETBASE_H