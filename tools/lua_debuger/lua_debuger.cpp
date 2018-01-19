#include "lua_debuger_sock.h"

int main( int argc, char** argv )
{
	std::thread thr;
	if( argc > 1 )
	{
		thr = std::thread( client, str2numeric< int >( argv[1] ) );

		while( true )
		{
			wait_signal();
		}
	}
}