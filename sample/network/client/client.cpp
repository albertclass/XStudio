#include "header.h"
#include "session.h"

xgc_bool running = true;

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

	net::ConnectAsync( addr, port, 0, XGC_NEW CNetSession() );
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
			std::this_thread::sleep_for( std::chrono::milliseconds(1) );
	}
	net::DestroyNetwork();
}