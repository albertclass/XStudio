#include "frame.h"
#include "exception.h"
#include "debuger.h"

using namespace xgc;
using namespace xgc::common;

void call_1( int c );
void call_2( const char *s );
void crash()
{
	int *p = 0;
	*p = 5;
}

void call_1( int c )
{
	FUNCTION_BEGIN
	output( "call_1 c = %d\n", c );
	if( c > 0 )
	{
		call_1( c - 1 );
		crash();
	}
	FUNCTION_END
}

void call_2( const char *s )
{
	FUNCTION_BEGIN
	output( "call_2 s = %s\n", s );
	if( *s != 0 )
	{
		call_2( ++s );
		DumpStackFrame();
	}
	FUNCTION_END
}

static int testmain( int agrc, char * argv[] )
{
	InitException();
	FUNCTION_BEGIN
	const int thread_count = 100;
	std::thread t[thread_count];

	getInvokeWatcherMgr().Start();
	for( int i = 0; i < thread_count; ++i )
	{
		if( i % 2 )
			t[i] = std::thread( std::bind( call_1, 10 ) );
		else
			t[i] = std::thread( std::bind( call_2, "vklqawersdfaqer" ) );
	}

	for( int i = 0; i < thread_count; ++i )
	{
		t[i].join();
	}
	getInvokeWatcherMgr().Stop();
	FUNCTION_END
	FiniException();

	return 0;
}

UNIT_TEST( "debuger", "test debuger", testmain );
