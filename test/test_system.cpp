#include "frame.h"
#include "xsystem.h"

using namespace xgc;
using namespace xgc::common;

static int testmain( int agrc, char * argv[] )
{
	const char* module_name = get_module_name();
	const char* module_path = get_module_path();

	output( "module name = %s\n", module_name );
	output( "module path = %s\n", module_path );

	char path[XGC_MAX_PATH] = { 0 };
	get_normal_path( path, sizeof(path), "%s", "../../src/" );
	output( "normal path = %s\n", path );
}

UNIT_TEST( "system", "system test", testmain );
