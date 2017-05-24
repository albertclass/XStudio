#pragma  once
#ifndef _NETBASE_H
#define _NETBASE_H

#include "defines.h"
#include "exports.h"

#include <memory>
#include <functional>
#define NETBASE_API COMMON_API

//����ͨѶ�����
#define NETBASE								_T("Net")
#define MAX_PACKET_SIZE						(64*1024)
#define INVALID_NETWORK_HANDLE				(~0)
namespace xgc
{
	namespace net
	{
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		/// �׽�����Ϣ
		#define SOCKET_MESSAGE_TYPE	0xff
		/// ֪ͨ��Ϣ
		#define NOTIFY_MESSAGE_TYPE	0xfe

		/// ���ӽ����¼�
		#define EVENT_HANGUP	0
		/// ���ӽ����¼�
		#define EVENT_ACCEPT	1
		/// ���ӽ����¼�
		#define EVENT_CONNECT	2
		/// ���ӹر��¼�
		#define EVENT_CLOSE		3
		/// ���Ӵ����¼�
		#define EVENT_ERROR		4
		/// �����¼�
		#define EVENT_DATA		5
		/// PING��Ϣ
		#define EVENT_PING		6
		/// PONG��Ϣ
		#define EVENT_PONG		7
		/// ��ʱ����Ϣ
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
		/// \brief ���ݰ��ӿ�
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:19
		///
		struct INetPacket
		{
			///
			/// \brief �����׵�ַ
			///
			/// \author albert.xu
			/// \date 2016/02/24 11:40
			///
			virtual xgc_lpvoid	data() = 0;

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
			virtual xgc_void	freedom() throw() = 0;
		};

		///
		/// \brief ���ӻỰ�ӿ�
		///
		/// \author albert.xu
		/// \date 2017/03/01 10:41
		///
		struct NETBASE_API INetworkSession
		{
			///
			/// \brief ���ݰ��Ƿ�
			/// \return	0 ~ �ɹ�, -1 ~ ʧ��
			///
			virtual int OnParsePacket( const void* data, xgc_size size ) = 0;

			///
			/// \brief ���ӽ���
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:09
			///
			virtual xgc_void OnAccept( net::network_t handle ) = 0;

			///
			/// \brief ���ӽ���
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:09
			///
			virtual xgc_void OnConnect( net::network_t handle ) = 0;

			///
			/// \brief ���Ӵ���
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:09
			///
			virtual xgc_void OnError( xgc_uint32 error_code ) = 0;

			///
			/// \brief ���ӹر�
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:10
			///
			virtual xgc_void OnClose() = 0;

			///
			/// \brief ��������
			///
			/// \author albert.xu
			/// \date 2017/02/28 11:10
			///
			virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size ) = 0;

			///
			/// \brief ���籣���¼�
			///
			/// \author albert.xu
			/// \date 2017/03/03 10:41
			///
			virtual xgc_void OnAlive() = 0;
		};

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
		struct Param_GetSession
		{
			network_t	handle;
			INetworkSession* session;
		};
		#define Operator_GetSession	4

		///
		/// \brief �����û�����
		///
		/// \return 0 �ɹ� -1 ʧ��
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
		/// \brief �������ã�ֻӰ����ĺ󴴽����׽���
		///
		/// \author albert.xu
		/// \date 2016/02/26 16:08
		///
		struct Param_SetPacketSize
		{
			/// ���ͻ����С
			xgc_uint32	send_packet_size;
			/// ���ջ����С
			xgc_uint32	recv_packet_size;
		};
		#define Operator_SetPacketSize	9

		/// �������ӻỰ
		typedef std::function< INetworkSession* () > SessionCreator;

		/// ����������
		typedef std::list< std::tuple< xgc_lpvoid, xgc_size > > BufferChains;

		extern "C"
		{
			/// 
			/// 
			/// \brief ����������
			/// 
			/// \author albert.xu
			/// \date ʮһ�� 2015
			/// 
			NETBASE_API xgc_lpvoid StartServer( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, SessionCreator creator );

			///
			/// \brief �رշ�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:00
			///
			NETBASE_API xgc_void CloseServer( xgc_lpvoid server );

			///
			/// \brief ���ӵ�������
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:00
			///
			NETBASE_API xgc_bool Connect( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, INetworkSession* session );

			///
			/// \brief �첽���ӵ�������
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:00
			///
			NETBASE_API xgc_bool ConnectAsync( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, INetworkSession* session );

			///
			/// \brief ������Ϣ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPacket( network_t handle, xgc_lpvoid data, xgc_size size );

			///
			/// \brief ������Ϣ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPacketChains( network_t handle, const BufferChains& buffers );

			///
			/// \brief ������Ϣ�����Ͽ���������
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendLastPacket( network_t handle, xgc_lpvoid data, xgc_size size );

			///
			/// \brief ����һ����Ϣ��
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size );

			///
			/// \brief ���͵�һ����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void SendToGroup( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself );

			///
			/// \brief �ر�����
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_void CloseLink( network_t handle );

			///
			/// \brief �ر�����
			///
			/// \author albert.xu
			/// \date 2017/05/24 15:06
			///
			NETBASE_API xgc_void SetTimer( xgc_uint32 id, xgc_real64 period, xgc_real64 after, const std::function< void() > &on_timer );

			///
			/// \brief �ر�����
			///
			/// \author albert.xu
			/// \date 2017/05/24 15:06
			///
			NETBASE_API xgc_void DelTimer( xgc_uint32 id );

			///
			/// \brief ���������¼�
			///
			/// \author albert.xu
			/// \date 2017/03/02 13:54
			///
			NETBASE_API xgc_long ProcessNetEvent( xgc_long step );

			///
			/// \brief ��������״̬
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:01
			///
			NETBASE_API xgc_ulong ExecuteState( xgc_uint32 operate_code, xgc_lpvoid param );

			///
			/// \brief ���������
			///
			/// \author albert.xu
			/// \date 2016/02/17 16:25
			///
			NETBASE_API xgc_bool CreateNetwork( int workthreads );

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