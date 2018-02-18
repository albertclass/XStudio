#include "header.h"
#include "gate.pb.h"
#include "Application.h"

#include "GameSrvSession.h"
#include "ChatSrvSession.h"

class Test0 : public CGameSrvSession
{
public:
	Test0( xgc_lpcstr username, xgc_lpcstr password )
		: CGameSrvSession( username, password )
	{

	}

private:
	virtual xgc_bool OnChatMsg( xgc_uint16 msgid, xgc_lpcstr ptr, xgc_long len )
	{
		if( msgid == chat::CHAT_USERAUTH_ACK )
		{
			Disconnect();
			return true;
		}

		return false;
	}
};
xgc_void test_0( int argc, char *argv[] )
{
	puts( "test 0 is running ... " );

	if( false == net::CreateNetwork( 1 ) )
		return;

	connect_options options_c;
	memset( &options_c, 0, sizeof( options_c ) );

	options_c.timeout = 3000;
	options_c.is_async = true;

	auto net_h = Connect(
		theApp.getGameAddr(),
		theApp.getGamePort(),
		XGC_NEW Test0( "test100001", "test100001" ),
		&options_c );

	while( true )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
	}

	DestroyNetwork();
}
