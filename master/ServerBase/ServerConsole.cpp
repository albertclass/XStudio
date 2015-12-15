#include "stdafx.h"
#include "console.h"
#include "ServerConsole.h"

using namespace XGC;

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

xgc_void InitializeConsole( IniFile &ini )
{
	FUNCTION_BEGIN;
	xgc_uint16 nRows = ini.GetItemValue( "Console", "Rows", 120 );
	xgc_uint16 nCols = ini.GetItemValue( "Console", "Cols", 40 );

	xgc_bool bRedirect = ini.GetItemValue( "Console", "Redirect", true );

	// 初始化
	console::initialize_console_library( nRows, nCols, HandlerRoutine );
	xgc_uint16 console_rows = console::get_console_rows();
	xgc_uint16 console_cols = console::get_console_cols();

	g_window_server = console::window( 0, 0, console_cols, 10, g_buffer_server );
	g_window_logger = console::window( 0, 9, console_cols, 16, g_buffer_logger );

	g_window_stdout = console::window( 0, 24, console_cols, 14, g_buffer_stdout );
	g_window_input  = console::window( 0, console_rows - 2, console_cols, 1, g_buffer_input, false );

	console::show_cursor( false );
	console::draw_window( g_window_server );
	console::draw_window( g_window_logger );
	console::draw_window( g_window_stdout );

	console::active_window( g_window_logger );

	if( bRedirect )
	{
		console::redirect( stdout, g_buffer_stdout );
		console::redirect( stderr, g_buffer_stdout );
	}

	console::printf_text( g_buffer_input, "控制台输出模块初始化完成!" );

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

	console::finialize_console_library();
	FUNCTION_END;
}

xgc_void ConsoleLog( xgc_lpcstr pszLogText )
{
	if( console::is_console_library() )
	{
		console::printf_text( g_buffer_logger, "%s\n", pszLogText );
	}
}

extern xgc_int32 ProcessConsoleInput( xgc_lpcstr pszCommandLine );

xgc_int32 ConsoleUpdate()
{
	xgc_char CommandLine[1024];

	// 控制台输出 [2/24/2014 xufeng04]
	if( !console::update_input( g_window_input, CommandLine, sizeof( CommandLine ) ) )
		return 0;

	return ProcessConsoleInput( CommandLine );
}
