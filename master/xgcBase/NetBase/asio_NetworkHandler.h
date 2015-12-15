#pragma once
#ifndef _ASIO_NETWORK_HANDLER_H_
#define _ASIO_NETWORK_HANDLER_H_
#include "asio_NetworkPacket.h"

struct IMessageQueue;
namespace XGC
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
		template< class > class CHandlerManager;
		struct asio_Socket : public std::enable_shared_from_this< asio_Socket >
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
			xgc_void accept( xgc_uintptr from );

			/*!
			 *
			 * \brief ������Ϣ
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void send( const void* data, size_t size );

			/*!
			 *
			 * \brief �ر�����
			 *
			 * \author albert.xu
			 * \date ʮһ�� 2015
			 */
			xgc_void close();

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
			xgc_uint32 filter_packet( const xgc_byte *data, xgc_size size );

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
				return handler_;
			}

			xgc_void set_handler( network_t handler )
			{
				handler_ = handler;
			}
			xgc_void set_userdata( xgc_lpvoid userdata )
			{
				userdata_ = userdata; holder_->SetUserdata( userdata );
			}
			xgc_void set_disconnect()
			{
				disconnect_ = true;
			}

			xgc_bool is_connected()const
			{
				return network_ != 0;
			}
		private:
			ip::tcp::socket		socket_;           // �׽���
			std::mutex			send_buffer_lock;  // ���ͻ�������
			asio_NetworkPacket	send_buffer_;      // ���ͻ���
			asio_NetworkPacket	recv_buffer_;      // ���ջ���
			asio_NetworkHandler	*network_;         // ���
			INetworkSession		*holder_;          // ���ݴ�����
			network_t			handler_;          // ������
			xgc_uint16			pingpang_inerval_; // �������ͼ��
			xgc_uint16			failed_count_;     // ʧ�ܴ���
			xgc_bool			disconnect_;       // �Ƿ����ӶϿ�
			xgc_lpvoid			userdata_;         // �û�����
			deadline_timer		timer_;			   // ���ʱ��
			PacketProtocal*		protocal_;		   // Э����
		};

		typedef std::shared_ptr< asio_Socket > asio_SocketPtr;
		class asio_NetworkHandler
		{
			friend class CHandlerManager< asio_NetworkHandler >;
		public:
			enum Error_Enum
			{
				HANDLE_OPEN_ERROR,				//����򿪴���
				HANDLE_READ_ERROR,				//���׽��ִ���
				HANDLE_WRITE_ERROR,				//д�׽��ִ���
				HANDLE_PROCESS_PACKET_ERROR,	//���ݰ����������������߼����ش���Ҳ��������ҹ���������Ĵ���
				HANDLE_SEND_COMPLETE_ERROR		//���ʹ���
			};
		public:
			asio_NetworkHandler( asio_SocketPtr& sock, xgc_uintptr from );

			~asio_NetworkHandler( xgc_void );

			//---------------------------------------------------//
			// [7/21/2009 Albert]
			// Description:	���û�����
			//---------------------------------------------------//
			XGC_INLINE xgc_void SetUserdata( xgc_lpvoid pUserdata );

			//---------------------------------------------------//
			// [3/14/2012 Albert]
			// Description:	��ȡ�û�����
			//---------------------------------------------------//
			XGC_INLINE xgc_lpvoid GetUserdata();

			//---------------------------------------------------//
			// [7/17/2009 Albert]
			// Description:	�ر�����
			//---------------------------------------------------//
			XGC_INLINE xgc_void CloseHandler();

			//////////////////////////////////////////////////////////////////////////
			// [7/17/2009 Albert]
			// �������ݰ�
			//////////////////////////////////////////////////////////////////////////
			XGC_INLINE xgc_void SendPacket( const void *data, size_t size, xgc_bool lastone = false );

			//////////////////////////////////////////////////////////////////////////
			// [7/17/2009 Albert]
			// ��ȡ������Ϣ
			//////////////////////////////////////////////////////////////////////////
			xgc_uintptr GetHandleInfo( xgc_int16 mask, xgc_byte* data );

			//---------------------------------------------------//
			// [11/11/2010 Albert]
			// Description:	����pingֵ 
			//---------------------------------------------------//
			XGC_INLINE xgc_void SetHandlePing( xgc_uint32 nPingPong );

			//---------------------------------------------------//
			// [3/6/2012 Albert]
			// Description:	��ȡЭ��ָ��
			//---------------------------------------------------//
			XGC_INLINE PacketProtocal* GetProtocal() const;

			//---------------------------------------------------//
			// [3/14/2012 Albert]
			// Description:	��鸽��
			//---------------------------------------------------//
			XGC_INLINE xgc_bool Belong( xgc_uintptr handle );

		protected:
			//---------------------------------------------------//
			// [7/17/2009 Albert]
			// Description:	������Դ
			//---------------------------------------------------//
			xgc_void Fetch();

			//---------------------------------------------------//
			// [7/17/2009 Albert]
			// Description:	�ͷ���Դ
			//---------------------------------------------------//
			xgc_void Free();

			///function	:	���ش����Ӿ���ı��
			///param	:	��
			///return	:	��0����
			XGC_INLINE network_t GetHandler()const
			{
				if( m_pSocket )
					return m_pSocket->get_handler();

				return INVALID_NETWORK_HANDLE;
			}

			///function	:	�������Ӿ���ı�ţ�Ҫ��Ϊ��0����
			///param	:	id	��0����
			///return	:	��
			XGC_INLINE xgc_void SetHandler( network_t nHandler )
			{
				if( m_pSocket )
					m_pSocket->set_handler( nHandler );
			}

		private:
			asio_SocketPtr	m_pSocket;
			std::mutex		m_oCriResource;
			xgc_uint32		m_nPingPong;

			xgc_uintptr	_from;
		};

		XGC_INLINE xgc_void asio_NetworkHandler::SetUserdata( xgc_lpvoid pUserdata )
		{
			if( m_pSocket )
				m_pSocket->set_userdata( pUserdata );
		}

		XGC_INLINE xgc_lpvoid asio_NetworkHandler::GetUserdata()
		{
			if( m_pSocket )
				return m_pSocket->get_userdata();

			return xgc_nullptr;
		}

		//---------------------------------------------------//
		// [7/17/2009 Albert]
		// Description:	�رվ��
		//---------------------------------------------------//
		XGC_INLINE xgc_void asio_NetworkHandler::CloseHandler()
		{
			m_pSocket->close();
		}

		XGC_INLINE xgc_void asio_NetworkHandler::SendPacket( const void *data, size_t size, xgc_bool lastone )
		{
			if( lastone )
				m_pSocket->set_disconnect();

			m_pSocket->send( data, size );
		}

		//---------------------------------------------------//
		// [7/17/2009 Albert]
		// Description:	������Դ
		//---------------------------------------------------//
		XGC_INLINE xgc_void asio_NetworkHandler::Fetch()
		{
			m_oCriResource.lock();
		}

		//---------------------------------------------------//
		// [7/17/2009 Albert]
		// Description:	�ͷ���Դ
		//---------------------------------------------------//
		XGC_INLINE xgc_void asio_NetworkHandler::Free()
		{
			m_oCriResource.unlock();
		}

		XGC_INLINE xgc_void asio_NetworkHandler::SetHandlePing( xgc_uint32 nPingPong )
		{
			m_nPingPong = nPingPong;
			// printf( "ping %d \n", nPingPong );
		}

		XGC_INLINE PacketProtocal* asio_NetworkHandler::GetProtocal() const
		{
			return m_pSocket->get_protocal();
		}

		//---------------------------------------------------//
		// [3/14/2012 Albert]
		// Description:	��鸽��
		//---------------------------------------------------//
		XGC_INLINE xgc_bool asio_NetworkHandler::Belong( xgc_uintptr handle )
		{
			return handle == _from;
		}

	};
};

#endif // _ASIO_NETWORK_HANDLER_H_