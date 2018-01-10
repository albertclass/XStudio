#include "frame.h"
#include "logger.h"

using namespace xgc;
using namespace xgc::common;

#define LOG( NAME, TAGS, FMT, ... )	get_logger(NAME).write( logger_context( __FILE__, __FUNCTION__, __LINE__, TAGS ), FMT, ##__VA_ARGS__ )
#define SYS_LOG(FMT, ...) LOG("sys", "sys", FMT "\n", ##__VA_ARGS__ )
#define ERR_LOG(FMT, ...) LOG("err", "err", FMT "\n", ##__VA_ARGS__ )
#define USR_LOG(FMT, ...) LOG("usr", "usr", FMT "\n", ##__VA_ARGS__ )

static int testmain()
{
	init_logger( "../logger.ini" );

	char buf[] = "0123456789ABCDEF";
	for( int i = 1; i < 1024 * 32; ++i )
	{
		SYS_LOG("%05d - %s", i, buf);
		USR_LOG("%05d - %s", i, buf);
	}
	fini_logger();

	return 0;
}

UNIT_TEST( "logger", "test logger", testmain );