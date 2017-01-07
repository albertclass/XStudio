#include "filewatcher.h"
#include "frame.h"
#include <thread>
#include <chrono>
#include <conio.h>

using namespace xgc;
using namespace xgc::common;

xgc_void notify( xgc_lpcstr path, xgc_lpcstr filename, xgc_ulong action )
{
	output( "%s/%s - %08x\n", path, filename, action );
}

static int testmain( int agrc, char * argv[] )
{
	init_filewatcher(0);
	add_filewatcher( "D:\\GitHub\\XStudio", FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE, true, notify, true );
	while( true )
	{
		if( _kbhit() )
		{
			int ch = _getch();
			if( ch == 27 || ch == 'q' )
				break;
		}

		if( FILE_WATCHER_NOCHANGE == do_filewatcher_notify( 1000 ) )
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	fini_filewatcher();
	return 0;
}

UNIT_TEST( "filewatcher", "filewatcher test", testmain );
