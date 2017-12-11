#include "header.h"
#include "protocol.h"
#include "gate_message.h"

xgc_bool InitGameServer( ini_reader &ini );
xgc_bool ProcGameServer( xgc_bool busy );
xgc_void FiniConfiguration( xgc_lpvoid params );

int ServerMain( int argc, char **argv )
{
	if( InitServer( argv[1], InitGameServer ) )
	{
		LoopServer( ProcGameServer );
	}

	FiniServer( FiniConfiguration, nullptr );
	return 0;
}

xgc_bool InitGameServer( ini_reader &ini )
{
	RegistPipeHandler( "*-*-1-*", OnPipeMsg, OnPipeEvt );
	RegistVirtualSockHandler( "*-*-1-*", OnVirtualSockMsg, OnVirtualSockEvt );

	return true;
}

xgc_bool ProcGameServer( xgc_bool busy )
{
	return false;
}

xgc_void FiniConfiguration( xgc_lpvoid params )
{

}