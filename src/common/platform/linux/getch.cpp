#if defined( __GNUC__ )
#include <termios.h>
#include <stdio.h>

static struct termios _old, _new;

/* Read 1 character - echo defines echo mode */
int getch_(int echo) 
{
	int ch;
	tcgetattr(0, &_old); /* grab old terminal i/o settings */
	_new = _old; /* make new settings same as old settings */
	_new.c_lflag &= ~ICANON; /* disable buffered i/o */
	_new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &_new); /* use these new terminal i/o settings now */
	ch = getchar();
	tcsetattr(0, TCSANOW, &_old);
	return ch;
}

/* Read 1 character without echo */
int _getch(void) 
{
	return getch_(0);
}

/* Read 1 character with echo */
int _getche(void) 
{
	return getch_(1);
}
#endif