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
		#define EVENT_HANGUP	0
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

		#define NET_ERROR_CONNECT				0x87771000L
		#define NET_ERROR_CONNECT_TIMEOUT		0x87771001L
		#define NET_ERROR_NOT_ENOUGH_MEMROY		0x87771002L
		#define NET_ERROR_HEADER_LENGTH			0x87771003L
		#define NET_ERROR_DECRYPT_LENGTH		0x87771004L
		#define NET_ERROR_PACKET_SPACE			0x87771005L
		#define NET_ERROR_PACKET_INVALID		0x87771006L
		#define NET_ERROR_SEND_BUFFER_FULL		0x87771007L

		#define FILTER_ERROR					0
		#define FILTER_PASS						1
		#define FILTER_REFUSE					2
		#define FILTER_SYSTEM_PING				3
		#define FILTER_SYSTEM_PONG				4
		#define FILTER_SYSTEM_ENCRYPTION		5
		#define FILTER_SYSTEM_DECRYPTION		6

		typedef xgc_uint32	network_t;
		typedef xgc_uint32	group_t;

		///
		/// \brief 数据包接口
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:19
		///
		struct INetPacket
		{
			///
			/// \brief 数据首地址
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_lpvoid	data() = 0;

			///
			/// \brief 数据长度
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size	size()const = 0;

			///
			/// \brief 数据包总容量
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size	capacity()const = 0;

			///
			/// \brief 连接句柄
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual network_t	handle()const = 0;

			///
			/// \brief 消息包创建时的时间戳
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:41
			///
			virtual time_t		timestamp()const = 0;

			///
			/// \brief 释放消息包
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:46
			///
			virtual xgc_void	freedom() throw() = 0;
		};

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
			virtual xgc_void OnError( xgc_uint32 error_code ) = 0;

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
		/// \brief 新建组
		///
		/// \return 成功 返回新的组ID -1 没有可用的组编号
		/// \author albert.xu
		/// \date 2016/02/26 16:13
		///
		struct Param_NewGroup
		{
			network_t self_handle;
		};
		#define Operator_NewGroup		0	

		///
		/// \brief 加入组
		///
		/// \return 0 成功 -1 指定的组已不存在 -2 网络句柄已被添加 -3 添加句柄到组失败
		/// \author albert.xu
		/// \date 2016/02/26 16:13
		///
		struct Param_EnterGroup
		{
			group_t		group;
			network_t	handle;
		};
		#define Operator_EnterGroup		1	

		///
		/// \brief 离开组
		///
		/// \return 0 成功 -1 指定的组已不存在 -2 网络句柄不存在
		/// \author albert.xu
		/// \date 2016/02/26 16:13
		///
		struct Param_LeaveGroup
		{
			group_t		group;
			network_t	handle;
		};
		#define Operator_LeaveGroup		2	

		///
		/// \brief 移除组
		///
		/// \return 0 成功 -1 指定的组已不存在
		/// \author albert.xu
		/// \date 2016/02/26 16:13
		///
		struct Param_RemoveGroup
		{
			group_t group;
		};
		#define Operator_RemoveGroup	3	

		///
		/// \brief 获取用户数据
		///
		/// \return 0 成功 -1 失败
		/// \author albert.xu
		/// \date 2016/02/26 16:12
		///
		struct Param_GetSession
		{
			network_t	handle;
			INetworkSession* session;
		};
		#define Operator_GetSession	4

		///
		/// \brief 设置用户数据
		///
		/// \return 0 成功 -1 失败
		/// \author albert.xu
		/// \date 2016/02/26 16:12
		///
		struct Param_SetSession
		{
			network_t	handle;
			INetworkSession* session;
		};
		#define Operator_SetSession	5

		///
		/// \brief 设置超时时间
		///
		/// \return 0 成功 -1 失败
		/// \author albert.xu
		/// \date 2016/02/26 16:12
		///
		struct Param_SetTimeout
		{
			network_t handle;
			xgc_uint32 sec;
			xgc_uint32 msec;
		};
		#define Operator_SetTimeout		6

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
			network_t	handle;
			xgc_int16	mask;
			xgc_byte	data[1];

			struct endpoint
			{
				xgc_uint32 addr;
				xgc_uint16 port;
			};
		};
		#define NET_LOCAL_ADDRESS	1
		#define NET_REMOT_ADDRESS	2
		#define NET_LOCAL_PING		4
		#define Operator_QueryHandleInfo	7

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
		#define Operator_SetBufferSize	8

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
		#define Operator_SetPacketSize	9

		/// 创建连接会话
		typedef std::function< INetworkSession* () > SessionCreator;

		/// 缓冲链定义
		typedef std::list< std::tuple< xgc_lpvoid, xgc_size > > BufferChains;

		extern "C"
		{
			/// 
			/// 
			/// \brief 开启服务器
			/// 
			/// \author albert.xu
			/// \date 十一月 2015
			/// 
			NETBASE_API xgc_lpvoid StartServer( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, SessionCreator creator );

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
			NETBASE_API xgc_bool Connect( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, INetworkSession* session );

			///
			/// \brief 异步连接到服务器
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:00
			///
			NETBASE_API xgc_bool ConnectAsync( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, INetworkSession* session );

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
			/// \brief 发送消息包并断开网络连接
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendLastPacket( network_t handle, xgc_lpvoid data, xgc_size size );

			///
			/// \brief 发送一组消息包
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size );

			///
			/// \brief 发送到一个组
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendToGroup( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself );

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
			NETBASE_API xgc_void SetTimer( xgc_uint32 id, xgc_real64 period, xgc_real64 after, const std::function< void() > &on_timer );

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