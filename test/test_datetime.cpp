#include "frame.h"
#include "datetime.h"

using namespace xgc;
using namespace xgc::common;

static int testmain( int agrc, char * argv[] )
{
	char sz_date[64];
	char sz_time[64];

	char sz_datetime[64];

	datetime dt = datetime::now();

	output( "now = %s\n", dt.to_string( sz_datetime ) );

	timespan sp = timespan::convert( "1:00:00" );
	
	output( "now = %s\n", (dt + sp).to_string(sz_datetime) );

	datetime dt1 = datetime::convert( "2016-12-31" );
	datetime dt2 = datetime::convert( "2017-1-1" );

	char sz_dt1[64];
	char sz_dt2[64];
	output( "dt1 = %s, dt2 = %s, dt1 < dt2 is %s\n", dt1.to_string(sz_dt1), dt2.to_string(sz_dt2), dt1 < dt2 ? "true" : "false");
	dt1 += timespan::from_days(1);
	output( "dt1 = %s, dt2 = %s, dt1 = dt2 is %s\n", dt1.to_string(sz_dt1), dt2.to_string(sz_dt2), dt1 == dt2 ? "true" : "false");
	dt1 += timespan::from_days(32);
	output( "dt1 = %s, dt2 = %s, dt1 > dt2 is %s\n", dt1.to_string(sz_dt1), dt2.to_string(sz_dt2), dt1 > dt2 ? "true" : "false");

	dt1 = datetime::convert( "2077-10-1 12:32:57" );
	output( "dt1 = %s\n", dt1.to_string(sz_datetime) );
	output( "dt2 = %s\n", dt2.to_string(sz_datetime) );

	dt2.set_date( dt1.date() );
	output( "set date, dt2 = %s\n", dt2.to_string(sz_datetime) );

	dt2.set_time( dt1.time() );
	output( "set time, dt2 = %s\n", dt2.to_string(sz_datetime) );

	dt = datetime::from_ctime( time(nullptr) );
	output( "dt from ctime %s\n", dt.to_string(sz_datetime) );

	return 0;
}

UNIT_TEST( "datetime", "datetime test", testmain )
