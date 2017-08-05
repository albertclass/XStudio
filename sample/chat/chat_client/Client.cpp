#include "header.h"
#include "Client.h"

CClient theClient;

int main( int argc, char* argv[] )
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	auto pLocale = setlocale( LC_ALL, "chs" );
	if( pLocale )
		printf( "locale set with %s\n", pLocale );
	else
		printf( "locale set failed\n" );

	auto pConfigFile = "chat-server.ini";
	if( argc > 1 )
		pConfigFile = argv[1];

	if( false == theClient.Setup( pConfigFile ) )
		return -1;

	theClient.Run();

	return 0;
}

CClient::CClient()
{
}

CClient::~CClient()
{
}

xgc_bool CClient::Setup( xgc_lpcstr lpConfigFile )
{
	char conf_file[XGC_MAX_FNAME] = { 0 };

	if( xgc_nullptr == get_absolute_path( conf_file, lpConfigFile ) )
	{
		fprintf( stderr, "format conf path error %s", conf_file );
		return false;
	}

	ini_reader ini;
	if( false == ini.load( conf_file ) )
	{
		fprintf( stderr, "conf load error %s", conf_file );
		return false;
	}

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 64 * 1024;
	param1.send_buffer_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		return false;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 64 * 1024;
	param2.send_packet_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		return false;
	}

	auto addr = ini.get_item_value( "Client", "GameAddr", "0.0.0.0" );
	strcpy_s( mGameAddr, addr );
	auto port = ini.get_item_value( "GameServer", "Port", 40001 );
	mGamePort = port;

	addr = ini.get_item_value( "Client", "ChatAddr", "127.0.0.1" );
	strcpy_s( mChatAddr, addr );
	port = ini.get_item_value( "ChatServer", "CliPort", 50001 );
	mChatPort = port;

	return true;
}

xgc_void CClient::Run()
{
	if( false == net::CreateNetwork( 1 ) )
		return;

	mGameLink = Connect(
		mGameAddr,
		mGamePort,
		NET_CONNECT_OPTION_ASYNC | NET_CONNECT_OPTION_TIMEOUT,
		3000,
		XGC_NEW CGameSrvSession() );

	fprintf( stdout, "client is running.\n" );
	int n = 0;
	while( mRunning )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	// CloseLink( mGameLink );
	// CloseLink( mChatLink );
	DestroyNetwork();
}