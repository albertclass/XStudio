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
		class asio_Server
		{
		public:
			///
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			asio_Server( io_service& service_, SessionCreator creator, server_options *options );

			///
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:49
			///
			virtual ~asio_Server();

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
			xgc_void StopServer();

			///
			/// \brief �������ӻỰ
			///
			/// \author albert.xu
			/// \date 2017/03/02 15:01
			///
			INetworkSession* CreateSession()
			{
				return creator_();
			}
		protected:
			///
			/// \brief �Ƿ��ڰ�������
			///
			/// \author albert.xu
			/// \date 2017/08/22
			///
			xgc_bool allow( xgc_uint32 addr );

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

		protected:
			/// ����������
			io_service&			service_;
			/// ����
			ip::tcp::acceptor	acceptor_;
			/// ��ʱʱ��
			server_options		options_;
			/// ���Ӷ�����
			std::atomic< xgc_uint16 > acceptor_count_;
			/// ������״̬ 0 - ��������ʼ״̬�� 1 - �������ѿ����� 2 - �ȴ��ر�
			std::atomic< xgc_uint16 > status_;
			/// ������Ϣ������
			SessionCreator		creator_;
		};
	}
}
#endif // _ASIO_SERVER_H_