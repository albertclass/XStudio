#include "stdafx.h"
#include "console.h"
#include "screen.h"

using namespace xgc;

static console::window_t g_window_input  = INVALID_WINDOW_INDEX;
static console::window_t g_window_logger = INVALID_WINDOW_INDEX;
static console::window_t g_window_server = INVALID_WINDOW_INDEX;
static console::window_t g_window_stdout = INVALID_WINDOW_INDEX;

static console::buffer_t g_buffer_input  = INVALID_BUFFER_INDEX;
static console::buffer_t g_buffer_logger = INVALID_BUFFER_INDEX;
static console::buffer_t g_buffer_server = INVALID_BUFFER_INDEX;
static console::buffer_t g_buffer_stdout = INVALID_BUFFER_INDEX;

BOOL WINAPI HandlerRoutine( DWORD dwCtrlType )
{
	switch( dwCtrlType )
	{
		case CTRL_C_EVENT:
		XGC_DEBUG_MESSAGE( "CTRL_C_EVENT" );
		break;
		case CTRL_BREAK_EVENT:
		XGC_DEBUG_MESSAGE( "CTRL_BREAK_EVENT" );
		break;
		case CTRL_CLOSE_EVENT:
		XGC_DEBUG_MESSAGE( "CTRL_CLOSE_EVENT" );
		break;
		case CTRL_LOGOFF_EVENT:
		XGC_DEBUG_MESSAGE( "CTRL_LOGOFF_EVENT" );
		break;
		case CTRL_SHUTDOWN_EVENT:
		XGC_DEBUG_MESSAGE( "CTRL_SHUTDOWN_EVENT" );
		break;
	}

	return TRUE;
}

xgc_void InitializeConsole( ini_reader &ini )
{
	FUNCTION_BEGIN;
	xgc_int16 cols = ini.get_item_value( "Console", "Cols", 120 );
	xgc_int16 rows = ini.get_item_value( "Console", "Rows", 40 );

	xgc_bool redirect = ini.get_item_value( "Console", "Redirect", true );

	// 初始化
	console::console_init( cols, rows, HandlerRoutine );

	g_window_server = console::window( 0, 0, 80, 10, g_buffer_server, WINDOW_STYLE_DEFAULT, "[Server Infomation]" );
	g_window_logger = console::window( 0, 9, 80, 16, g_buffer_logger, WINDOW_STYLE_DEFAULT, "[Server Log]" );

	g_window_stdout = console::window( 0, 24, 80, 14, g_buffer_stdout, WINDOW_STYLE_DEFAULT, "[STDIO]" );
	g_window_input  = console::window( 0, rows - 2, cols, 1, g_buffer_input, WINDOW_STYLE_BORDER | WINDOW_STYLE_EDITOR );

	if( redirect )
	{
		console::redirect( stdout, g_buffer_stdout );
		console::redirect( stderr, g_buffer_stdout );
	}

	FUNCTION_END;
}

xgc_void FinializeConsole()
{
	FUNCTION_BEGIN;
	if( g_window_server != INVALID_WINDOW_INDEX )
	{
		console::free_window( g_window_server );
		g_window_server = INVALID_WINDOW_INDEX;
	}
	if( g_window_logger != INVALID_WINDOW_INDEX )
	{
		console::free_window( g_window_logger );
		g_window_logger = INVALID_WINDOW_INDEX;
	}
	if( g_window_input != INVALID_WINDOW_INDEX )
	{
		console::free_window( g_window_input );
		g_window_input = INVALID_WINDOW_INDEX;
	}

	console::console_fini();
	FUNCTION_END;
}

xgc_void ConsoleLog( xgc_lpcstr pszLogText )
{
	if( console::console_is_init() )
	{
		console::printf_text( g_buffer_logger, "%s\n", pszLogText );
	}
}

xgc_void ConsoleServer( xgc_lpcstr pszLogText )
{
	if( console::console_is_init() )
	{
		console::printf_text( g_buffer_server, "%s\n", pszLogText );
	}
}

extern xgc_int32 ProcessConsoleInput( xgc_lpcstr pszCommandLine );

xgc_int32 ConsoleUpdate()
{
	xgc_char CommandLine[1024];

	// 控制台输出 [2/24/2014 xufeng04]
	//if( !console::update_input( g_window_input, CommandLine, sizeof( CommandLine ) ) )
	//	return 0;
	auto len = console::buffer_read(g_buffer_input, CommandLine, sizeof(CommandLine), false);
	if( len > 0)
	{
		if (CommandLine[len -1] == '\n')
			CommandLine[len -1] = 0;
		return ProcessConsoleInput( CommandLine );
	}
	return 0;
}
