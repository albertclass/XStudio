#include "Header.h"

#define NETBASE_PING		EVENT_PING
#define NETBASE_PONG		EVENT_PONG

namespace xgc
{
	namespace net
	{
		using namespace asio;

		static xgc_size send_buffer_size = 1024 * 16;
		static xgc_size recv_buffer_size = 1024 * 16;

		static xgc_size send_packet_max = 1024 * 4;
		static xgc_size recv_packet_max = 1024 * 4;

		// ���÷��ͻ����С
		xgc_void set_send_buffer_size( xgc_size size )
		{
			if( size > 1024 * 16 )
				send_buffer_size = size;
		}

		// ���ý��ջ����С
		xgc_void set_recv_buffer_size( xgc_size size )
		{
			if( size > 1024 * 16 )
				recv_buffer_size = size;
		}

		// ���÷������ݴ�С
		xgc_void set_send_packet_size( xgc_size size )
		{
			if( size > 1024 * 8 )
				send_packet_max = size;
		}

		// ���ý������ݴ�С
		xgc_void set_recv_packet_size( xgc_size size )
		{
			if( size > 1024 * 8 )
				recv_packet_max = size;
		}

		asio_Socket::asio_Socket( io_service& s, xgc_uint16 timeout, xgc_lpvoid userdata, xgc_lpvoid from )
			: socket_( s )
			, connect_info_( xgc_nullptr )
			, handle_( INVALID_NETWORK_HANDLE )
			, send_buffer_( send_buffer_size )
			, recv_buffer_( recv_buffer_size )
			, connect_status_( 0 )
			, timeout_( timeout )
			, timer_( s )
			, from_( from )
			, userdata_( userdata )
		{
		}

		asio_Socket::~asio_Socket()
		{
			SAFE_DELETE( connect_info_ );
		}

		xgc_void asio_Socket::hangup( xgc_lpvoid from )
		{
			// ������Դ
			from_ = from;
			connect_status_ = 1;

			make_event( EVENT_HANGUP, 0, from_ );
		}

		xgc_bool asio_Socket::connect( xgc_lpcstr address, xgc_int16 port, xgc_uint16 options, xgc_uint16 timeout )
		{
			// ������
			asio::error_code ec;

			asio::ip::tcp::endpoint ep( ip::address_v4::from_string( address, ec ), port );
			XGC_ASSERT_RETURN( !ec, false );

			return connect( ep, options, timeout );
		}

		xgc_bool asio_Socket::connect( const asio::ip::tcp::endpoint& address, xgc_uint32 options, xgc_uint16 timeout )
		{
			// �������Ӷ���
			if( xgc_nullptr == connect_info_ )
			{
				connect_info_ = XGC_NEW connect_info;
			}

			// �������Ӷ�������
			connect_info_->address = address;
			connect_info_->options = options;
			connect_info_->timeout = timeout;

			if( XGC_CHK_FLAGS( connect_info_->options, NET_CONNECT_OPTION_ASYNC ) )
			{
				if( XGC_CHK_FLAGS( connect_info_->options, NET_CONNECT_OPTION_TIMEOUT ) && connect_info_->timeout )
				{
					// ͨ��strand����֤��ʱ���������Ӻ����Ǵ��еġ�
					asio::io_service::strand strand( socket_.get_io_service() );

					timer_.expires_from_now( std::chrono::milliseconds( timeout ) );
					timer_.async_wait(
						strand.wrap( std::bind( &asio_Socket::handle_timeout, shared_from_this(), std::placeholders::_1 ) ) );

					// async connect server.
					socket_.async_connect( connect_info_->address, strand.wrap( std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) ) );
				}
				else
				{
					// async connect server.
					socket_.async_connect( connect_info_->address, std::bind( &asio_Socket::handle_connect, shared_from_this(), std::placeholders::_1 ) );
				}

				return true;
			}
			else
			{
				asio::error_code ec;

				if( socket_.connect( connect_info_->address, ec ) )
					accept( EVENT_CONNECT );
				else
					close();

				return !ec;
			}
		}

		xgc_void asio_Socket::accept( int event )
		{
			// ��������״̬
			connect_status_ = 1;

			// �ύ�¼� CONNECT / ACCEPT
			make_event( event, 0, userdata_ );

			// �ύ�첽��ȡ
			socket_.async_read_some(
				buffer( recv_buffer_.end(), recv_buffer_.space( recv_packet_max ) ),
				std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			timer_.cancel();

			// �����Ҫ����ύ���ʱ��
			if( timeout_ )
			{
				timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( 500, timeout_.load() ) ) );
				timer_.async_wait( bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
			}
		}

		xgc_void asio_Socket::handle_connect( const asio::error_code& error )
		{
			if( !socket_.is_open() )
				return;

			if( !error )
			{
				accept( EVENT_CONNECT );
			}
			else
			{
				make_event( EVENT_ERROR, NET_ERROR_CONNECT, userdata_ );
			}
		}

		xgc_void asio_Socket::handle_timeout( const asio::error_code& error )
		{
			if( connect_status_ == 2 )
				return;

			if( is_connected() )
				return;

			if ( !error )
			{
				// ���ӳ�ʱ
				if ( connect_info_ && XGC_CHK_FLAGS( connect_info_->options, NET_CONNECT_OPTION_RECONNECT ) && socket_.is_open() )
				{
					// �������������ģ���������
					connect( connect_info_->address, connect_info_->options, connect_info_->timeout );
				}
				else
				{
					make_event( EVENT_ERROR, NET_ERROR_CONNECT_TIMEOUT, userdata_ );
				}
			}
		}

		xgc_void asio_Socket::handle_recv( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				if( false == recv_buffer_.push( translate ) )
				{
					XGC_ASSERT( false );
					make_event( EVENT_ERROR, NET_ERROR_PACKET_SPACE, userdata_ );

					close();
					return;
				}

				INetworkSession* session = (INetworkSession*) userdata_;
				int packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );

				if( packet_length > recv_buffer_.capacity() )
				{
					XGC_ASSERT( false );
					make_event( EVENT_ERROR, NET_ERROR_PACKET_SPACE, userdata_ );

					close();
					return;
				}

				// �ѽ��յ����ݰ�
				while( packet_length && recv_buffer_.length() >= packet_length )
				{
					// �����ݰ����͵�����
					EventHeader evt;
					evt.handle = handle_;
					evt.event = EVENT_DATA;
					evt.error = packet_length;
					evt.bring = userdata_;

					auto packet = CNetworkPacket::allocate( sizeof(EventHeader) + packet_length );
					packet->putn( &evt, sizeof(evt) );
					packet->putn( recv_buffer_.begin(), packet_length );

					getSocketMgr().Push( packet );

					// �������յ����ݰ�
					recv_buffer_.pop( packet_length );

					// ���¼�������ȣ�����һ�����ն���������⡣
					packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );
				}

				if( false == recv_buffer_.enough( recv_packet_max ) )
					recv_buffer_.reset();

				// ������ȡЭ����
				socket_.async_read_some(
					buffer( recv_buffer_.end(), recv_buffer_.space( recv_packet_max ) ),
					std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
			else
			{
				make_event( EVENT_ERROR, error.value(), userdata_ );

				close();
			}
		}

		xgc_void asio_Socket::send( xgc_lpvoid data, xgc_size size, xgc_bool last /*= false*/ )
		{
			XGC_ASSERT( data && size );

			std::lock_guard< std::mutex > guard( send_buffer_lock );

			// ���ݷ��뷢�ͻ���
			if( send_buffer_.put( data, size ) != size )
			{
				make_event( EVENT_ERROR, NET_ERROR_SEND_BUFFER_FULL, userdata_ );

				close();
				return;
			}

			// ���ͺ��Ƿ�Ͽ�����
			if( last )
			{
				xgc_uint16 status = 1;
				connect_status_.compare_exchange_strong( status, 2 ); // �ȴ��ر�
			}

			// ���ͺͷ�����ɿ��ܲ���ͬһ�߳���
			if( send_buffer_.length() == size )
			{
				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}

		xgc_void asio_Socket::send( const std::list< std::tuple< xgc_lpvoid, xgc_size > >& buffers )
		{
			xgc_size total = 0;
			std::lock_guard< std::mutex > guard( send_buffer_lock );

			for( auto &t : buffers )
			{
				xgc_lpvoid data = std::get< 0 >( t ) ;
				xgc_size   size = std::get< 1 >( t ) ;

				// ���ݷ��뷢�ͻ���
				if( send_buffer_.put( data, size ) != size )
				{
					make_event( EVENT_ERROR, NET_ERROR_SEND_BUFFER_FULL, userdata_ );

					close();
					return;
				}

				total += size;
			}

			// ���ͺͷ�����ɿ��ܲ���ͬһ�߳���
			if( send_buffer_.length() == total )
			{
				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}


		xgc_void asio_Socket::handle_send( const asio::error_code& error, size_t translate )
		{
			if( !error )
			{
				std::lock_guard< std::mutex > guard( send_buffer_lock );

				// ����ָ���ͷŽ�������Ϣ���屻�ͷ� 
				// make_event( EVENT_SEND, translate );

				send_buffer_.pop( translate );

				// Ҫ��Ͽ�����
				if( connect_status_ == 2 )
				{
					close();
				}
				else if( send_buffer_.length() )
				{
					socket_.async_send(
						buffer( send_buffer_.begin(), send_buffer_.length() ),
						std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
				}
			}
			else
			{
				make_event( EVENT_ERROR, error.value(), userdata_ );

				close();
			}
		}

		xgc_void asio_Socket::close()
		{
			asio::error_code ec;

			if( socket_.is_open() )
			{
				connect_status_ = 2;

				// ��ȡ����ʱ��
				timer_.cancel( ec );
				XGC_ASSERT( !ec );

				socket_.shutdown( ip::tcp::socket::shutdown_both, ec );
				socket_.close( ec );
				XGC_ASSERT( !ec );

				make_event( EVENT_CLOSE, 0, userdata_ );

				userdata_ = xgc_nullptr;

				connect_status_ = 0;
			}

			LinkDown( shared_from_this() );
		}

		xgc_ulong asio_Socket::get_socket_info( xgc_int16 mask, xgc_byte * data )
		{
			xgc_byte* ptr = data;
			try
			{
				if( mask & NET_LOCAL_ADDRESS )
				{
					Param_QueryHandleInfo::endpoint* point = (Param_QueryHandleInfo::endpoint*)ptr;
					point->addr = socket_.local_endpoint().address().to_v4().to_ulong();
					point->port = socket_.local_endpoint().port();

					ptr = (xgc_byte*) (point + 1);
				}

				if( mask & NET_REMOT_ADDRESS )
				{
					Param_QueryHandleInfo::endpoint* point = (Param_QueryHandleInfo::endpoint*)ptr;
					point->addr = socket_.remote_endpoint().address().to_v4().to_ulong();
					point->port = socket_.remote_endpoint().port();

					ptr = (xgc_byte*) (point + 1);
				}
			}
			catch( asio::error_code& ec )
			{
				ec;
				return -1;
			}

			return xgc_ulong(ptr - data);
		}

		xgc_void asio_Socket::handle_timer( const asio::error_code& e )
		{
			if( connect_status_ == 2 )
				return;

			if( e != asio::error::operation_aborted )
			{
				if( is_connected() && timeout_ )
				{
					// check socket timeout
					make_event( EVENT_PING, 0, userdata_ );

					timer_.expires_from_now( std::chrono::milliseconds( XGC_MAX( timeout_.load(), 500 ) ) );
					timer_.async_wait( std::bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
				}
			}
		}

		xgc_void asio_Socket::make_event( xgc_uint32 event, xgc_uint32 error_code, xgc_lpvoid bring )
		{
			EventHeader evt;
			evt.handle = handle_;
			evt.event = event;
			evt.error = error_code;
			evt.bring = bring;

			getSocketMgr().Push( CNetworkPacket::allocate( &evt, sizeof( evt ), handle_ ) );
		}
	}
}