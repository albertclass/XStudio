#pragma  once
#ifndef _NETBASE_H
#define _NETBASE_H

#include "defines.h"
#include "exports.h"

#include <memory>
#define NETBASE_API COMMON_API

//����ͨѶ�����
#define NETBASE								_T("Net")
#define MAX_PACKET_SIZE						(64*1024)
#define INVALID_NETWORK_HANDLE				(~0)
namespace xgc
{
	namespace net
	{
#pragma pack(1)

		//��Ϣͷ����
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
			// return data length include head length.
			xgc_size	(*packet_length)( xgc_lpstr header );
			// filter your interesting packet, return event code
			xgc_uint32	(*packet_filter)( xgc_lpstr header );
			// event = EVENT_CODE { EVENT_ACCEPT, EVENT_CLOSE, EVENT_ERROR, EVENT_CONNECT, EVENT_PING, EVENT_PONG }
			// return packet write size;
			xgc_size	(*packet_system)( xgc_byte event, xgc_lpstr header ); 
			// set data length exclude head length.
			xgc_size	(*packet_finial)( xgc_lpstr header, xgc_size length );
			// verify packet data correctly
			xgc_bool	(*packet_verifiy)( xgc_lpstr header, xgc_size size, xgc_lpvoid context);

			//////////////////////////////////////////////////////////////////////////
			// decrypt packet return dest buffer need size if output is null
			// otherwise, return actual size of decrypt data size
			//////////////////////////////////////////////////////////////////////////
			xgc_uint32	(*packet_decrypt)( xgc_lpstr data, xgc_size size, xgc_byte* output, xgc_lpvoid context); 

			//////////////////////////////////////////////////////////////////////////
			// encrypt packet return actual size, work in multil thread
			//////////////////////////////////////////////////////////////////////////
			xgc_uint32	(*packet_encrypt)( xgc_lpstr data, xgc_size size, xgc_byte* output, xgc_lpvoid context); 
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
		/// \brief �½���
		///
		/// \return �ɹ� �����µ���ID -1 û�п��õ�����
		/// \author albert.xu
		/// \date 2016/02/26 16:13
		///
		struct Param_NewGroup
		{
			network_t self_handle; 
		};
		#define Operator_NewGroup		0	

		///
		/// \brief ������
		///
		/// \return 0 �ɹ� -1 ָ�������Ѳ����� -2 �������ѱ���� -3 ��Ӿ������ʧ��
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
		/// \brief �뿪��
		///
		/// \return 0 �ɹ� -1 ָ�������Ѳ����� -2 ������������
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
		/// \brief �Ƴ���
		///
		/// \return 0 �ɹ� -1 ָ�������Ѳ�����
		/// \author albert.xu
		/// \date 2016/02/26 16:13
		///
		struct Param_RemoveGroup
		{
			group_t group;
		};
		#define Operator_RemoveGroup	3	

		///
		/// \brief ��ȡ�û�����
		///
		/// \return 0 �ɹ� -1 ʧ��
		/// \author albert.xu
		/// \date 2016/02/26 16:12
		///
		struct Param_GetUserdata
		{
			network_t	handle;
			xgc_lpvoid	userdata_ptr;
		};
		#define Operator_GetUserdata	4

		///
		/// \brief �����û�����
		///
		/// \return 0 �ɹ� -1 ʧ��
		/// \author albert.xu
		/// \date 2016/02/26 16:12
		///
		struct Param_SetUserdata
		{
			network_t	handle;
			xgc_lpvoid	userdata_ptr;
		};
		#define Operator_SetUserdata	5	

		///
		/// \brief ���ó�ʱʱ��
		///
		/// \return 0 �ɹ� -1 ʧ��
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
		/// \brief ��ѯ�׽�����Ϣ
		///
		/// \param [in]	in = { xgc_uint32 handle, xgc_uint32 type }; type 0 - local_ip, 1 - local_port, 2 - remont_ip, 3 - remont_port, 4 - connect_time
		/// \return -1 δ�ҵ�ָ�������ӣ� -2 ��ȡ��Ϣʱ����
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
		/// \brief �������ã�ֻӰ����ĺ󴴽����׽���
		///
		/// \author albert.xu
		/// \date 2016/02/26 16:08
		///
		struct Param_SetBufferSize
		{
			/// ���ͻ����С
			xgc_uint32	send_buffer_size;
			/// ���ջ����С
			xgc_uint32	recv_buffer_size;
		};
		#define Operator_SetBufferSize	8

		///
		/// \brief ���ݰ��ӿ�
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:19
		///
		struct INetPacket
		{
			///
			/// \brief ���ݰ��׵�ַ
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_lpcstr	header()const = 0;

			///
			/// \brief ���ݰ�����
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size	length()const = 0;

			///
			/// \brief �����׵�ַ
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_lpcstr	data()const = 0;

			///
			/// \brief ���ݳ���
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size	size()const = 0;

			///
			/// \brief ���ݰ�������
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_size	capacity()const = 0;

			///
			/// \brief ���Ӿ��
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual network_t	handle()const = 0;

			///
			/// \brief �û�����
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:41
			///
			virtual xgc_lpvoid	userdata()const = 0;

			///
			/// \brief ��Ϣ������ʱ��ʱ���
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:41
			///
			virtual time_t		timestamp()const = 0;

			///
			/// \brief �ͷ���Ϣ��
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:46
			///
			virtual xgc_void	release() throw() = 0;
		};

		///
		/// \brief ��Ϣ���нӿ�
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:19
		///
		struct IMessageQueue
		{
			virtual xgc_bool PopMessage( INetPacket** ) = 0;
			virtual xgc_void PushMessage( INetPacket* ) = 0;

			virtual xgc_size Length()const = 0;

			virtual xgc_void Release() = 0;
		};

		typedef std::shared_ptr< IMessageQueue > MessageQueuePtr;

		///
		/// \brief ����Ự�ӿ�
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:20
		///
		struct INetworkSession
		{
			INetworkSession()
				: userdata_( NULL )
				, handle_( -1 )
			{

			}

			virtual ~INetworkSession()
			{

			}

			///
			/// \brief ���ӹر�ʱ�Ĵ����������ش˺���ʱ��������Ӧ����Ҫ�������������ɾ��������
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnClose() = 0;

			///
			/// \brief �µ�Զ�����ӳɹ�ʱ�Ĵ�����
			/// \param	new_handle	�����ӵ��׽���
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnAccept( network_t handle, xgc_lpvoid from ) = 0;

			///
			/// \brief �ɹ����ܵ��������ݰ����ύ��iocp�ϲ�Ӧ�ô���ĺ���
			/// \param	data ���ܵ�������,����len��Χ�ڿɿ���Ч
			/// \param	len	���ܵ����ݵĳ���,��������iocp��֤����ʵ�ԡ�
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnRecv( const void *data, size_t size ) = 0;

			///
			/// \brief �ɹ������������ݰ����iocp�ϲ㴦��ĺ�����Ŀǰ��ʱδʹ��
			/// \param data �ɹ����͵�����
			/// \param len �ɹ����͵����ݳ���
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnSend( const void *data, size_t size ) = 0;

			///
			/// \brief iocp���񵽴����ṩ��Ӧ�õĴ������
			/// \param error �������
			/// \return 0 ~ �ɹ�, -1 ~ ʧ��
			virtual int OnError( int error ) = 0;

			///
			/// \brief ��ȡ�û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			const xgc_lpvoid GetUserdata()const
			{ 
				return userdata_; 
			}

			///
			/// \brief �����û�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:23
			///
			xgc_void SetUserdata( xgc_lpvoid pUserdata )
			{ 
				userdata_ = pUserdata; 
			}

		protected:
			/// ���Ӿ��
			network_t		handle_;
			/// �û�����
			xgc_lpvoid		userdata_;
		};

		/// ����Ự��������ָ������
		typedef INetworkSession* (*create_handler_func)( xgc_uintptr lParam, const PacketProtocal* protocal_ptr );

		/// ��Ϣ������ָ������
		typedef xgc_void (*pfnProcessor)( network_t, xgc_lpcstr, xgc_size, xgc_lpvoid );

		enum eNetLibrary{ asio };
		extern "C"
		{
			NETBASE_API extern xgc_bool		(*InitNetwork)( int workthreads );
			NETBASE_API extern xgc_void		(*FiniNetwork)();
			NETBASE_API extern xgc_uintptr  (*StartServer)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, MessageQueuePtr &queue_ptr );
			NETBASE_API extern xgc_uintptr  (*StartServerEx)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, create_handler_func call, xgc_uintptr param );
			NETBASE_API extern xgc_void		(*CloseServer)( xgc_uintptr server_h );
			NETBASE_API extern xgc_bool		(*ConnectServer)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );
			NETBASE_API extern xgc_bool		(*ConnectServerAsync)( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );
			
			NETBASE_API extern xgc_void		(*SendPacket)( network_t handle, xgc_lpvoid data, xgc_size size );
			NETBASE_API extern xgc_void		(*SendLastPacket)( network_t handle, xgc_lpvoid data, xgc_size size );
			NETBASE_API extern xgc_void		(*SendPackets)( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size );
			NETBASE_API extern xgc_void		(*SendToGroup)( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself );
			NETBASE_API extern xgc_void		(*CloseLink)( network_t handle );
			NETBASE_API extern xgc_uintptr	(*ExecuteState)( xgc_uint32 operate_code, xgc_uintptr param );// ���� 0 ��ʾ�ɹ�, < 0 ��ʾ�������� > 0 ��ʾ��������ִ��,������������.

			// ������ʹ��
			NETBASE_API xgc_void InstallDeliver( xgc_byte type, xgc_byte id, pfnProcessor fn );
			NETBASE_API xgc_long DeliverMessage( MessageQueuePtr pQueue, xgc_long nCount );

			///
			/// \brief ���������
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:25
			///
			NETBASE_API xgc_bool CreateNetwork( eNetLibrary );

			///
			/// \brief ���������
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:25
			///
			NETBASE_API xgc_void DestroyNetwork();
		};
	}
};
#endif // _NETBASE_H