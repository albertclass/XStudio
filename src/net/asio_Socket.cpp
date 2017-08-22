#include "Header.h"

#define NETBASE_PING		EVENT_PING
#define NETBASE_PONG		EVENT_PONG

namespace xgc
{
	namespace net
	{
		using namespace asio;

		static xgc_size g_send_buffer_size = 1024 * 16;
		static xgc_size g_recv_buffer_size = 1024 * 16;

		static xgc_size g_send_packet_max = 1024 * 4;
		static xgc_size g_recv_packet_max = 1024 * 4;

		// ���÷��ͻ����С
		xgc_void set_send_buffer_size( xgc_size size )
		{
			if( size > 1024 * 16 )
				g_send_buffer_size = size;
		}

		// ���ý��ջ����С
		xgc_void set_recv_buffer_size( xgc_size size )
		{
			if( size > 1024 * 16 )
				g_recv_buffer_size = size;
		}

		// ���÷������ݴ�С
		xgc_void set_send_packet_size( xgc_size size )
		{
			if( size > 1024 * 8 )
				g_send_packet_max = size;
		}

		// ���ý������ݴ�С
		xgc_void set_recv_packet_size( xgc_size size )
		{
			if( size > 1024 * 8 )
				g_recv_packet_max = size;
		}

		asio_Socket::asio_Socket( io_service& s, xgc_uint16 timeout, xgc_lpvoid userdata, xgc_lpvoid from )
			: socket_( s )
			, connect_info_( xgc_nullptr )
			, handle_( INVALID_NETWORK_HANDLE )
			, connect_status_( 0 )
			, operator_count_( 0 )
			, timeout_( XGC_MAX( 500, timeout ) )
			, timer_( s )
			, from_( from )
			, userdata_( userdata )
			, recv_packet_max_( g_recv_packet_max )
			, send_packet_max_( g_send_packet_max )
		{
		}

		asio_Socket::~asio_Socket()
		{
			SAFE_DELETE( connect_info_ );
		}

		xgc_bool asio_Socket::set_buffer_size( set_option option, xgc_size buffer_size )
		{
			if( option == e_recv )
				return recv_buffer_.open( buffer_size );

			if( option == e_send )
				return send_buffer_.open( buffer_size );

			if( option == e_both )
				return recv_buffer_.open( buffer_size ) && send_buffer_.open( buffer_size );

			return false;
		}

		xgc_void asio_Socket::set_packet_max( set_option option, xgc_size packet_max )
		{
			if( option == e_recv )
				send_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );

			if( option == e_send )
				recv_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );

			if( option == e_both )
			{
				send_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );
				recv_packet_max_ = XGC_MAX( packet_max, 4 * 1024 );
			}
		}

		xgc_void asio_Socket::pending( xgc_lpvoid from )
		{
			// ������Դ
			from_ = from;
			connect_status_ = 1;

			//socket_base::send_buffer_size _send_buffer_size;
			//socket_base::receive_buffer_size _recv_buffer_size;

			//socket_.get_option( _send_buffer_size );
			//socket_.get_option( _recv_buffer_size );

			make_event( EVENT_PENDING );
		}

		xgc_bool asio_Socket::connect( xgc_lpcstr address, xgc_int16 port, connect_options *options )
		{
			// ������
			asio::error_code ec;

			asio::ip::tcp::endpoint ep( ip::address_v4::from_string( address, ec ), port );
			XGC_ASSERT_RETURN( !ec, false );

			socket_.open( ip::tcp::v4(), ec );
			XGC_ASSERT_RETURN( !ec, false );
			// �ÿ�����ϵͳ���ջ�����
			socket_.set_option( socket_base::receive_buffer_size( 0 ), ec );
			XGC_ASSERT_RETURN( !ec, false );
			// �ÿ�����ϵͳ���ͻ�����
			socket_.set_option( socket_base::send_buffer_size( 0 ), ec );
			XGC_ASSERT_RETURN( !ec, false );

			// �������Ӷ���
			connect_info_ = XGC_NEW connect_info;
			connect_info_->address = ep;
			if( options )
			{
				connect_info_->timeout = options->timeout;
				connect_info_->is_async = options->is_async ? 1 : 0;
				connect_info_->is_reconnect_timeout = options->is_reconnect_timeout ? 1 : 0;
				connect_info_->is_reconnect_passive = options->is_reconnect_passive ? 1 : 0;
			}
			else
			{
				connect_info_->timeout = 0;
				connect_info_->is_async = 0;
				connect_info_->is_reconnect_timeout = 0;
				connect_info_->is_reconnect_passive = 0;
			}

			if( options && options->recv_buffer_size )
				set_buffer_size( e_recv, options->recv_buffer_size );
			if( options && options->send_buffer_size )
				set_buffer_size( e_send, options->send_buffer_size );

			if( options && options->recv_packet_max )
				set_packet_max( e_recv, options->recv_packet_max );
			if( options && options->send_buffer_size )
				set_packet_max( e_send, options->send_packet_max );
			
			return do_connect();
		}

		xgc_bool asio_Socket::do_connect()
		{
			//NET_INFO( "connect %s:%d %s timeout %d"
			//	, connect_info_->address.address().to_string().c_str()
			//	, connect_info_->address.port()
			//	, connect_info_->is_async ? "async" : "sync"
			//	, connect_info_->timeout );

			if( connect_info_->is_async )
			{
				if( connect_info_->timeout )
				{
					// ͨ��strand����֤��ʱ���������Ӻ����Ǵ��еġ�
					asio::io_service::strand strand( socket_.get_io_service() );

					timer_.expires_from_now( std::chrono::milliseconds( connect_info_->timeout ) );
					timer_.async_wait(
						strand.wrap( std::bind( &asio_Socket::handle_connect_timeout, shared_from_this(), std::placeholders::_1 ) ) );

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
					socket_.close();

				return !ec;
			}
		}

		xgc_void asio_Socket::accept( int event )
		{
			// ��������״̬
			connect_status_ = 1;

			// ����δ���������û���
			if( 0 == recv_buffer_.capacity() )
			{
				if( false == set_buffer_size( e_recv, g_recv_buffer_size ) )
				{
					// �ύ�¼� NET_ERROR_NOT_ENOUGH_MEMROY
					make_error( NET_ETYPE_ACCEPT, NET_ERROR_NO_MEMORY );
					
					close();
					return;
				}
			}

			// ����δ���������û���
			if( 0 == send_buffer_.capacity() )
			{
				if( false == set_buffer_size( e_send, g_send_buffer_size ) )
				{
					// �ύ�¼� NET_ERROR_NOT_ENOUGH_MEMROY
					make_error( NET_ETYPE_ACCEPT, NET_ERROR_NO_MEMORY );

					close();
					return;
				}
			}

			// �ύ�¼� CONNECT / ACCEPT
			// һ���Ȼ���׼���ú����ύ�¼��������п����¼����յ������廹δ׼����ɡ�
			make_event( event );

			++operator_count_;

			// �ύ�첽��ȡ
			socket_.async_read_some(
				buffer( recv_buffer_.end(), recv_buffer_.space( recv_packet_max_ ) ),
				std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			// �����Ҫ����ύ���ʱ��
			if( timeout_ )
			{
				timer_.expires_from_now( std::chrono::milliseconds( timeout_ ) );
				timer_.async_wait( bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
			}
		}

		xgc_void asio_Socket::handle_connect( const asio::error_code& error )
		{
			if( !socket_.is_open() )
				return;

			if( timer_.cancel() )
			{
				if( !error )
				{
					accept( EVENT_CONNECT );
				}
				else
				{
					make_error( NET_ETYPE_CONNECT, error.value() );

					if( connect_info_->is_reconnect_timeout )
					{
						// �������������ģ���������
						timer_.expires_from_now( std::chrono::milliseconds( timeout_ ) );
						timer_.async_wait( bind( &asio_Socket::do_connect, shared_from_this() ) );
					}
					else
					{
						close();
					}
				}
			}
		}

		xgc_void asio_Socket::handle_connect_timeout( const asio::error_code& error )
		{
			if( connect_status_ != 0 )
				return;

			if( is_connected() )
				return;

			if( !error )
			{
				if( connect_info_ && connect_info_->is_reconnect_timeout )
				{
					// �������������ģ���������
					do_connect();
				}
				else
				{
					// ���ӳ�ʱ
					make_error( NET_ETYPE_CONNECT, NET_ERROR_TIMEOUT );
					close();
				}
			}
		}

		xgc_void asio_Socket::handle_recv( const asio::error_code& error, size_t translate )
		{
			if( error )
			{
				--operator_count_;
				close( true );
				return;
			}

			if( false == recv_buffer_.push( translate ) )
			{
				XGC_ASSERT( false );
				make_error( NET_ETYPE_RECV, NET_ERROR_NO_SPACE );

				--operator_count_;
				close( true );
				return;
			}

			INetworkSession* session = (INetworkSession*) userdata_;
			int packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );

			if( packet_length > recv_buffer_.capacity() )
			{
				XGC_ASSERT( false );
				make_error( NET_ETYPE_RECV, NET_ERROR_MESSAGE_SIZE );

				--operator_count_;
				close( true );
				return;
			}

			// �ѽ��յ����ݰ�
			while( packet_length && recv_buffer_.length() >= packet_length )
			{
				// �����ݰ����͵�����
				NetEvent evt;
				evt.session = userdata_;
				evt.handle = handle_;
				evt.event = EVENT_DATA;

				NetData dat;
				dat.length = packet_length;

				asio_NetBuffer packet( NET_PACKET_SIZE( packet_length ) );
				packet.put( &evt, sizeof( evt ) );
				packet.put( &dat, sizeof( dat ) );
				packet.put( recv_buffer_.begin(), packet_length );

				getSocketMgr().Push( packet.release(), NET_PACKET_SIZE( packet_length ) );

				// �������յ����ݰ�
				recv_buffer_.pop( packet_length );

				// ���¼�������ȣ�����һ�����ն���������⡣
				packet_length = session->OnParsePacket( (char*) recv_buffer_.begin(), recv_buffer_.length() );
			}

			if( false == recv_buffer_.enough( recv_packet_max_ ) )
				recv_buffer_.reset();

			if( connect_status_ == 1 )
			{
				// ������ȡЭ����
				socket_.async_read_some(
					buffer( recv_buffer_.end(), recv_buffer_.space( recv_packet_max_ ) ),
					std::bind( &asio_Socket::handle_recv, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );

			}
			else
			{
				--operator_count_;
				close( true );
			}
		}

		xgc_void asio_Socket::send( xgc_lpvoid data, xgc_size size )
		{
			XGC_ASSERT( data && size );

			std::lock_guard< std::mutex > guard( send_buffer_guard_ );

			// ���ݷ��뷢�ͻ���
			if( send_buffer_.put( data, size ) != size )
			{
				make_error( NET_ETYPE_SEND, NET_ERROR_NO_SPACE );

				close( true );
				return;
			}

			// ���ͺͷ�����ɿ��ܲ���ͬһ�߳���
			if( send_buffer_.length() == size )
			{
				++operator_count_;

				// ��������׼������
				if( !send_buffer_.enough( send_packet_max_ ) )
					send_buffer_.reset();

				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}

		xgc_void asio_Socket::send( const std::list< std::tuple< xgc_lpvoid, xgc_size > >& buffers )
		{
			xgc_size total = 0;
			std::lock_guard< std::mutex > guard( send_buffer_guard_ );

			for( auto &t : buffers )
			{
				xgc_lpvoid data = std::get< 0 >( t ) ;
				xgc_size   size = std::get< 1 >( t ) ;

				// ���ݷ��뷢�ͻ���
				if( send_buffer_.put( data, size ) != size )
				{
					make_error( NET_ETYPE_SEND, NET_ERROR_NO_SPACE );

					close( true );
					return;
				}

				total += size;
			}

			// ���ͺͷ�����ɿ��ܲ���ͬһ�߳���
			if( send_buffer_.length() == total )
			{
				++operator_count_;

				// ��������׼������
				if( !send_buffer_.enough( send_packet_max_ ) )
					send_buffer_.reset();

				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
		}


		xgc_void asio_Socket::handle_send( const asio::error_code& error, size_t translate )
		{
			if( error )
			{
				--operator_count_;
				close( true );
				return;
			}

			std::lock_guard< std::mutex > guard( send_buffer_guard_ );

			auto pop_size = send_buffer_.pop( translate );
			XGC_ASSERT( pop_size == translate );

			// ���������ӵ�����δ���ͣ����������
			if( connect_status_ == 1 && send_buffer_.length() )
			{
				++operator_count_;

				// ��������׼������
				if( !send_buffer_.enough( send_packet_max_ ) )
					send_buffer_.reset();

				socket_.async_send(
					buffer( send_buffer_.begin(), send_buffer_.length() ),
					std::bind( &asio_Socket::handle_send, shared_from_this(), std::placeholders::_1, std::placeholders::_2 ) );
			}
			
			--operator_count_;
			if( connect_status_ == 2 )
				close( true );
		}

		xgc_void asio_Socket::close( xgc_bool passive /*= false*/ )
		{
			asio::error_code ec;

			// ����������رյģ������ñ�������ʧЧ
			if( connect_info_ && !passive )
				connect_info_->is_reconnect_passive = false;

			xgc_uint16 exp1 = 1;
			if( connect_status_.compare_exchange_strong( exp1, 2 ) )
			{
				// ��ȡ����ʱ��
				timer_.cancel( ec );
				XGC_ASSERT( !ec );

				socket_.shutdown( ip::tcp::socket::shutdown_both, ec );
				XGC_ASSERT( !ec || ec.value() == 10054 );

				socket_.close( ec );
				XGC_ASSERT( !ec );
			}
			
			xgc_uint16 exp2 = 2;
			if( operator_count_ == 0 && connect_status_.compare_exchange_strong( exp2, 3 ) )
			{
				make_event( EVENT_CLOSE );
				connect_status_ = 0;
			}

			if( operator_count_ == 0 && connect_status_ == 0 )
			{
				if( connect_info_ && connect_info_->is_reconnect_passive )
				{
					do_connect();
				}
				else
				{
					LinkDown( shared_from_this() );
				}
			}
		}

		xgc_ulong asio_Socket::get_socket_info( xgc_uint32 addr[2], xgc_uint16 port[2] )
		{
			try
			{
				// local addr
				addr[0] = socket_.local_endpoint().address().to_v4().to_ulong();
				// local port
				port[0] = socket_.local_endpoint().port();

				// remote addr
				addr[1] = socket_.remote_endpoint().address().to_v4().to_ulong();
				// remote port
				port[1] = socket_.remote_endpoint().port();
			}
			catch( asio::error_code& ec )
			{
				ec;
				return -1;
			}

			return 0;
		}

		xgc_void asio_Socket::handle_timer( const asio::error_code& e )
		{
			if( connect_status_ != 1 )
				return;

			if( e != asio::error::operation_aborted )
			{
				if( is_connected() && timeout_ )
				{
					// check socket timeout
					make_event( EVENT_PING );

					timer_.expires_from_now( std::chrono::milliseconds( timeout_ ) );
					timer_.async_wait( std::bind( &asio_Socket::handle_timer, shared_from_this(), std::placeholders::_1 ) );
				}
			}
		}

		xgc_void asio_Socket::make_event( xgc_uint32 event )
		{
			NetEvent evt;
			evt.handle = handle_;
			evt.event = event;
			evt.session = userdata_;

			asio_NetBuffer packet( NET_EVENT_SIZE );
			packet.put( &evt, sizeof(evt) );

			getSocketMgr().Push( packet.release(), NET_EVENT_SIZE );
		}

		xgc_void asio_Socket::make_error( xgc_int16 error_type, xgc_int16 error_code )
		{
			NetEvent evt;
			evt.handle = handle_;
			evt.session = userdata_;
			evt.event = EVENT_ERROR;

			NetError err;
			err.error_type = error_type;
			err.error_code = error_code;

			asio_NetBuffer packet( NET_ERROR_SIZE );
			packet.put( &evt, sizeof(evt) );
			packet.put( &err, sizeof(err) );

			getSocketMgr().Push( packet.release(), NET_ERROR_SIZE );
		}

	}
}