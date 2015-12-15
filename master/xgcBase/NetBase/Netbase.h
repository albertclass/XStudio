#pragma  once
#ifndef _NETBASE_H
#define _NETBASE_H

#include "defines.h"
//#define _WIN32_WINNT 0x0601

#ifdef NETBASE_EXPORTS
#define NETBASE_API __declspec(dllexport)
#else
#define NETBASE_API __declspec(dllimport)
#endif

//网络通讯包相关
#define NETBASE								_T("Net")
#define MAX_PACKET_SIZE						(1024*16)
#define INVALID_NETWORK_HANDLE				(~0)
namespace XGC
{
	namespace net
	{
#pragma pack(1)

		//消息头定义
		struct MessageHeader
		{
			xgc_uint16 length;
			xgc_byte type;
			xgc_byte code;
		};

#pragma pack()

		struct PacketProtocal
		{
			xgc_size	(*header_space)();
			xgc_size	(*packet_length)( const xgc_byte* header );	// return data length include head length.
			xgc_uint32	(*packet_filter)( const xgc_byte* header );	// filter your interesting packet, return event code
			// event = EVENT_CODE { EVENT_ACCEPT, EVENT_CLOSE, EVENT_ERROR, EVENT_CONNECT, EVENT_PING, EVENT_PONG }
			xgc_size	(*packet_system)( xgc_byte event, xgc_byte* header ); // return packet write size;
			xgc_size	(*packet_finial)( xgc_byte* header, xgc_size length ); // set data length exclude head length.

			xgc_bool	(*packet_verifiy)( const xgc_byte* header, xgc_size size, xgc_lpvoid context); // verify packet data correctly

			//////////////////////////////////////////////////////////////////////////
			// decrypt packet return dest buffer need size if output is null
			// otherwise, return actual size of decrypt data size
			//////////////////////////////////////////////////////////////////////////
			xgc_uint32	(*packet_decrypt)( const void *data, xgc_size size, xgc_byte* output, xgc_lpvoid context); 

			//////////////////////////////////////////////////////////////////////////
			// encrypt packet return actual size, work in multil thread
			//////////////////////////////////////////////////////////////////////////
			xgc_uint32	(*packet_encrypt)( const void *data, xgc_size size, xgc_byte* output, xgc_lpvoid context); 
		};

		NETBASE_API extern PacketProtocal ProtocalDefault;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		#define SYSTEM_MESSAGE_TYPE	0
		#define EVENT_ACCEPT	0
		#define EVENT_CLOSE		1
		#define EVENT_CONNECT	2
		#define EVENT_ERROR		3
		#define EVENT_PING		4
		#define EVENT_PONG		5

		#define NET_ERROR_CONNET				0x87771000L
		#define NET_ERROR_CONNET_TIMEOUT		0x87771001L
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

		// return 成功 返回新的组ID -1 没有可用的组编号
		struct Param_NewGroup
		{
			network_t self_handle; 
		};
		#define Operator_NewGroup		0	

		// reutrn 0 成功 -1 指定的组已不存在 -2 网络句柄已被添加 -3 添加句柄到组失败
		struct Param_EnterGroup
		{
			group_t		group;
			network_t	handle;
		};
		#define Operator_EnterGroup		1	

		// return 0 成功 -1 指定的组已不存在 -2 网络句柄不存在
		struct Param_LeaveGroup
		{
			group_t		group;
			network_t	handle;
		};
		#define Operator_LeaveGroup		2	

		// return 0 成功 -1 指定的组已不存在
		struct Param_RemoveGroup
		{
			group_t group;
		};
		#define Operator_RemoveGroup	3	

		// return 0 成功 -1 失败
		struct Param_GetUserdata
		{
			network_t	handle;
			xgc_lpvoid	userdata_ptr;
		};
		#define Operator_GetUserdata	4

		// return 0 成功 -1 失败
		struct Param_SetUserdata
		{
			network_t	handle;
			xgc_lpvoid	userdata_ptr;
		};
		#define Operator_SetUserdata	5	

		// return 0 成功 -1 失败
		struct Param_SetTimeout
		{
			network_t handle;
			xgc_uint32 sec;
			xgc_uint32 msec;
		};
		#define Operator_SetTimeout		6

		// param [in]	in = { xgc_uint32 handle, xgc_uint32 type }; type 0 - local_ip, 1 - local_port, 2 - remont_ip, 3 - remont_port, 4 - connect_time
		// return -1 未找到指定的连接， -2 获取信息时出错
		struct Param_QueryHandleInfo
		{
			network_t	handle;
			xgc_int16	mask;
			xgc_byte	data[1];
		};
		#define NET_LOCAL_ADDRESS	1
		#define NET_REMOT_ADDRESS	2
		#define NET_LOCAL_PING		4
		#define Operator_QueryHandleInfo	7

		struct __declspec(novtable) INetPacket
		{
			virtual const xgc_byte*	header()const = 0;
			virtual const xgc_byte*	data()const = 0;
			virtual xgc_size		size()const = 0;
			virtual xgc_size		capacity()const = 0;
			virtual network_t		handle()const = 0;
			virtual xgc_lpvoid		userdata()const = 0;
			virtual time_t			timestamp()const = 0;
			virtual xgc_void		release() = 0;
		};

		struct __declspec(novtable) IMessageQueue
		{
			virtual xgc_bool PopMessage( INetPacket** ) = 0;
			virtual xgc_void PushMessage( INetPacket* ) = 0;

			virtual xgc_size Length()const = 0;

			virtual xgc_void Release() = 0;
		};

		struct __declspec(novtable) INetworkSession
		{
			INetworkSession()
				: m_pUserdata( NULL )
				, m_nLinkHandle( -1 )
			{

			}

			virtual ~INetworkSession()
			{

			}

			///function	:	连接关闭时的处理函数。重载此函数时需在做完应用需要做的事情后将连接删除或重置
			///param	:	无
			///return	:	0	成功	-1 失败
			virtual int OnClose( xgc_void ) = 0;

			///function	:	新的远端连接成功时的处理函数
			///param	:	new_handle	新连接的套接字
			///return	:	0	成功	-1 失败
			virtual int OnAccept( network_t handle, xgc_uintptr from ) = 0;

			///function	:	成功接受到网络数据包后提交到iocp上层应用处理的函数
			///param	:	data	接受到的数据	在中len范围内可靠有效
			///param	:	len		接受到数据的长度	此数据由iocp保证其真实性。
			///return	:	0	成功	-1 失败
			virtual int OnRecv( const void *data, size_t size ) = 0;

			///function	:	成功发送网络数据包后给iocp上层处理的函数，目前暂时未使用
			///param	:	data	成功发送的数据
			///param	:	len		成功发送的数据长度
			///return	:	0	成功	-1 失败
			virtual int OnSend( const void *data, size_t size ) = 0;

			///function	:	iocp捕获到错误并提供给应用的处理机会
			///param	:	error	错误代码
			///return	:	0	成功	-1 失败
			virtual int OnError( int error ) = 0;

			//--------------------------------------------------------//
			//	created:	26:1:2010   13:02
			//	filename: 	Netbase
			//	author:		Albert.xu
			//
			//	purpose:	用户数据
			//--------------------------------------------------------//
			const xgc_lpvoid GetUserdata()const{ return m_pUserdata; }

			//--------------------------------------------------------//
			//	created:	26:1:2010   13:03
			//	filename: 	Netbase
			//	author:		Albert.xu
			//
			//	purpose:	 设置用户数据
			//--------------------------------------------------------//
			xgc_void SetUserdata( xgc_lpvoid pUserdata ){ m_pUserdata = pUserdata; }

		protected:
			network_t		m_nLinkHandle;
			xgc_lpvoid		m_pUserdata;
		};

		typedef INetworkSession* (*create_handler_func)( xgc_uintptr lParam, const PacketProtocal* protocal_ptr );

		typedef int  (*pfnProcessor)( network_t, xgc_lpvoid, const xgc_byte*, xgc_size );

		enum eNetLibrary{ asio };
		extern "C"
		{
			NETBASE_API extern xgc_bool		(*InitNetwork)( int workthreads );
			NETBASE_API extern xgc_void		(*FiniNetwork)();
			NETBASE_API extern xgc_uintptr  (*StartServer)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, IMessageQueue** msg_queue_ptr );
			NETBASE_API extern xgc_uintptr  (*StartServerEx)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, create_handler_func call, xgc_uintptr param );
			NETBASE_API extern xgc_void		(*CloseServer)( xgc_uintptr server_h );
			NETBASE_API extern xgc_bool		(*ConnectServer)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, IMessageQueue** msg_queue );
			NETBASE_API extern xgc_bool		(*ConnectServerAsync)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, IMessageQueue** msg_queue );
			NETBASE_API extern xgc_lpvoid	(*AllocPacket)( xgc_size &size );
			NETBASE_API extern xgc_void		(*SendPacket)( network_t handle, const void* data, size_t size );
			NETBASE_API extern xgc_void		(*SendLastPacket)( network_t handle, const void* data, size_t size );
			NETBASE_API extern xgc_void		(*SendPackets)( network_t *handle, xgc_uint32 count, const void* data, size_t size );
			NETBASE_API extern xgc_void		(*SendToGroup)( group_t group, const void* data, size_t size, xgc_bool toself );
			NETBASE_API extern xgc_void		(*CloseLink)( network_t handle );
			NETBASE_API extern xgc_uintptr	(*ExecuteState)( xgc_uint32 operate_code, xgc_uintptr param );// 返回 0 表示成功, < 0 表示发生错误 > 0 表示可以正常执行,但有其他问题.

			// 服务器使用
			NETBASE_API xgc_bool	RegistFunction( xgc_byte type, xgc_byte id, pfnProcessor fn );
			NETBASE_API xgc_bool	UnRegistFunction( xgc_byte type, xgc_byte id );
			NETBASE_API xgc_int32	DispatchMessage( IMessageQueue* msg_queue );
			//--------------------------------------------------------//
			//	created:	3:12:2009   14:27
			//	filename: 	net
			//	author:		Albert.xu
			//
			//	purpose:	加载网络库
			//--------------------------------------------------------//
			NETBASE_API xgc_bool CreateNetwork( eNetLibrary );

			//--------------------------------------------------------//
			//	created:	3:12:2009   14:27
			//	filename: 	net
			//	author:		Albert.xu
			//
			//	purpose:	释放网络库
			//--------------------------------------------------------//
			NETBASE_API xgc_void DestroyNetwork();
		};
	}
};
#endif // _NETBASE_H