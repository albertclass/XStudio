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
			asio_ServerBase( io_service& service_, xgc_uint16 acceptor_count, xgc_uint16 timeout, const pfnCreateHolder & creator );

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

		protected:
			io_service&			service_;
			ip::tcp::acceptor	acceptor_;
			xgc_uint16			timeout_;
			xgc_uint16			acceptor_count_;
			///
			/// \brief ��������Ự
			///
			/// \author albert.xu
			/// \date 2016/02/26 15:48
			///
			pfnCreateHolder		creator_;
		};
	}
}
#endif // _ASIO_SERVER_H_