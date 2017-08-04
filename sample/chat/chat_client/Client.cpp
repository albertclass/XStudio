#include "header.h"
#include "GateSrvSession.h"
xgc_bool running = true;

int main( int argc, char* argv[] )
{
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

	network_t mNet = Connect( 
		"127.0.0.1", 
		60001, 
		NET_CONNECT_OPTION_ASYNC | NET_CONNECT_OPTION_TIMEOUT, 
		1000, 
		XGC_NEW CGateSrvSession() );

	fprintf( stdout, "client is running.\n" );
	int n = 0;
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	CloseLink( mNet );
	DestroyNetwork();
	return 0;
}