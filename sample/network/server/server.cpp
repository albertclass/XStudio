#include "header.h"
#include "session.h"
#include "server_files.h"

/// 是否运行中
xgc_bool running = true;

/// 根路径
xgc_char root_path[XGC_MAX_PATH] = { 0 };

int main( int argc, char* argv[] )
{
	initscr();
	start_color();
	init_pair( 1, COLOR_WHITE, COLOR_BLUE);
	init_pair( 2, COLOR_WHITE, COLOR_BLACK);

	cbreak();
	noecho();
	clear();
	nodelay( stdscr, 1 );
	curs_set(0);
	refresh();
	scrollok( stdscr, true );

	raw();
	keypad( stdscr, TRUE );
	mousemask( ALL_MOUSE_EVENTS, NULL );

	int max_row, max_col;
	getmaxyx( stdscr, max_row, max_col );

	WINDOW *win_err_border = newwin( 15, max_col, 0, 0 );
	if( win_err_border )
	{
		PANEL  *pel_err = new_panel( win_err_border );
		set_panel_userptr( pel_err, (void*)"err" );
		wattrset( win_err_border, COLOR_PAIR( 1 & A_CHARTEXT ) );
		wbkgd( win_err_border, COLOR_PAIR( 1 & A_CHARTEXT ) );

		box( win_err_border, 0, 0 );
		char title[] = "[error output window]";
		mvwprintw( win_err_border, 0, (getmaxx(win_err_border) - XGC_COUNTOF(title))/2, title );
	}

	WINDOW *win_err = subwin( win_err_border, 13, max_col - 2, 1, 1 );
	if( win_err )
	{
		scrollok( win_err, 1 );
		wmove( win_err, 0, 0 );
		wprintw( win_err, "error messages ... \n" );
	}

	WINDOW *win_inf_border = newwin( max_row - 16, max_col, 15, 0 );
	if( win_inf_border )
	{
		PANEL  *pel_files = new_panel(win_inf_border);
		set_panel_userptr( pel_files, (void*)"infomation" );

		wattrset( win_inf_border, COLOR_PAIR( 1 & A_CHARTEXT ) );
		wbkgd( win_inf_border, COLOR_PAIR( 1 & A_CHARTEXT ) );

		box( win_inf_border, 0, 0 );
		char title[] = "[infomation]";
		mvwprintw( win_inf_border, 0, (getmaxx(win_inf_border) - XGC_COUNTOF(title))/2, title );
	}
	
	WINDOW *win_inf = subwin( win_inf_border, 14, max_col - 2, 1, 1 );
	if( win_inf )
	{
		scrollok( win_inf, 1 );
		wmove( win_inf, 0, 0 );
	}

	update_panels();
	doupdate();

	char conf_path[1024] = { 0 };
	if( xgc_nullptr == get_absolute_path( conf_path, "../server.ini" ) )
	{
		wprintw( win_err, "format conf path error %s", conf_path );
		return -1;
	}

	ini_reader ini;
	if( false == ini.load( conf_path ) )
	{
		wprintw( win_err, "conf load error %s", conf_path );
		return -1;
	}

	auto addr = ini.get_item_value( "Server", "Listen", "0.0.0.0" );
	auto port = ini.get_item_value( "Server", "Port", 50001 );

	auto path = ini.get_item_value( "Server", "Path", xgc_nullptr );
	XGC_ASSERT_RETURN( path, -1 );

	get_absolute_path( root_path, "%s", path );

	auto exist = _access( root_path, 6 );
	XGC_ASSERT_RETURN( exist == 0, -1 );

	getServerFiles().GenIgnoreList( root_path );

	if( false == net::CreateNetwork( 1 ) )
		return -1;

	struct net::Param_SetBufferSize param1;
	param1.recv_buffer_size = 64 * 1024;
	param1.send_buffer_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetBufferSize, &param1 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	struct net::Param_SetPacketSize param2;
	param2.recv_packet_size = 64 * 1024;
	param2.send_packet_size = 64 * 1024;

	if( 0 != net::ExecuteState( Operator_SetPacketSize, &param2 ) )
	{
		net::DestroyNetwork();
		return -1;
	}

	auto srv = net::StartServer( addr, port, 0, [](){ return XGC_NEW CNetSession(); } );

	wprintw( win_inf, "server is running.\n" );
	int n = 0;
	while( running )
	{
		if( net::ProcessNetEvent( 100 ) == 100 )
		{
			napms( 1 );
			// std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
			int ch = getch();
			if( ch == 'x' )
				break;
			
			if( ch != ERR )
			{
				wprintw( win_err, "key %s - %d\n", keyname(ch), ch );
				update_panels();
				doupdate();
			}
		}
	}

	net::CloseServer( srv );
	net::DestroyNetwork();

	endwin();

	return 0;
}

#ifdef _WINDOWS
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
					  LPSTR lpszCmdLine, int nCmdShow)
{
	char *argv[30];
	int i, argc = 1;

	argv[0] = "newtest";
	for( i = 0; lpszCmdLine[i]; i++)
		if( lpszCmdLine[i] != ' ' && (!i || lpszCmdLine[i - 1] == ' '))
			argv[argc++] = lpszCmdLine + i;

	for( i = 0; lpszCmdLine[i]; i++)
		if( lpszCmdLine[i] == ' ')
			lpszCmdLine[i] = '\0';

	PDC_set_function_key( FUNCTION_KEY_SHUT_DOWN, ALT_X );

	return main( argc, (char **)argv);
}
#endif // _WINDOWS