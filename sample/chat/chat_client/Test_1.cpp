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
		// DBG_INFO( "User %s deleted.", mUsername.c_str() );
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

	virtual xgc_void OnError( xgc_uint32 error ) override
	{
		if( error == NET_ERROR_CONNECT )
			++info_->gate_connect_failed;

		if( error == NET_ERROR_CONNECT_TIMEOUT )
			++info_->gate_connect_timeout;

		CGameSrvSession::OnError( error );
	}

	virtual xgc_void OnRecv( xgc_lpvoid data, xgc_size size )override
	{
		MessageHeader &header = *(MessageHeader*)data;
		auto length = htons( header.length );
		auto message = htons( header.message );

		if( message == gate::GATE_MSG_LOGIN_ACK )
		{
			++info_->gate_authenticate;
			++info_->chat_connect_count;
		}

		CGameSrvSession::OnRecv( data, size );
	}

	virtual xgc_void OnChatConnect( network_t handle )override
	{
		++info_->chat_connected;
		CGameSrvSession::OnChatConnect( handle );
	}

	virtual xgc_void OnChatError( xgc_uint32 error )override
	{
		if( error == NET_ERROR_CONNECT )
			++info_->chat_connect_failed;

		if( error == NET_ERROR_CONNECT_TIMEOUT )
			++info_->chat_connect_timeout;

		CGameSrvSession::OnChatError( error );
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

	options_c.timeout = 3000;
	options_c.is_async = true;
	options_c.is_reconnect_timeout = true;

	if( false == net::CreateNetwork( 1 ) )
		return;

	fprintf( stdout, "client is running.\n" );

	xgc_time64 last = current_milliseconds();
	xgc_time64 tick = current_milliseconds();
	xgc_uint64 pmem = 0;
	xgc_uint64 vmem = 0;

	get_process_memory_usage( &pmem, &vmem );
	CountInfo info;
	memset( &info, 0, sizeof( info ) );

	long account_idx = 0;

	long round = 5;
	while( round < 10 )
	{
		xgc_time64 now = current_milliseconds();

		auto lhs = info.gate_connect_count + info.chat_connect_count;
		auto rhs = 
			info.gate_connect_failed + info.gate_connect_timeout + info.gate_disconnected +
			info.chat_connect_failed + info.chat_connect_timeout + info.chat_disconnected;

		if( lhs == rhs )
		{
			// 第一轮不显示统计信息
			if( round++ )
			{
				auto gate_connect_rate = info.gate_connected * 100.0f / info.gate_connect_count;
				auto chat_connect_rate = info.chat_connected * 100.0f / info.chat_connect_count;
				printf( "gate connect success = %d, failed == %d, timeout = %d, connect rate = %0.2f%%\n", 
					info.gate_connected,
					info.gate_connect_failed,
					info.gate_connect_timeout,
					gate_connect_rate );

				printf( "chat connect success = %d, failed == %d, timeout = %d, connect rate = %0.2f%%\n",
					info.chat_connected,
					info.chat_connect_failed,
					info.chat_connect_timeout,
					gate_connect_rate );

				xgc_uint64 pmem_last = pmem;
				xgc_uint64 vmem_last = vmem;

				get_process_memory_usage( &pmem, &vmem );

				printf( "using %0.4f seconds. pmem inc %0.2fM, vmem inc %0.2fM\n",
					( now - last ) / 1000.0f,
					( pmem - pmem_last ) * 1.0f / 1024,
					( vmem - vmem_last ) * 1.0f / 1024 );
			}

			// 重置成功与失败次数
			memset( &info, 0, sizeof( info ) );

			// 重置时间
			last = current_milliseconds();
			// 重置连接数
			info.gate_connect_count = round * 100;

			printf( "connect count = %d\n", info.gate_connect_count );

			for( long i = 0; i < info.gate_connect_count; ++i, ++account_idx )
			{
				char szUsername[64];
				sprintf_s( szUsername, "test%05d", account_idx );
				char szPassword[64];
				sprintf_s( szPassword, "test%05d", account_idx );

				DBG_INFO( "user = %s, password = %s", szUsername, szPassword );
				Connect(
					theApp.getGameAddr(),
					theApp.getGamePort(),
					XGC_NEW Test1( &info, szUsername, szPassword ),
					&options_c );
			}
		}

		if( now - tick >= 500 )
		{
			printf( "gate connect success = %d, login = %d, failed == %d, timeout = %d, disconnected = %d\n",
				info.gate_connected,
				info.gate_authenticate,
				info.gate_connect_failed,
				info.gate_connect_timeout,
				info.gate_disconnected );

			printf( "chat connect success = %d, auth = %d, failed == %d, timeout = %d, disconnected = %d\n",
				info.chat_connected,
				info.chat_authenticate,
				info.chat_connect_failed,
				info.chat_connect_timeout,
				info.chat_disconnected );

			tick = now;
		}

		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	DestroyNetwork();
}
