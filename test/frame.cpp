#include "frame.h"
#include <ncurses.h>

extern WINDOW * show_w;

unit_test_frame &get_test_frame()
{
	static unit_test_frame inst;
	return inst;
}

int output( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	vwprintw( show_w, fmt, ap );
	va_end(ap);
}