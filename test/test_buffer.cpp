#include "frame.h"
#include "xbuffer.h"

using namespace xgc;
using namespace xgc::common;

static int testmain( int agrc, char * argv[] )
{
	char buffer[11];
	output( "------------------------------------------\n" );
	output( "test shared memory buffer\n" );
	output( "------------------------------------------\n" );
	shared_memory_buffer shm;
	if( shm.create( "gs_buffer", 64, "./" ) != -1 )
	{
		ring_buffer< reference_buffer > rb( shm );

		for( int i = 0; i < 100; ++i )
		{
			rb.write_overflow( "1234567890\n", 11 );
			rb.read_some(buffer, 10);
		}
	}

	output( "%s\n", (char*)shm.base() );


	output( "------------------------------------------\n" );
	output( "test static buffer\n" );
	output( "------------------------------------------\n" );

	static_buffer< 64 > sb;
	ring_buffer< reference_buffer > rb( sb );

	for( int i = 0; i < 100; ++i )
	{
		rb.write_overflow( "1234567890\n", 11 );
		rb.read_some(buffer, 10);
	}

	output( "%s\n", (char*)sb.base() );

}

UNIT_TEST( "buffer", "buffer test", testmain );
