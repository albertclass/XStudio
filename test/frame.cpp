#include "config.h"
#include "frame.h"

#ifdef _LINUX
#	include <sys/select.h>
#	include <sys/types.h>
#	include <sys/time.h>
#	include <termios.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <stdio.h>
#endif

#ifdef _LINUX
int getch()
{
  struct termios oldt, newt;
  int ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;	
}

int kbhit()
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
  	return 0;
}

#endif // _LINUX

unit_test_frame &get_test_frame()
{
	static unit_test_frame inst;
	return inst;
}

int output( const char *fmt, ... )
{
	va_list ap;
	va_start( ap, fmt );
	int bytes = vprintf( fmt, ap );
	va_end(ap);

	return bytes;
}