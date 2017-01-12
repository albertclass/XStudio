#include "frame.h"
#include "xsystem.h"
#include "ini_reader.h"

using namespace xgc;
using namespace xgc::common;

static int testmain( int agrc, char * argv[] )
{
	xgc_char file[XGC_MAX_PATH];
	get_normal_path( file, "../conf.ini" );

	ini_reader ini;
	ini.load( file );

	auto sections = ini.get_section_count();
	for( size_t i = 0; i < sections; ++i )
	{
		auto sec = ini.get_section( i );
		output( "\n[%s]\n", ini.get_section_name(sec) );
		auto items = ini.get_item_count(sec);
		for( size_t item = 0; item < items; ++item )
		{
			auto key = ini.get_item_name(sec, item);
			auto val = ini.get_item_value(sec, item, "<<empty>>");
			output( "%s=%s\n", key, val );
		}
	}

	output( "\ntest get_item_value\n");
	auto aaa = ini.get_item_value<int>( "Params", "aaa", 0 );
	output( "Params.aaa=%d\n", aaa );

	auto bbb = ini.get_item_value<float>( "Params", "bbb", 0.0 );
	output( "Params.bbb=%lf\n", bbb );

	auto ccc = ini.get_item_value<double>( "Params", "ccc", 0.0 );
	output( "Params.ccc=%lf\n", ccc );

	output( "\ntest get_item_count(section_name)\n");
	auto item_count = ini.get_item_count( "Params" );
	for( xgc_size i = 0; i < item_count; ++i )
	{
		auto key = ini.get_item_name("Params", i);
		auto val = ini.get_item_value("Params", i, "<<empty>>" );
		output( "%s=%s\n", key, val );
	}

	output( "\ntest get_item_count(section_name, item_name)\n");
	item_count = ini.get_item_count( "Params", "ppp" );
	for( xgc_size i = 0; i < item_count; ++i )
	{
		auto val = ini.get_item_value("Params", "ppp", i, "<<empty>>" );
		output( "ppp=%s\n", val );
	}
} 

UNIT_TEST( "ini_reader", "ini read test", testmain );