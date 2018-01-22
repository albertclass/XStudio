#include "lua_debuger_sock.h"
void proc_response()
{
	for( auto &ln : cli.response )
		printf( "%s\n", ln.c_str() );
}

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

	printf( "lua_debuger connect %s:%d ...\n", host, port );

	// Æô¶¯ÍøÂçÏß³Ì
	std::unique_lock< std::mutex > lock( cli.mtx );
	thr = std::thread( client, host, port );
	cli.signal.wait( lock );
	lock.unlock();

	vector< tuple< long, string > > cmds;
	long cur = 0;
	long max = 0;

	while( true )
	{
		cmds.push_back( make_tuple( 0, string() ) );

		auto &cmd = std::get< 1 >( cmds[max] );
		long &pos = std::get< 0 >( cmds[max] );

		cur = max = (long)cmds.size() - 1;
		
		// clear whole line and go home of line.
		printf( "\33[2K\33[G" );

		for( int ch = getch(); ch != '\r' && ch != '\n'; ch = getch() )
		{
			switch( ch )
			{
				case 0x1b: // esc
				cmd.clear();
				printf( "\33[2K%s\r", cmd.c_str() + pos );
				pos = 0;
				break;
				case 0x08: // back
				if( pos > 0 )
				{
					cmd.erase( --pos );
					printf( "\33[D\33[%dX%s", cmd.size() - pos + 1, cmd.c_str() + pos );
				}
				break;
				case 0xe0:
				ch = getch();
				switch( ch )
				{
					case 'H': // arrow up
					if( cur > 0 )
					{
						--cur;
						cmd = std::get< 1 >( cmds[cur] );
						pos = (long)cmd.size();
						printf( "\33[2K\33[G%s", cmd.c_str() );
					}
					break;
					case 'P': // arrow down
					if( cur < max )
					{
						++cur;
						cmd = std::get< 1 >( cmds[cur] );
						pos = (long)cmd.size();
						printf( "\33[2K\33[G%s", cmd.c_str() );
					}
					
					break;
					case 'K': // arrow left
					if( pos > 0 )
					{
						--pos;
						printf( "\33[D" );
					}
					break;
					case 'M': // arrow right
					if( pos < (long)cmd.size() )
					{
						++pos;
						printf( "\33[C" );
					}
					break;
					case 'G': // home
					{
						printf( "\33[G" );
						pos = 0;
					}
					break;
					case 'O': // end
					{
						char end[8];
						sprintf_s( end, "\33[%dG", (long)cmd.size() + 1 );
						printf( end );
						pos = (long)cmd.size();
					}
					break;
				}

				break;
				default:
				if( isprint( ch ) )
				{
					cmd.insert( cmd.begin() + pos, ch );
					printf( "\33[s\33[0K%s\33[u\33[C", cmd.c_str() + pos );
					++pos;
				}
				break;
			}
		};

		// add cmd if not empty 
		if( !cmd.empty() ) ++max;

		request( cmd.c_str(), (int)cmd.size() + 1 );
		wait_signal();

		proc_response();
	}
}