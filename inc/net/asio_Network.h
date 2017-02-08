///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file asio_Network.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ����API��װ
///
///////////////////////////////////////////////////////////////
#ifndef _NETWORK_H_
#define _NETWORK_H_
#pragma once

namespace xgc
{
	namespace net
	{
		///
		/// \brief ����������װ��
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:58
		///
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

		///
		/// \brief get asio network
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		asio_Network& getNetwork();

		///
		/// \brief ��ʼ������
		///
		/// \param workthreads �����������߳���
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		xgc_bool asio_InitNetwork( int workthreads );


		///
		/// \brief �ر�����
		///
		/// \author albert.xu
		/// \date 2016/02/17 15:59
		///
		xgc_void asio_FiniNetwork();


		/// 
		/// \brief	����������
		/// \param	address ������������ַ
		/// \param	port �����������˿�
		/// \param	msg_queue ��������Ϣ����
		/// 
		/// \return server ������رշ�����ʱ��Ҫ�ṩ��Ӧ���
		/// 
		xgc_uintptr asio_StartServer( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );

		/// 
		/// 
		/// \brief ����������
		/// 
		/// \author albert.xu
		/// \date ʮһ�� 2015
		/// 
		xgc_uintptr asio_StartServerEx( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );

		///
		/// \brief �رշ�����
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_void asio_CloseServer( xgc_uintptr server_h );

		///
		/// \brief ���ӵ�������
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_Connect( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );

		///
		/// \brief �첽���ӵ�������
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_ConnectAsync( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, MessageQueuePtr &queue_ptr );

		///
		/// \brief ���ӵ�������
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_ConnectEx( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );

		///
		/// \brief �첽���ӵ�������
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:00
		///
		xgc_bool asio_ConnectAsyncEx( xgc_lpcstr address, xgc_uint16 port, xgc_uint16 timeout, const pfnCreateHolder &creator );
		///
		/// \brief ������Ϣ��
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendPacket( network_t handle, xgc_lpvoid data, xgc_size size );

		///
		/// \brief ������Ϣ�����Ͽ���������
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendLastPacket( network_t handle, xgc_lpvoid data, xgc_size size );

		///
		/// \brief ����һ����Ϣ��
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendPackets( network_t *handle, xgc_uint32 count, xgc_lpvoid data, xgc_size size );

		///
		/// \brief ���͵�һ����
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_SendToGroup( group_t group, xgc_lpvoid data, xgc_size size, xgc_bool toself );

		///
		/// \brief �ر�����
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_void asio_CloseLink( network_t handle );

		///
		/// \brief ��������״̬
		///
		/// \author albert.xu
		/// \date 2016/02/17 16:01
		///
		xgc_uintptr asio_ExecuteState( xgc_uint32 operate_code, xgc_uintptr param );

	}
}
#endif // _NETWORK_H_