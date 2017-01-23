#include "frame.h"
#include "thread_pool.h"

using namespace xgc;
using namespace xgc::common;

int got_sum( int arr[], int count )
{
	int sum = 0;
	for( int i = 0; i < count; ++i )
	{
		sum += arr[i];
	}

	return sum;
}

class theClass
{
public:
	int got_sum( int arr[], int count )
	{
		int sum = 0;
		for( int i = 0; i < count; ++i )
		{
			sum += arr[i];
		}

		return sum;
	}
};

static int testmain( int agrc, char * argv[] )
{
	thread_pool pool(16);

	int arr[] = { 1, 4, 7, 16, 32, 99, 87, 1230, 3334 };
	std::future<int> ret = pool.commit( got_sum, arr, (int)XGC_COUNTOF(arr) );
	int sum = ret.get();

	theClass cls;
	ret = pool.commit( &theClass::got_sum, &cls, arr, (int)XGC_COUNTOF(arr) );
	sum = ret.get();

	pool.shutdown();
	return 0;
}

UNIT_TEST( "thread", "test thread pool", testmain );
