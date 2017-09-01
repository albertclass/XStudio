// LuaDebugConsole.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "LuaDebugCommander.h"
bool work = true;

BOOL WINAPI HandlerRoutine( DWORD dwCtrlType )
{
	if( dwCtrlType == CTRL_CLOSE_EVENT )
	{
		SetEvent( GetStdHandle( STD_INPUT_HANDLE ) );
		work = false;
	}
	return TRUE;
}

bool PrintResult( char* data, size_t size, size_t maxsize )
{
	printf( (const char*)data );
	return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
	SetConsoleCtrlHandler( HandlerRoutine, TRUE );
	
	int ConsoleWidth = 120;
	int ConsoleHeight = 40;
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD size;
	size.X = ConsoleWidth;
	size.Y = ConsoleHeight*3;
	SetConsoleScreenBufferSize(hStd, size);
	SMALL_RECT rc = { 0, 0, ConsoleWidth -1, ConsoleHeight -1 };

	SetConsoleWindowInfo( hStd, TRUE, &rc );

	LuaDebugCommander commander;
	setlocale( LC_ALL, "chs");
	const char*	lpszPipename = "\\\\.\\pipe\\lua\\ui";
	CStringList l;
	extern bool pipelist( CStringList& l );
	if( pipelist( l ) )
	{
		int i = 0;
		CStringList::const_iterator c = l.begin();
		while( l.end() != c )
		{
			printf( "%03d %s\n", i++, c->c_str() );
			++c;
		}

		printf( "choice pipe ( 0 ~ %d ) :", i - 1 );
		char ch = _getch();
		int sel = ch - '0';
		if( sel >= 0 && sel < i )
		{
			lpszPipename = l[sel].c_str();
		}
	}
	else
	{
		puts( "no debug pipe" );
	}
	commander.initialize( lpszPipename, PrintResult );

	_tprintf( _T("连接LuaDebuger成功。\n") );
	TCHAR szCommand[256];
	DWORD dwSize = 0;
	commander.command( "cd ..\\Resource\\Scripts\\ui" );
	commander.waitSignal();
	commander.command( "open utility.lua" );
	commander.waitSignal();
	commander.command( "list" );
	commander.waitSignal();
	while(work)
	{
		_tprintf( _T("\n>>") );
		ReadConsole( GetStdHandle( STD_INPUT_HANDLE ), szCommand, _countof(szCommand), &dwSize, NULL );
		if( dwSize >= _countof(szCommand) )
		{
			break;
		}
		while( !_istprint( szCommand[dwSize-1] ) ) --dwSize;
		szCommand[dwSize] = 0;

		if( commander.command( szCommand ) )
		{
			commander.waitSignal();
		}
		Sleep(1);
	}
	return 0;
}
