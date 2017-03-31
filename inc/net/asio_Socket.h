#pragma once
#ifndef _ASIO_NETWORK_HANDLER_H_
#define _ASIO_NETWORK_HANDLER_H_

namespace xgc
{
	namespace net
	{
		using namespace asio;
		// ���÷��ͻ����С
		xgc_void set_send_buffer_size( xgc_size size );
		// ���ý��ջ����С
		xgc_void set_recv_buffer_size( xgc_size size );

		class asio_ServerBase;
		class asio_NetworkHandler;

		class asio_Socket : public std::enable_shared_from_this< asio_Socket >
		{
		friend class asio_ServerBase;
		friend class asio_SocketMgr;
		public:
			asio_Socket( io_service& s, xgc_lpvoid userdata, xgc_uint16 timeout );

			~asio_Socket();

			/*!
			 *
			 * \brief ���ӵ�������
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void hangup( xgc_lpvoid from );

			/*!
			 *
			 * \brief ���ӵ�������
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_bool connect( xgc_lpcstr address, xgc_int16 port, xgc_bool async = false, xgc_uint16 timeout = 1 );

			/*!
			 *
			 * \brief ��������
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void accept( int event );

			/*!
			 *
			 * \brief ������Ϣ
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void send( xgc_lpvoid data, xgc_size size, xgc_bool last = false );

			/*!
			*
			* \brief �ϲ���Ϣ������
			*
			* \author albert.xu
			* \date ʮһ�� 2015
			*/
			xgc_void send( const std::list< std::tuple< xgc_lpvoid, xgc_size > >& buffers );

			/*!
			 *
			 * \brief �ر�����
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void close();

			/*!
			*
			* \brief �ر�����
			*
			* \author albert.xu
			* \date ʮһ�� 2015
			*/
			xgc_ulong get_socket_info( xgc_int16 mask, xgc_byte* data );

			/*!
			*
			* \brief �ر�����
			*
			* \author albert.xu
			* \date ʮһ�� 2015
			*/
			xgc_void make_event( xgc_uint32 event, xgc_uint32 error_code, xgc_lpvoid bring = xgc_nullptr );
		private:
			/*!
			*
			* \brief �������Ӿ��
			*
			* \author albert.xu
			* \date ʮһ�� 2015
			*/
			xgc_void set_handle( network_t handler )
			{
				handle_ = handler;
			}

			/*!
			 *
			 * \brief �������ӽ����¼�
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void handle_connect( const asio::error_code& error );

			/*!
			 *
			 * \brief ���ӳ�ʱ
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void handle_timeout( const asio::error_code& error );

			/*!
			 *
			 * \brief ���ݽ��մ���
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void handle_recv( const asio::error_code& error, size_t translate );

			/*!
			 *
			 * \brief ���ݷ��ʹ���
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void handle_send( const asio::error_code& error, size_t translate );

			/*!
			 *
			 * \brief ���ӱ��ʱ��
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void handle_timer( const asio::error_code& e );

		public:
			///
			/// \brief ��ȡ��Դ��Ϣ
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:57
			///
			xgc_lpvoid get_from()const
			{
				return from_;
			}

			///
			/// \brief ��ȡ���Ӿ��
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:58
			///
			network_t get_handle()const
			{
				return handle_;
			}

			///
			/// \brief �Ƿ�������״̬
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:58
			///
			xgc_bool is_connected()const
			{
				return socket_.is_open() && connect_status_ == 1;
			}

			///
			/// \brief �жϴ�����ϵ
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:58
			///
			xgc_bool belong( xgc_lpvoid srv )
			{
				return srv == from_;
			}

			///
			/// \brief �����û�����
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:59
			///
			xgc_void set_userdata( xgc_lpvoid userdata )
			{
				userdata_ = userdata;
			}

			///
			/// \brief ��ȡ�û�����
			///
			/// \author albert.xu
			/// \date 2017/03/02 14:59
			///
			xgc_lpvoid get_userdata()const
			{
				return userdata_;
			}
		private:
			ip::tcp::socket		socket_;            // �׽���
			std::mutex			send_buffer_lock;   // ���ͻ�������
			asio_NetBuffer		send_buffer_;       // ���ͻ���
			asio_NetBuffer		recv_buffer_;       // ���ջ���

			std::atomic< xgc_uint16	> timeout_; // �������ͼ��
			std::atomic< xgc_uint16 > connect_status_;   // ��ǰ����״̬ 0 - δ���ӣ� 1 - �����ӣ� 2 - �ȴ��ر�
			network_t			handle_;            // ������
			xgc_lpvoid			from_;			    // ��������
			xgc_lpvoid			userdata_;			// �û�����
			asio::steady_timer	timer_;			    // ���ʱ��
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
	};
};

#endif // _ASIO_NETWORK_HANDLER_H_