#include "header.h"
#include "protocol.h"
#include "gate_message.h"

xgc_bool InitConfiguration( ini_reader &ini, xgc_lpvoid params );
xgc_bool MainLoop( xgc_bool busy, xgc_lpvoid params );
xgc_void FiniConfiguration( xgc_lpvoid params );

int ServerMain( int argc, char **argv )
{
	if( ServerInit( argv[1], InitConfiguration, nullptr ) )
	{
		ServerLoop( MainLoop, nullptr );
	}

	ServerFini( FiniConfiguration, nullptr );
	return 0;
}

xgc_bool InitConfiguration( ini_reader &ini, xgc_lpvoid params )
{
	RegistPipeHandler( "*-*-1-*", OnPipeMsg, OnPipeEvt );
	RegistVirtualSockHandler( "*-*-1-*", OnVirtualSockMsg, OnVirtualSockEvt );

	return true;
}

xgc_bool MainLoop( xgc_bool busy, xgc_lpvoid params )
{
	return false;
}

xgc_void FiniConfiguration( xgc_lpvoid params )
{

}