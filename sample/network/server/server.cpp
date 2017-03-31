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
	getServerFiles().GenFileList( root_path );

	if( false == net::CreateNetwork( 1 ) )
		return -1;

	net::StartServer( addr, port, 0, [](){ return XGC_NEW CNetSession(); } );
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}
	net::DestroyNetwork();
}