#include "defines.h"
#include "frame.h"
#ifdef _WINDOWS
#	include <conio.h>
#	include "console.h"
using namespace xgc::console;
#else
#	include <sys/select.h>
#	include <sys/types.h>
#	include <sys/time.h>
#	include <termios.h>
#	include <unistd.h>
#	include <stdio.h>
#	include <ncurses.h>
WINDOW * show_w;
#endif

int main( int argc, char* argv[] )
{
	int page = 0;
	int show = 10;

	bool exit = false;

	auto frame = get_test_frame();
	auto prepage = frame.tests( page, show );

	int row_max, col_max;

#ifdef _WINDOWS
	console_init( 120, 40 );
	buffer_t menu_b = INVALID_BUFFER_INDEX;
	buffer_t info_b = INVALID_BUFFER_INDEX;

	window_t menu_w = window( 0,  0, 120, 14, menu_b, WINDOW_STYLE_BORDER );
	window_t info_w = window( 0, 14, 120, 26, info_b, WINDOW_STYLE_BORDER );

	redirect( stdout, buffer(info_w) );

	int i = 0;
	for( auto &it : prepage )
	{
		printf_text( menu_b, "%d. %s\n", i++, it->display );
	}

	printf_text( menu_b, "q. exit\n" );

	redraw_window( menu_w, true );

	while( false == exit )
	{
		int ch = getch();
		if( ch >= '0' && ch <= '9' )
		{
			auto id = ch - '0';
			// argv[0] = prepage[id]->name;
			prepage[id]->entry( argc, argv );
		}
		//else if( ch == KEY_NPAGE )
		//{
		//	output( "%s", "choice page down.\n" );
		//}
		//else if( ch == KEY_PPAGE )
		//{
		//	output( "%s", "choice page up.\n" );
		//}
		else
		{
			exit = (ch == 27 || ch == 'q');
		}

		redraw_window( menu_w, true );
		redraw_window( info_w, true );
		refresh();
	}

	console_fini();
#else
	initscr();
	raw();
	noecho();
	curs_set(0);
	mousemask( ALL_MOUSE_EVENTS, NULL );
	printw( "unit test framework v1.0!\n");

	getmaxyx(stdscr,row_max,col_max);

	WINDOW *test_border_w = newwin( 14, 60, 0, 0 );
	box( test_border_w, 0, 0 );
	wrefresh( test_border_w );

	WINDOW *test_w = newwin( 12, 58, 1, 1 );
	int i = 0;
	for( auto &item : prepage )
	{
		wprintw( test_w, "%d. %s\n", i++, item->display );
	}
	wprintw( test_w, "q. exit\n" );

	mvwprintw( test_w, 11, 0, "choice test run : " );
	keypad( test_w, true );
	wrefresh( test_w );

	WINDOW *show_border_w = newwin( LINES - 14, COLS, 14, 0 );
	box( show_border_w, 0, 0 );
	wrefresh( show_border_w );

	show_w = newwin( LINES - 14 - 2, COLS - 2, 15, 1 );
	scrollok( show_w, true );
	while( false == exit )
	{
		int ch = wgetch(test_w);
		if( ch >= '0' && ch <= '9' )
		{
			auto id = ch - '0';
			// argv[0] = prepage[id]->name;
			prepage[id]->entry( argc, argv );
		}
		else if( ch == KEY_NPAGE )
		{
			output( "%s", "choice page down.\n" );
		}
		else if( ch == KEY_PPAGE )
		{
			output( "%s", "choice page up.\n" );
		}
		else
		{
			exit = (ch == 27 || ch == 'q');
		}

		wrefresh( show_w );
	}
	
	endwin();
#endif
	return 0;
}
