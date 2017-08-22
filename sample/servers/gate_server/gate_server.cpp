#include "header.h"
#include "protocol.h"
#include "game_message.h"
#include "client_message.h"

xgc_bool InitConfiguration( ini_reader &ini, xgc_lpvoid params );
xgc_bool MainLoop( xgc_bool busy, xgc_lpvoid params );
xgc_void FiniConfiguration( xgc_lpvoid params );

int ServerMain( int argc, char **argv )
{
	XGC_ASSERT_RETURN( argc > 1, -1 );

	if( ServerInit( argv[1], InitConfiguration, nullptr ) )
	{
		ServerLoop( MainLoop, nullptr );
	}

	ServerFini( FiniConfiguration, nullptr );
	return 0;
}

xgc_bool InitConfiguration( ini_reader &ini, xgc_lpvoid params )
{
	RegistPipeHandler( "*-*-0-*", OnPipeMsg, OnPipeEvt );
	RegistVirtualSockHandler( "*-*-0-*", OnVirtualSockMsg, OnVirtualSockEvt );

	RegistClientHandler( OnClientMsg, OnClientEvt, DefaultPacketParser );

	return true;
}

xgc_bool MainLoop( xgc_bool busy, xgc_lpvoid params )
{
	return false;
}

xgc_void FiniConfiguration( xgc_lpvoid params )
{

}

xgc_void OnServerRefresh( xgc_lpcstr lpInvoke, datetime dtPrevInvokeTime, xgc_lpvoid lpContext )
{

}
