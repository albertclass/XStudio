// MessageCreator.cpp : 定义控制台应用程序的入口点。
//
#include "parse.h"
#include <conio.h>
#include <direct.h>
#include <process.h>

using namespace std;
int main(int argc, char* argv[])
{
	root r;
	int ierror = 0;

	list< string > filelist;
	r.config.outdir = ".\\";
	for( int i = 1; i < argc; ++i )
	{
		if( argv[i][0] != '-' && argv[i][0] != '/' )
		{
			filelist.push_back( argv[i] );
		}
		else if( _strnicmp( argv[i], "-outdir", 7 ) == 0 && argv[i][7] == ':' )
		{
			r.config.outdir = argv[i]+8;
			mkdir( r.config.outdir.c_str() );
		}
		else if( _strnicmp( argv[i], "-prefix", 7 ) == 0 && argv[i][7] == ':' )
		{
			r.config.prefix = argv[i]+8;
		}
		else if( _strnicmp( argv[i], "-userdef", 8 ) == 0 && argv[i][8] == ':' )
		{
			r.config.userdef = argv[i]+9;
		}
		else if( _strnicmp( argv[i], "-yu", 3 ) == 0 && argv[i][3] == ':' )
		{
			r.config.yu = argv[i]+4;
		}
	}

	for( list<string>::iterator i = filelist.begin(); i != filelist.end(); ++i )
	{
		ierror += analysefile( &r, i->c_str() );
	}

	if( ierror == 0 )
	{
		writefile( &r );
		destroyall( &r );
	}
	else
	{
		puts( "error found program abort!" );
	}

	return 0;
}
