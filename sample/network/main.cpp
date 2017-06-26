#include "header.h"

extern int main( int argc, char *argv[] );

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

	initscr();
	resize_term( 30, 120 );
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

	PDC_set_function_key( FUNCTION_KEY_ABORT, KEY_ALT_L & KEY_F(4) );
	int _exit_code = main( argc, (char **)argv);
	endwin();

	return _exit_code;
}
