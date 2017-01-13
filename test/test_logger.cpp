#include "frame.h"
#include "logger.h"

using namespace xgc;

static int testmain( int agrc, char * argv[] )
{
	init_logger();
}

UNIT_TEST( "logger", "test logger", testmain );