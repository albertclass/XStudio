#ifndef _UNITE_FRAME_H_
#define _UNITE_FRAME_H_
#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

typedef int (*PFN_Test)();

struct unit_test
{
	const char* name;
	const char* display;

	PFN_Test entry;
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

	///
	/// \brief 注册测试项 
	/// \date 1/9/2018
	/// \author albert.xu
	///
	void regist( const char* name, const char * display, PFN_Test entry );

	///
	/// \brief 按页显示测试项 
	/// \date 1/9/2018
	/// \author albert.xu
	///
	int show_page( int page, int show );

	///
	/// \brief 测试给定的项 
	/// \date 1/9/2018
	/// \author albert.xu
	///
	void exec_test( int page, int show, int item );
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
	unit_test_register( const char * name, const char * display, PFN_Test entry )
	{
		get_test_frame().regist( name, display, entry );
	}
};

#define UNIT_TEST( NAME, DISPLAY, ENTRY )\
	static unit_test_register regist_test( NAME, DISPLAY, ENTRY );
	
int output( const char *fmt, ... );
#endif // _UNITE_FRAME_H_