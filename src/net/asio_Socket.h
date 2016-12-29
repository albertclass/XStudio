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
		public:
			asio_Socket( io_service& s, PacketProtocal* protocal, INetworkSession* holder, xgc_uint16 interval );

			~asio_Socket();

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
			xgc_void accept( xgc_lpvoid from );

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
			xgc_uintptr get_socket_info( xgc_int16 mask, xgc_byte* data );
		private:
			/*!
			 *
			 * \brief ���ӳɹ�
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
			xgc_void handle_timer();

			/*!
			 *
			 * \brief ���˰�
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_uint32 filter_packet( xgc_lpstr, xgc_size size );

		public:
			ip::tcp::socket& socket()
			{
				return socket_;
			}

			PacketProtocal*	get_protocal()const
			{
				return protocal_;
			}

			xgc_lpvoid	get_userdata()const
			{
				return userdata_;
			}

			network_t	get_handler()const
			{
				return handle_;
			}

			xgc_void set_handler( network_t handler )
			{
				handle_ = handler;
			}

			xgc_void set_userdata( xgc_lpvoid userdata )
			{
				userdata_ = userdata; holder_->SetUserdata( userdata );
			}

			xgc_bool is_connected()const
			{
				return socket_.is_open();
			}

			xgc_bool belong( xgc_lpvoid token )
			{
				return token == from_;
			}

		private:
			ip::tcp::socket		socket_;            // �׽���
			std::mutex			send_buffer_lock;   // ���ͻ�������
			asio_NetworkPacket	send_buffer_;       // ���ͻ���
			asio_NetworkPacket	recv_buffer_;       // ���ջ���

			network_t			handle_;            // ������
			std::atomic< xgc_uint32	> pingpong_;		 // pingֵ
			std::atomic< xgc_uint16	> pingpang_inerval_; // �������ͼ��
			std::atomic< xgc_uint16	> pingpong_failed_;  // ʧ�ܴ���
			std::atomic< xgc_uint16 > connect_status_;   // ��ǰ����״̬ 0 - δ���ӣ� 1 - �����ӣ� 2 - �ȴ��ر�
			xgc_lpvoid			userdata_;          // �û�����
			xgc_lpvoid			from_;			    // ��������
			deadline_timer		timer_;			    // ���ʱ��
			PacketProtocal		*protocal_;		    // Э����
			INetworkSession		*holder_;           // ���ݴ�����
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
	};
};

#endif // _ASIO_NETWORK_HANDLER_H_