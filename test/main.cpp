#include "defines.h"
#include "frame.h"
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>

WINDOW * show_w;
int main( int argc, char* argv[] )
{
	int page = 0;
	int show = 10;

	int row_max, col_max;
	initscr();
	raw();
	noecho();
	curs_set(0);
	mousemask( ALL_MOUSE_EVENTS, NULL );
	printw( "unit test framework v1.0!\n");
	auto frame = get_test_frame();
	bool exit = false;

	getmaxyx(stdscr,row_max,col_max);

	WINDOW *test_border_w = newwin( 14, 60, 0, 0 );
	box( test_border_w, 0, 0 );
	wrefresh( test_border_w );

	WINDOW *test_w = newwin( 12, 58, 1, 1 );
	auto prepage = frame.tests( page, show );
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
	return 0;
}
