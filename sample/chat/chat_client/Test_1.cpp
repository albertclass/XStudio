#include "header.h"
#include "gate.pb.h"
#include "Application.h"

#include "GameSrvSession.h"
#include "ChatSrvSession.h"

struct CountInfo
{
	long gate_connect_count;
	long gate_connected;
	long gate_connect_failed;
	long gate_connect_timeout;
	long gate_authenticate;
	long gate_disconnected;

	long chat_connect_count;
	long chat_connected;
	long chat_connect_failed;
	long chat_connect_timeout;
	long chat_authenticate;
	long chat_disconnected;
};

class Test1 : public CGameSrvSession
{
protected:
	CountInfo *info_;

public:
	Test1( CountInfo *info, xgc_lpcstr username, xgc_lpcstr password )
		: CGameSrvSession( username, password )
		, info_( info )
	{

	}

	~Test1()
	{
		// DBG_TIP( "User %s deleted.", mUsername.c_str() );
	}

	virtual xgc_void OnConnect( net::network_t handle ) override
	{
		++info_->gate_connected;
		CGameSrvSession::OnConnect( handle );
	}

	virtual xgc_void OnClose() override
	{
		++info_->gate_disconnected;
		CGameSrvSession::OnClose();
	}

	virtual xgc_void OnError( xgc_int16 error_type, xgc_int16 error_code ) override
	{
		if( error_type == NET_ETYPE_CONNECT )
		{
			if( error_code == NET_ERROR_TIMEOUT )
			{
				++info_->gate_connect_timeout;
			}
			else
			{
				++info_->gate_connect_failed;
			}
		}

		CGameSrvSession::OnError( error_type, error_code );
	}

	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size )override
	{
		MessageHeader &header = *(MessageHeader*)data;
		auto length = htons( header.length );
		auto message = htons( header.message );

		if( message == gate::GATE_MSG_LOGIN_ACK )
		{
			auto ptr = (xgc_lpstr)data + sizeof( MessageHeader );
			auto len = (int)( size - sizeof( MessageHeader ) );

			gate::login_ack ack;
			ack.ParseFromArray( ptr, len );

			if( ack.result() >= 0 )
			{
				++info_->gate_authenticate;
				++info_->chat_connect_count;
			}
		}

		CGameSrvSession::OnRecv( data, size );
	}

	virtual xgc_void OnChatConnect( network_t handle )override
	{
		++info_->chat_connected;
		CGameSrvSession::OnChatConnect( handle );
	}

	virtual xgc_void OnChatError( xgc_int16 error_type, xgc_int16 error_code )override
	{
		if( error_type == NET_ETYPE_CONNECT )
		{
			if( error_code == NET_ERROR_TIMEOUT )
			{
				++info_->chat_connect_timeout;
			}
			else
			{
				++info_->chat_connect_failed;
			}
		}

		CGameSrvSession::OnChatError( error_type, error_code );
	}

	virtual xgc_void OnChatClose()override
	{
		++info_->chat_disconnected;
		CGameSrvSession::OnChatClose();
	}

	virtual xgc_bool OnChatMsg( xgc_uint16 msgid, xgc_lpcstr ptr, xgc_long len )override
	{
		if( msgid == chat::MSG_USERAUTH_ACK )
		{
			++info_->chat_authenticate;
			Disconnect();
			return true;
		}

		return false;
	}
};

/// 连接压力测试
xgc_void test_1( int argc, char *argv[] )
{
	connect_options options_c;
	memset( &options_c, 0, sizeof( options_c ) );

	options_c.recv_buffer_size = 8 * 1024;
	options_c.send_buffer_size = 8 * 1024;

	options_c.recv_packet_max = 1024;
	options_c.send_packet_max = 1024;

	options_c.timeout = 3000;
	options_c.is_async = true;

	if( false == net::CreateNetwork( 1 ) )
		return;

	fprintf( stdout, "client is running.\n" );

	xgc_time64 last = current_milliseconds();
	xgc_time64 tick = current_milliseconds();

	/// 连接耗时
	xgc_time64 connect_start = 0;
	xgc_time64 connect_spend_socket = 0;
	xgc_time64 connect_spend = 0;

	xgc_uint64 pmem = 0;
	xgc_uint64 vmem = 0;

	CountInfo info;
	memset( &info, 0, sizeof( info ) );

	while( true )
	{
		xgc_time64 now = current_milliseconds();

		auto lhs = info.gate_connect_count + info.chat_connect_count;
		auto rhs =
			info.gate_connect_failed + info.gate_connect_timeout + info.gate_disconnected +
			info.chat_connect_failed + info.chat_connect_timeout + info.chat_disconnected;

		if( info.gate_connect_count == info.chat_connected + info.gate_connect_failed + info.gate_connect_timeout )
		{
			connect_spend = current_milliseconds() - connect_start;
		}

		if( lhs == rhs )
		{
			xgc_time64 now = current_milliseconds();

			if( info.gate_connect_count )
			{
				auto gate_connect_rate = info.gate_connected * 100.0f / info.gate_connect_count;
				auto chat_connect_rate = info.chat_connected * 100.0f / info.chat_connect_count;
				printf( "gate connect(%d), S(%d), F(%d), T(%d), A(%d), R(%0.2f%%)\n",
					info.gate_connect_count,
					info.gate_connected,
					info.gate_connect_failed,
					info.gate_connect_timeout,
					info.gate_authenticate,
					gate_connect_rate );

				printf( "chat connect(%d), S(%d), F(%d), T(%d), A(%d), R(%0.2f%%)\n",
					info.chat_connect_count,
					info.chat_connected,
					info.chat_connect_failed,
					info.chat_connect_timeout,
					info.chat_authenticate,
					chat_connect_rate );

				printf( "connect using %llu milliseconds, avg %0.2f connect pre seconds\n", connect_spend, info.gate_connect_count * 1000.0 / connect_spend );
				printf( "make socket using %llu milliseconds, avg %0.2f make socket pre seconds\n", connect_spend_socket, info.gate_connect_count * 1000.0 / connect_spend_socket );
			}

			for( int i = 0; i < 10; ++i )
			{
				printf( "%d. %d connection\n", i, i * 500 ? i * 1000 : 1 );
			}

			puts( "q. exit" );
			auto ch = getch();
			if( ch == 'q' )
				break;

			if( ch - '0' > 9 ) continue;

			// 重置成功与失败次数
			memset( &info, 0, sizeof( info ) );

			// 重置连接数
			info.gate_connect_count = ( ch - '0' ) ? ( ch - '0' ) * 1000 : 1;

			printf( "connect count = %d\n", info.gate_connect_count );

			get_process_memory_usage( &pmem, &vmem );

			connect_start = current_milliseconds();

			for( long i = 0; i < info.gate_connect_count; ++i )
			{
				char szUsername[64];
				sprintf_s( szUsername, "test%05d", i );
				char szPassword[64];
				sprintf_s( szPassword, "test%05d", i );

				DBG_TIP( "user = %s, password = %s", szUsername, szPassword );
				Connect(
					theApp.getGameAddr(),
					theApp.getGamePort(),
					XGC_NEW Test1( &info, szUsername, szPassword ),
					&options_c );
			}

			connect_spend_socket = current_milliseconds() - connect_start;

			xgc_uint64 pmem_last = pmem;
			xgc_uint64 vmem_last = vmem;

			get_process_memory_usage( &pmem, &vmem );

			printf( "using %0.4f seconds. pmem inc %0.4fM, vmem inc %0.4fM\n",
				( now - last ) / 1000.0f,
				( pmem - pmem_last ) * 1.0f / ( 1024 * 1024 ),
				( vmem - vmem_last ) * 1.0f / ( 1024 * 1024 ) );

			// 重置时间
			last = now;
		}

		if( now - tick >= 1000 )
		{
			printf( "gate connect success = %d, login = %4d, failed == %4d, timeout = %4d, close = %4d\n",
				info.gate_connected,
				info.gate_authenticate,
				info.gate_connect_failed,
				info.gate_connect_timeout,
				info.gate_disconnected );

			printf( "chat connect success = %d,  auth = %4d, failed == %4d, timeout = %4d, close = %4d\n",
				info.chat_connected,
				info.chat_authenticate,
				info.chat_connect_failed,
				info.chat_connect_timeout,
				info.chat_disconnected );

			xgc_uint64 pmem_last = pmem;
			xgc_uint64 vmem_last = vmem;

			get_process_memory_usage( &pmem, &vmem );

			printf( "using %0.4f seconds. pmem inc %0.4fM, vmem inc %0.4fM\n",
				( now - last ) / 1000.0f,
				pmem > pmem_last ? ( pmem - pmem_last ) * 1.0f / ( 1024 * 1024 ) : ( pmem_last - pmem ) * -1.0f / ( 1024 * 1024 ),
				vmem > vmem_last ? ( vmem - vmem_last ) * 1.0f / ( 1024 * 1024 ) : ( vmem_last - vmem ) * -1.0f / ( 1024 * 1024 ) );

			tick = now;
		}

		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	DestroyNetwork();
}
