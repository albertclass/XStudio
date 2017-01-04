#include "config.h"
#include "frame.h"
unit_test_frame &get_test_frame()
{
	static unit_test_frame inst;
	return inst;
}

#ifdef _WINDOWS
#include "console.h"
int output( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	int bytes = vprintf_s( fmt, ap );
	va_end(ap);

	return bytes;
}
#else
#include <ncurses.h>
extern WINDOW * show_w;

int output( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	int bytes = vwprintw( show_w, fmt, ap );
	va_end(ap);

	return bytes;
}
#endif