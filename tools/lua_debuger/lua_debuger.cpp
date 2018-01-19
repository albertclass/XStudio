#include "lua_debuger_sock.h"

int main( int argc, char** argv )
{
	std::thread thr;
	const char* host = "127.0.0.1";
	int port = 5001;

	if( argc > 2 )
	{
		host = argv[1];
		port = str2numeric< int >( argv[2] );
	}
	else if( argc > 1 )
	{
		port = str2numeric< int >( argv[1] );
	}
	else
	{
		printf( "usage : lua_debuger host port" );
		return -1;
	}

	// Æô¶¯ÍøÂçÏß³Ì
	std::unique_lock< std::mutex > lock( cli.mtx );
	thr = std::thread( client, str2numeric< int >( argv[1] ) );
	cli.signal.wait( lock );
	lock.unlock();

	vector< tuple< long, string > > cmds;
	long cur = 0;
	while( true )
	{
		int ch = getchar();
		while( cur >= cmds.size() )
		{
			cmds.push_back( make_tuple( 0, string() ) );
		}

		auto &cmd = std::get< 1 >( cmds[cur] );
		long &pos = std::get< 0 >( cmds[cur] );

		while( ch != '\n' )
		{
			switch( ch )
			{
				case 0x27: // esc
				cmd.clear();
				pos = 0;
				break;
				case 0x13: // back
				if( pos > 0 )
				{
					cmd.erase( pos-- );
				}
				break;
				default:
				if( isprint( ch ) )
				{
					cmd.insert( cmd.begin() + pos, ch );
					++pos;
				}
				break;
			}

			send( cmd.c_str(), cmd.size() + 1 );
			wait_signal();
		};
		wait_signal();
	}
}