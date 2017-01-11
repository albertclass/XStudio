#include "frame.h"
#include "xsystem.h"
#include "csv_reader.h"

using namespace xgc;
using namespace xgc::common;

static int testmain( int agrc, char * argv[] )
{
	xgc_char file[XGC_MAX_PATH];
	get_normal_path( file, "../table.csv" );

	csv_reader csv;
	csv.load( file, '\t', false );

	auto rows = csv.get_rows();
	auto cols = csv.get_cols();

	for( xgc_size row = 0; row < rows; ++row )
	{
		for( xgc_size col = 0; col < cols; ++col )
		{
			auto value = csv.get_value(row, col, "empty" );
			output( "%c%s", col ? '\t' : ' ', value );
		}

		output( "\n" );
	}
}

UNIT_TEST( "csv_reader", "csv read test", testmain );