#include "defines.h"
#include "frame.h"
#ifdef _WINDOWS
#	include <conio.h>
#endif

#ifdef _LINUX
#	include <sys/select.h>
#	include <sys/types.h>
#	include <sys/time.h>
#	include <termios.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <stdio.h>
#endif


int main( int argc, char* argv[] )
{
	int page = 0;
	int show = 10;

	auto frame = get_test_frame();
	auto prepage = frame.tests( page, show );

	bool exit = false;
	while( exit == false )
	{
		int i = 0;
		for( auto &it : prepage )
		{
			printf( "%d. %s\n", i++, it->display );
		}

		printf( "q. exit\n" );

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
	}

	return 0;
}
