#include "defines.h"
#include "exports.h"

#include "filewatcher.h"
#include "frame.h"
#include <thread>
#include <chrono>
#ifdef _WINDOWS
#	include <conio.h>
#	define FILE_WATCHER_FILTER (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE)
#elif defined(_LINUX)
#	include <sys/inotify.h>
#	define FILE_WATCHER_FILTER (IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE)
#endif

using namespace xgc;
using namespace xgc::common;

xgc_void notify( xgc_lpcstr path, xgc_lpcstr filename, xgc_ulong action )
{
	output( "%s/%s - %08x\n", path, filename, action );
}

static int testmain()
{
	init_filewatcher(0);
	#if defined(_WINDOWS)
	auto ret = add_filewatcher( "D:\\GitHub\\XStudio", FILE_WATCHER_FILTER, true, notify, true );
	#endif

	#if defined(_LINUX)
	auto ret = add_filewatcher( "/home/albert/Workspace/XStudio/", FILE_WATCHER_FILTER, true, notify, true );
	#endif

	XGC_ASSERT_RETURN( ret == FILE_WATCHER_SUCCESS, -1 );

	output( "file watcher start...\n" );
	while( true )
	{
		#if defined(_WINDOWS)
		if( _kbhit() )
		{
			int ch = _getch();
			if( ch == 27 || ch == 'q' )
				break;
		}
		#elif defined(_LINUX)
		if( kbhit() )
		{
			int ch = getch();
			if( ch == 27 || ch == 'q' )
				break;
		}
		#endif

		if( FILE_WATCHER_NOCHANGE == do_filewatcher_notify( 1000 ) )
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	output( "file watcher end...\n" );
	fini_filewatcher();
	return 0;
}

UNIT_TEST( "filewatcher", "test filewatcher", testmain );
