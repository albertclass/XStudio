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

	auto frame = get_test_frame();
	bool exit = false;
	while( exit == false )
	{
		puts( "+---------------------------------------" );
		page = frame.show_page( page, 10 );

		puts( "+---------------------------------------" ); 
		puts( "+ =. next page" );
		puts( "+ -. prev page" );
		puts( "+---------------------------------------" ); 
		puts( "q. exit" );

		int ch = getch();
		if( ch >= '0' && ch <= '9' )
		{
			auto choice = ch - '0';
			frame.exec_test( page, 10, choice );
		}
		else if( ch == '=' )
		{
			++page;
		}
		else if( ch == '-' )
		{
			--page;
		}
		else
		{
			exit = (ch == 27 || ch == 'q');
		}
	}

	return 0;
}
