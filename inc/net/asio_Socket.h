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
		// ���÷������ݴ�С
		xgc_void set_send_packet_size( xgc_size size );
		// ���ý������ݴ�С
		xgc_void set_recv_packet_size( xgc_size size );

		class asio_Server;
		class asio_NetworkHandler;

		class asio_Socket : public std::enable_shared_from_this< asio_Socket >
		{
		friend class asio_Server;
		friend class asio_SocketMgr;
		public:
			enum set_option { e_recv, e_send, e_both };

			///
			/// \brief ����
			/// \author albert.xu
			/// \date 2015/11/13
			///
			asio_Socket( io_service& s, xgc_uint16 timeout, xgc_lpvoid userdata, xgc_lpvoid from );

			///
			/// \brief ����
			/// \author albert.xu
			/// \date 2015/11/13
			///
			~asio_Socket();

			/*!
			 *
			 * \brief ���û�������С
			 *
			 * \author albert.xu
			 * \date 2017/08/10
			 */
			xgc_bool set_buffer_size( set_option option, xgc_size buffer_size );

			/*!
			*
			* \brief ���û�������С
			*
			* \author albert.xu
			* \date 2017/08/10
			*/
			xgc_void set_packet_max( set_option option, xgc_size packet_max );

			/*!
			 *
			 * \brief �ȴ����ӵ�������
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void pending( xgc_lpvoid from );

			/*!
			 *
			 * \brief ���ӵ�������
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_bool connect( xgc_lpcstr address, xgc_int16 port, connect_options *options );

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
			xgc_void send( xgc_lpvoid data, xgc_size size );

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
			xgc_ulong get_socket_info( xgc_uint32 addr[2], xgc_uint16 port[2] );

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
			* \brief ���ӵ�������
			*
			* \author albert.xu
			* \date 2017/07/18
			*/
			xgc_bool connect();

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
			/// ������Ϣ
			struct connect_info
			{
				/// ���ӵȴ�ʱ�䣨��ʱ��
				xgc_uint16 timeout;

				/// �Ƿ��첽
				xgc_uint16 is_async : 1;
				/// �Ƿ�ʱ���������Ӳ��ɹ�ʱ������
				xgc_uint16 is_reconnect_timeout : 1;
				/// �Ƿ񱻶������������Ͽ���������
				xgc_uint16 is_reconnect_passive : 1;
				// ���ӵ�ַ
				asio::ip::tcp::endpoint address;
			};
			
			/// ������Ϣ
			connect_info		*connect_info_;

			/// �׽���
			ip::tcp::socket		socket_;
			/// ���ͻ�������
			std::mutex			send_buffer_lock;
			/// ���ͻ���
			asio_NetBuffer		send_buffer_;       
			/// ���ջ���
			asio_NetBuffer		recv_buffer_;       

			/// �������ͼ��
			xgc_uint16			timeout_;
			
			/// ��ǰ����״̬ 0 - δ���ӣ� 1 - �����ӣ� 2 - �ȴ��ر�
			std::atomic_ushort  connect_status_;

			/// ��ǰͶ�ݵĲ�����
			std::atomic_ushort	operator_count_;

			/// ������
			volatile network_t	handle_;
			/// ��������
			volatile xgc_lpvoid	from_;
			/// �û�����
			volatile xgc_lpvoid	userdata_;
			
			/// �������ݰ����ߴ�
			volatile xgc_size send_packet_max_;
			/// �������ݰ����ߴ�
			volatile xgc_size recv_packet_max_;

			/// ���ʱ��
			asio::steady_timer	timer_;
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
	};
};

#endif // _ASIO_NETWORK_HANDLER_H_