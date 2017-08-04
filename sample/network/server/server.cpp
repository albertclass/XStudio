#include "header.h"
#include "session.h"
#include "server_files.h"

/// 是否运行中
xgc_bool running = true;

/// 根路径
xgc_char root_path[XGC_MAX_PATH] = { 0 };

int main( int argc, char* argv[] )
{
	char conf_path[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( conf_path, "../server.ini" ) )
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

	auto addr = ini.get_item_value( "Server", "Listen", "0.0.0.0" );
	auto port = ini.get_item_value( "Server", "Port", 50001 );

	auto path = ini.get_item_value( "Server", "Path", xgc_nullptr );
	XGC_ASSERT_RETURN( path, -1 );

	get_absolute_path( root_path, "%s", path );

	auto exist = _access( root_path, 6 );
	XGC_ASSERT_RETURN( exist == 0, -1 );

	getServerFiles().GenIgnoreList( root_path );

	if( false == net::CreateNetwork( 1 ) )
		return -1;

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 64 * 1024;
	param1.send_buffer_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 64 * 1024;
	param2.send_packet_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	auto srv = net::StartServer( addr, port, 0, [](){ return XGC_NEW CNetSession(); } );

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
	net::DestroyNetwork();

	return 0;
}

#ifdef _WINDOWS
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
					  LPSTR lpszCmdLine, int nCmdShow)
{
	char *argv[30];
	int i, argc = 1;

	argv[0] = "newtest";
	for( i = 0; lpszCmdLine[i]; i++)
		if( lpszCmdLine[i] != ' ' && (!i || lpszCmdLine[i - 1] == ' '))
			argv[argc++] = lpszCmdLine + i;

	for( i = 0; lpszCmdLine[i]; i++)
		if( lpszCmdLine[i] == ' ')
			lpszCmdLine[i] = '\0';

	return main( argc, (char **)argv);
}
#endif // _WINDOWS