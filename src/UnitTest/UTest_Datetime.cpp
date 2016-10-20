#include <stdlib.h>
#include <stdio.h>

#include "../Common/datetime.h"
using namespace xgc;
using namespace xgc::common;

void main()
{
	datetime dt = datetime::now();

	char sz[64];
	dt.to_string( sz );

	printf( "now is %s\n ", sz );
}