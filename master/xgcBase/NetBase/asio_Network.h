#ifndef _NETWORK_H_
#define _NETWORK_H_
#pragma once
#include "defines.h"
#include "asio_Header.h"
#include <vector>

namespace XGC
{
	namespace net
	{
		class asio_NetworkPacket;
		class asio_Network
		{
			friend asio_Network& getNetwork();

		private:
			asio_Network(void);
			~asio_Network(void);

			xgc_void run();	
		public:
			xgc_bool insert_workthread( xgc_int32 c );
			xgc_void exit();

			asio::io_service& Ref(){ return *service_; }
		private:
			asio::io_service *service_;
			std::vector< std::shared_ptr< asio::thread > >	workthreads_;
		};

		// get asio network
		asio_Network& getNetwork();

		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	��ʼ������
		//---------------------------------------------------//
		xgc_bool asio_InitNetwork( int workthreads );


		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	�ر�����
		//---------------------------------------------------//
		xgc_void asio_FiniNetwork();


		/*!
		 * [7/15/2009 Albert]
		 * \param	Description:	����������
		 * \param	address:		������������ַ
		 * \param	port:			�����������˿�
		 * \param	msg_queue:		��������Ϣ����

		 * \return server ������رշ�����ʱ��Ҫ�ṩ��Ӧ���
		 */
		xgc_uintptr asio_StartServer( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, IMessageQueue** msg_queue_ptr );

		/*!
		 *
		 * \brief ����������
		 *
		 * \author albert.xu
		 * \date ʮһ�� 2015
		 */
		xgc_uintptr asio_StartServerEx( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 ping_invent, xgc_uint16 timeout, xgc_uint16 acceptor_count, create_handler_func call, xgc_uintptr param );

		//---------------------------------------------------//
		// [7/15/2009 Albert]
		// Description:	�رշ�����
		//---------------------------------------------------//
		xgc_void asio_CloseServer( xgc_uintptr server_h );


		xgc_bool asio_ConnectServer( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, IMessageQueue** msg_queue );


		xgc_bool asio_ConnectServerAsync( xgc_lpcstr address, xgc_uint16 port, PacketProtocal &protocal, xgc_uint16 timeout, IMessageQueue** msg_queue );


		xgc_void asio_SendPacket( network_t handle, const void* data, size_t size );


		xgc_void asio_SendLastPacket( network_t handle, const void* data, size_t size );


		xgc_void asio_SendPackets( network_t *handle, xgc_uint32 count, const void* data, size_t size );


		xgc_void asio_SendToGroup( group_t group, const void* data, size_t size, xgc_bool toself );


		xgc_void asio_CloseLink( network_t handle );


		xgc_uintptr asio_ExecuteState( xgc_uint32 operate_code, xgc_uintptr param );

	}
}
#endif // _NETWORK_H_