#include "header.h"
#include "session.h"

int main( int argc, char *argv[] )
{
	CreateNetwork( 2 );
	connect_options options;
	memset( &options, 0, sizeof( options ) );

	options.recv_buffer_size = 1024 * 4;
	options.send_buffer_size = 1024 * 4;
	options.recv_packet_max = 1024 * 2;
	options.send_packet_max = 1024 * 2;

	options.is_async = true;
	options.is_reconnect_passive = false;
	options.is_reconnect_timeout = false;

	Connect( "127.0.0.1", 27030, XGC_NEW CSession(), &options );

	while( true )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	DestroyNetwork();
}