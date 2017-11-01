#include "header.h"
#include "session.h"

bool running = true;

// �洢Ŀ¼
char root_path[XGC_MAX_PATH] = {0};

int main( int argc, char* argv[] )
{
	#ifdef _WINDOWS
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	char conf_path[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( conf_path, "../client.ini" ) )
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

	auto addr = ini.get_item_value( "Client", "Host", "127.0.0.1" );
	auto port = ini.get_item_value( "Client", "Port", 50001 );
	auto root = ini.get_item_value( "Client", "Root", xgc_nullptr );
	XGC_ASSERT_RETURN( root, -1 );

	make_dirs( root );
	strcpy_s( root_path, root );

	if( false == net::CreateNetwork( 1 ) )
		return -1;

	net::connect_options opt;
	memset( &opt, 0, sizeof( opt ) );
	opt.is_async = false;
	opt.is_reconnect_passive = false;
	opt.is_reconnect_timeout = false;
	opt.recv_buffer_size = 64 * 1024;
	opt.send_buffer_size = 64 * 1024;

	opt.recv_packet_max = 64 * 1024;
	opt.recv_packet_max = 64 * 1024;

	opt.timeout = 1000;

	net::Connect( addr, port, XGC_NEW CNetSession(), &opt );
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
			std::this_thread::sleep_for( std::chrono::milliseconds(1) );
	}
	net::DestroyNetwork();

	return 0;
}