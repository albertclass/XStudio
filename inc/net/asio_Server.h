///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file asio_Server.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// �������������
///
///////////////////////////////////////////////////////////////
#pragma once
#ifndef _ASIO_SERVER_H_
#define _ASIO_SERVER_H_

namespace xgc
{
	namespace net
	{
		using namespace asio;

		///
		/// \brief �������������
		///
		/// \author albert.xu
		/// \date 2016/02/26 15:49
		///
		class asio_ServerBase
		{
		public:
			///
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			asio_ServerBase( io_service& service_, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 time_invent, xgc_uint16 timeout );

			///
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			virtual ~asio_ServerBase();

			///
			/// \brief ����������
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			xgc_bool StartServer( const char *address, xgc_int16 port );

			///
			/// \brief ֹͣ������
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			xgc_void StopServer( xgc_bool bCloseAllLink = true );

		protected:
			///
			/// \brief ��������
			///
			/// \author albert.xu
			/// \date 2016/02/26 16:01
			///
			xgc_void handle_accept( asio_SocketPtr pSocket, const asio::error_code& error);

			///
			/// \brief Ͷ����������
			///
			/// \author albert.xu
			/// \date 2016/02/26 16:01
			///
			xgc_void post_accept();

		private:
			///
			/// \brief ��������Ự
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:48
			///
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal ) = 0;
		protected:
			io_service&			service_;
			ip::tcp::acceptor	acceptor_;
			xgc_uint16			time_invent_;
			xgc_uint16			timeout_;
			xgc_uint16			acceptor_count_;
			PacketProtocal*		protocal_;
		};

		///
		/// \brief һ�����������
		///
		/// \author albert.xu
		/// \date 2016/02/26 15:49
		///
		class asio_Server : public asio_ServerBase
		{
		public:
			asio_Server( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, MessageQueuePtr queue_ptr );
			~asio_Server();

		protected:
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal );

		private:
			MessageQueuePtr	queue_ptr_;
		};

		///
		/// \brief �߼�����������
		///
		/// \author albert.xu
		/// \date 2016/02/26 15:50
		///
		class asio_ServerEx	:	public asio_ServerBase
		{
		public:
			asio_ServerEx( io_service& service, PacketProtocal& protocal, xgc_uint16 acceptor_count, xgc_uint16 ping_invent, xgc_uint16 timeout, create_handler_func call, xgc_uintptr param );
			~asio_ServerEx();

		protected:
			virtual INetworkSession* CreateHolder( PacketProtocal* protocal );

		private:
			create_handler_func call_;
			xgc_uintptr param_;
		};

	}
}
#endif // _ASIO_SERVER_H_