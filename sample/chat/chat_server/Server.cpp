#include "header.h"
#include "CliSession.h"
#include "SrvSession.h"

/// 是否运行中
xgc_bool running = true;

/// 根路径
xgc_char root_path[XGC_MAX_PATH] = { 0 };

int main( int argc, char* argv[] )
{
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	char conf_path[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( conf_path, "../chat-server.ini" ) )
	{
		fprintf( stderr, "format conf path error %s", conf_path );
		return -1;
	}

	ini_reader ini;
	if( false == ini.load( conf_path ) )
	{
		fprintf( stderr, "conf load error %s", conf_path );
		return -1;
	}

	if( false == net::CreateNetwork( 1 ) )
		return -1;

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 16 * 1024;
	param1.send_buffer_size = 16 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 8 * 1024;
	param2.send_packet_size = 8 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	xgc_lpvoid srv, mgr;

	auto bind = ini.get_item_value( "ChatServer", "Bind", "0.0.0.0" );
	auto client_listen_port = ini.get_item_value( "ChatServer", "CliPort", 50001 );
	auto server_listen_port = ini.get_item_value( "ChatServer", "SrvPort", 50002 );

	srv = net::StartServer( bind, client_listen_port, 0, [](){ return XGC_NEW CClientSession(); } );
	mgr = net::StartServer( bind, server_listen_port, 0, [](){ return XGC_NEW CServerSession(); } );

	fprintf( stdout, "server is running.\n" );
	int n = 0;
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	net::CloseServer( srv );
	net::CloseServer( mgr );
	net::DestroyNetwork();

	return 0;
}
