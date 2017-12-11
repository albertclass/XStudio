#include "header.h"
#include "protocol.h"
#include "game_message.h"
#include "client_message.h"

xgc_bool InitGateServer( ini_reader &ini );
xgc_bool OnceGateServer( xgc_bool busy );
xgc_void FiniGateServer( xgc_lpvoid params );

int ServerMain( int argc, char **argv )
{
	XGC_ASSERT_RETURN( argc > 1, -1 );

	if( InitServer( argv[1], InitGateServer ) )
	{
		LoopServer( OnceGateServer );
	}

	FiniServer( FiniGateServer, nullptr );
	return 0;
}

xgc_bool InitGateServer( ini_reader &ini )
{
	RegistPipeHandler( "*-*-0-*", OnPipeMsg, OnPipeEvt );
	RegistVirtualSockHandler( "*-*-0-*", OnVirtualSockMsg, OnVirtualSockEvt );

	RegistClientHandler( OnClientMsg, OnClientEvt, DefaultPacketParser );

	return true;
}

xgc_bool OnceGateServer( xgc_bool busy )
{
	return false;
}

xgc_void FiniGateServer( xgc_lpvoid params )
{

}