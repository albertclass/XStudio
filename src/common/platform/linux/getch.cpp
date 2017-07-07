#ifdef _LINUX
#include <termios.h>
#include <stdio.h>

static struct termios old, new;

/* Read 1 character - echo defines echo mode */
int getch_(int echo) 
{
	int ch;
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
	ch = getchar();
	tcsetattr(0, TCSANOW, &old);
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