#ifndef _UNITE_FRAME_H_
#define _UNITE_FRAME_H_
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

typedef int (*pfn_test)( int argc, char * argv[] );

struct unit_test
{
	const char* name;
	const char* display;

	pfn_test entry;
};

class unit_test_frame
{
public:
	unit_test_frame()
	{

	}

	~unit_test_frame()
	{

	}

	void regist( const char* name, const char * display, pfn_test entry )
	{
		unit_test test;
		test.name = name;
		test.display = display;
		test.entry = entry;

		view.push_back( test );
	}

	std::vector< const unit_test* > tests( int page, int show )
	{
		std::vector< const unit_test* > prepage;
		auto count = view.size();
		auto last = ( count + show - 1 ) / show;

		if( page > last )
			return prepage;

		for( auto i = page * show; i < (page + 1) * show && i < count; ++i )
			prepage.push_back( &view[i] );

		return prepage;
	}
private:
	std::vector< unit_test > view;
};

#ifdef _LINUX
int getch();
int kbhit();
#endif // _LINUX

unit_test_frame &get_test_frame();

struct unit_test_register
{
	unit_test_register( const char * name, const char * display, pfn_test entry )
	{
		get_test_frame().regist( name, display, entry );
	}
};

#define UNIT_TEST( NAME, DISPLAY, ENTRY )\
	static unit_test_register regist_test( NAME, DISPLAY, ENTRY );
	
int output( const char *fmt, ... );
#endif // _UNITE_FRAME_H_