#include "header.h"
#include "Server.h"

#include "UserMgr.h"
#include "ChannelMgr.h"

#include "ClientSession.h"
CServer theServer;

///
/// \brief 配置服务器
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_bool CServer::Setup( xgc_lpcstr lpConfigFile )
{
	char conf_path[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( conf_path, "%s", lpConfigFile ) )
	{
		fprintf( stderr, "format conf path error %s", conf_path );
		return false;
	}

	ini_reader ini;
	if( false == ini.load( conf_path ) )
	{
		fprintf( stderr, "conf load error %s", conf_path );
		return false;
	}

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 16 * 1024;
	param1.send_buffer_size = 16 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		return false;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 4 * 1024;
	param2.send_packet_size = 4 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		return false;
	}

	auto client_bind = ini.get_item_value( "ChatServer", "CliBind", "0.0.0.0" );
	strcpy_s( mChatBind, client_bind );

	auto server_bind = ini.get_item_value( "ChatServer", "SrvBind", "0.0.0.0" );
	strcpy_s( mGameBind, server_bind );

	mChatPort = ini.get_item_value( "ChatServer", "CliPort", 50001 );
	mGamePort = ini.get_item_value( "ChatServer", "SrvPort", 50002 );

	return init_logger( ini );
}

///
/// \brief 运行服务器
///
/// \author albert.xu
/// \date 2017/08/05
///
xgc_long CServer::Run()
{
	if( false == net::CreateNetwork( 1 ) )
		return -1;

	server_options options_s;
	memset( &options_s, 0, sizeof( options_s ) );
	options_s.acceptor_count = 10;

	options_s.recv_buffer_size = 1024 * 1024;
	options_s.send_buffer_size = 1024 * 1024;

	options_s.recv_packet_max = 16 * 1024;
	options_s.send_packet_max = 16 * 1024;

	mGameListener = net::StartServer( mGameBind, mGamePort, &options_s, [](){ return XGC_NEW CServerSession(); } );

	server_options options_c;
	memset( &options_c, 0, sizeof( options_c ) );
	options_c.acceptor_count = 32;

	options_c.recv_buffer_size = 16 * 1024;
	options_c.send_buffer_size = 16 * 1024;

	options_c.recv_packet_max = 2 * 1024;
	options_c.send_packet_max = 2 * 1024;
	mChatListener = net::StartServer( mChatBind, mChatPort, &options_c, [](){ return XGC_NEW CClientSession(); } );

	xgc_time64 last = current_milliseconds();

	while( mRunning )
	{
		auto now = current_milliseconds();

		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}

		if( now - last > 1000 )
		{
			getChannelMgr().eraseEmptyChannel();

			auto count_chn = getChannelMgr().getChannelCount();
			auto count_usr = getUserMgr().getUserCount();

			char sz_datetime[64];
			datetime::now( sz_datetime );
			printf( "%s : channel has %llu, user has %llu\n", sz_datetime, count_chn, count_usr );

			last = now;
		}
	}

	net::CloseServer( mChatListener );
	net::CloseServer( mGameListener );
	net::DestroyNetwork();

	return 0;
}

int main( int argc, char* argv[] )
{
	atexit( google::protobuf::ShutdownProtobufLibrary );

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	auto pLocale = setlocale( LC_ALL, "chs" );
	if( pLocale )
		printf( "locale set with %s\n", pLocale );
	else
		printf( "locale set failed\n" );

	auto pConfigFile = "chat-server.ini";
	if( argc > 1 )
		pConfigFile = argv[1];

	if( false == theServer.Setup( pConfigFile ) )
		return -1;

	return theServer.Run();
}