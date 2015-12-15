#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <process.h>
#include "console.h"
#include "synchronous.h"
#include "logger.h"

namespace XGC
{
	namespace console
	{
		using XGC::common::autolock;
		enum text_border
		{
			text_top,
			text_left,
			text_right,
			text_bottom,

			text_topleft,
			text_bottomleft,
			text_topright,
			text_bottomright,

			text_number,
			text_count,
		};

		enum buffer_flags : xgc_uint16
		{
			eTextEdit  = 1,
			eTextDirty = 2,
			eTextFlagCount,
		};

		struct text_buffer
		{
			xgc_uint16	w, h;	// buffer size
			xgc_uint16	position;
			xgc_uint16	flags;	// buffer flags
			PCHAR_INFO	buffer; // buffer;

			CRITICAL_SECTION critical;
		};

		struct text_window
		{
			xgc_uint16 x, y;		// position of global window
			xgc_uint16 cols, rows;	// window size
			xgc_uint16 sx, sy;		// current scroll position

			xgc_uint16 cx;
			xgc_uint16 cy;			// screen cursor for input;

			xgc_uint16 redraw;		// window need redraw

			PCHAR_INFO border;		// has border 
			PCHAR_INFO title;		// has title

			buffer_t text;			// text buffer handle

			CRITICAL_SECTION critical;
		};

		const xgc_int16 window_border_row_fix = 2;
		const xgc_int16 window_border_col_fix = 2;

		static XGC_INLINE xgc_uint16 window_rows( const text_window& w )
		{
			return w.border ? w.rows - window_border_row_fix : w.rows;
		}

		static XGC_INLINE xgc_uint16 window_cols( const text_window& w )
		{
			return w.border ? w.cols - window_border_col_fix : w.cols;
		}

		struct console_global
		{
			xgc_word current;
			xgc_word blank;
			xgc_word border[text_count * 2];
		};

		static console_global g =
		{
			FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
			0,
			{ 
				'-', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'|', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'|', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'-', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'.', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'.', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'.', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'.', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
				'+', FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_BLUE,
			},
		};

		static text_window _text_window[255];
		static text_buffer _text_buffer[255];

		static CRITICAL_SECTION _sort_window_critical;
		static window_t _sort_window[255];

		struct redirect
		{
			int fd[2];
			int old;
			HANDLE cout;
			buffer_t buffer;
			xgc_word color;
		};

		static struct redirect gstdio[3] =
		{
			{ { -1, -1 }, -1, GetStdHandle( STD_INPUT_HANDLE  ), INVALID_WINDOW_INDEX },
			{ { -1, -1 }, -1, GetStdHandle( STD_OUTPUT_HANDLE ), INVALID_WINDOW_INDEX },
			{ { -1, -1 }, -1, GetStdHandle( STD_ERROR_HANDLE  ), INVALID_WINDOW_INDEX },
		};

		static volatile xgc_long g_console_library = 0;
		static xgc_handle g_console_thread = xgc_nullptr;

		unsigned int _stdcall console_update_thread( xgc_lpvoid _param )
		{
			char buf[4096];
			InterlockedExchange( &g_console_library, 1UL );
			while( g_console_library )
			{
				for( int i = 0; i < 3; ++i )
				{
					if( gstdio[i].fd[0] == -1 )
						continue;

					while( !_eof( gstdio[i].fd[0] ) )
					{
						int cnt = _read( gstdio[i].fd[0], buf, sizeof( buf ) - 1 );
						buf[cnt] = 0;
						if( g_console_library == 0 )
							break;
						printf_text( gstdio[i].buffer, "%s", buf );
					}
				}

				autolock locker( _sort_window_critical );
				for( auto i = 0; i < XGC_COUNTOF( _sort_window ); ++i )
				{
					if( _sort_window[i] == INVALID_WINDOW_INDEX )
						break;

					draw_window( _sort_window[i], false );
				}
				locker.free_lock();

				Sleep( 100 );
			}

			return 0;
		}

		xgc_bool initialize_console_library( xgc_int16 rows, xgc_int16 cols, PHANDLER_ROUTINE pfnHandler )
		{
			if( pfnHandler )
			{
				SetConsoleCtrlHandler( pfnHandler, TRUE );
			}

			HMENU hSysMenu = GetSystemMenu( GetConsoleWindow(), FALSE );
			if( hSysMenu != 0 )
			{
				RemoveMenu( hSysMenu, SC_CLOSE, MF_BYCOMMAND );
			}

			memset( _text_window, 0, sizeof( _text_window ) );
			memset( _text_buffer, 0, sizeof( _text_buffer ) );
			memset( _sort_window, 0, sizeof( _sort_window ) );

			for( xgc_size i = 0; i < XGC_COUNTOF( _text_window ); ++i )
			{
				_text_window[i].text   = INVALID_BUFFER_INDEX;
				_text_window[i].border = xgc_nullptr;
				_text_window[i].title  = xgc_nullptr;
				_text_window[i].x      = 0;
				_text_window[i].y      = 0;
				_text_window[i].cx     = 0;
				_text_window[i].cy     = 0;
				_text_window[i].sx     = 0;
				_text_window[i].sy     = 0;

				InitializeCriticalSection( &_text_window[i].critical );
			}

			for( xgc_size i = 0; i < XGC_COUNTOF( _text_buffer ); ++i )
			{
				_text_buffer[i].buffer       = 0;
				_text_buffer[i].h            = 0;
				_text_buffer[i].w            = 0;
				_text_buffer[i].flags        = 0;

				InitializeCriticalSection( &_text_buffer[i].critical );
			}
				
			InitializeCriticalSection( &_sort_window_critical );
			for( xgc_size i = 0; i < XGC_COUNTOF( _sort_window ); ++i )
			{
				_sort_window[i] = INVALID_WINDOW_INDEX;
			}

			COORD dwConsoleBufferSize = { rows, cols };
			SMALL_RECT rcWindow = { 0, 0, rows - 1, cols - 1 };

			BOOL
			bRet = SetConsoleScreenBufferSize( gstdio[1].cout, dwConsoleBufferSize );
			bRet = SetConsoleWindowInfo( gstdio[1].cout, true, &rcWindow );

			g_console_thread = (xgc_handle)_beginthreadex( xgc_nullptr, 0, console_update_thread, xgc_nullptr, 0, xgc_nullptr );
			return true;
		}

		xgc_void finialize_console_library()
		{
			// 退出
			InterlockedExchange( &g_console_library, 0UL );
			for( xgc_size i = 0; i < XGC_COUNTOF( gstdio ); ++i )
			{
				if( -1 != gstdio[i].fd[1] )
					_write( gstdio[i].fd[1], "end", 4 );
			}

			WaitForSingleObject( (HANDLE) g_console_thread, INFINITE );

			for( buffer_t i = 0; i < XGC_COUNTOF( _text_buffer ); ++i )
			{
				free_text( i );
				DeleteCriticalSection( &_text_buffer[i].critical );
			}

			for( window_t i = 0; i < XGC_COUNTOF( _text_window ); ++i )
			{
				DeleteCriticalSection( &_text_window[i].critical );
			}

			for( xgc_size i = 0; i < XGC_COUNTOF( _sort_window ); ++i )
			{
				_sort_window[i] = INVALID_WINDOW_INDEX;
			}

			DeleteCriticalSection( &_sort_window_critical );

			for( xgc_size i = 0; i < XGC_COUNTOF( gstdio ); ++i )
			{
				if( -1 != gstdio[i].fd[1] )
				{
					_close( gstdio[i].fd[1] );
					gstdio[i].fd[1] = -1;
				}

				if( -1 != gstdio[i].fd[0] )
				{
					_close( gstdio[i].fd[0] );
					gstdio[i].fd[0] = -1;
				}

				if( -1 != gstdio[i].old )
				{
					if( 0 == _dup2( gstdio[i].old, (int)i ) )
						_close( gstdio[i].old );

					gstdio[i].old = -1;
				}
			}
		}

		///
		/// 屏幕是否被初始化了.
		/// [6/4/2015] create by albert.xu
		///
		xgc_bool is_console_library()
		{
			return g_console_library != 0;
		}

		// 根据缓冲区光标位置，重新计算窗口偏移
		xgc_void adjust_window( text_window& w )
		{
			text_buffer& t = _text_buffer[w.text];
			autolock locker( t.critical );
			XGC_ASSERT_RETURN( t.buffer, void( 0 ) );

			xgc_uint16 pos_x = t.position % t.w;
			xgc_uint16 pos_y = t.position / t.w;

			XGC_ASSERT_RETURN( pos_x < t.w && pos_y < t.h, void( 0 ) );

			w.sy = pos_y > window_rows( w ) ? pos_y - window_rows( w ) : 0;
			w.sx = pos_x > window_cols( w ) ? pos_x - window_cols( w ) : 0;
		}

		// 设置输出颜色
		xgc_void setcolor( xgc_word color )
		{
			g.current	= color;
			g.blank		= color;
		}

		// 向上滚动窗口row行
		static xgc_void scroll_buffer( text_buffer& t, xgc_uint16 row )
		{
			if( row == 0 )
				return;

			row = XGC_MIN( row, t.h );
			memmove( t.buffer, t.buffer + ( row * t.w ), ( t.h - row ) * t.w * sizeof( CHAR_INFO ) );
			CHAR_INFO chBlank;
			chBlank.Char.AsciiChar = ' ';
			chBlank.Attributes = g.blank;
			for( int i = ( t.h - row ) * t.w; i < t.w * t.h; ++i )
				t.buffer[i] = chBlank;

			if( t.position > row * t.w )
				t.position -= row * t.w;
			else
				t.position = 0;

			t.flags |= eTextDirty;
		}

		// 重定向标准输出
		xgc_bool redirect( FILE* file, buffer_t buffer )
		{
			int stdfd = _fileno( file );
			XGC_ASSERT_RETURN( stdfd >= 0 && stdfd < 3, false );
			
			int fd = _dup( stdfd );
			if( -1 == fd )
				return false;

			gstdio[stdfd].old = fd;
			gstdio[stdfd].cout = (xgc_handle)_get_osfhandle( fd );

			if( -1 == gstdio[stdfd].old )
				return false;

			if( -1 == _pipe( gstdio[stdfd].fd, 4096, _O_BINARY ) )
				return false;

			if( -1 == _dup2( gstdio[stdfd].fd[1], stdfd ) )
				return false;

			gstdio[stdfd].buffer = buffer;

			setvbuf( file, xgc_nullptr, _IONBF, 0 );
			return true;
		}

		// 输入更新
		xgc_size update_input( window_t window, xgc_lpstr buf, xgc_size len )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, false );
			autolock window_locker( _text_window[window].critical );
			text_window& w = _text_window[window];

			XGC_ASSERT_RETURN( w.text != INVALID_BUFFER_INDEX, false );
			autolock buffer_locker( _text_buffer[w.text].critical );
			text_buffer& t = _text_buffer[w.text];

			if( _kbhit() )
			{
				xgc_char ch = _getch();
				xgc_size position = t.position / t.w * t.w;
				switch( ch )
				{
				case 0x1b: // ESC
					if( !(t.flags & eTextEdit) )
					{
						t.flags = t.flags | eTextEdit;
					}
					scroll_buffer( t, 1 );
					show_cursor();
					break;
				case '\r': // return
					if( t.flags & eTextEdit )
					{
						t.flags = t.flags & ( ~eTextEdit );

						// 赋值数据到缓冲区
						for( xgc_size i = 0; i < len && i < t.w && position < t.position; ++i )
						{
							buf[i] = t.buffer[position].Char.AsciiChar;
							++position;
						}

						{
							xgc_size count = t.position - t.position / t.w * t.w;
							buf[count] = 0;
							scroll_buffer( t, 1 );
							show_cursor( false );
							draw_window( window, false );
							return count;
						}
					}
					break;
				case '\b': // backspace
					// 行首
					if( ( t.flags & eTextEdit ) && t.position % t.w != 0 )
					{
						xgc_size offset = t.position % t.w;
						// 将该行之后的向前移位;
						memmove( t.buffer + offset - 1, t.buffer + offset, (t.w - offset) * sizeof(t.buffer[0]) );
						t.buffer[position + t.w - 1].Attributes = g.blank;
						t.buffer[position + t.w - 1].Char.AsciiChar = ' ';
						t.position--;
						draw_window( window, false );
					}
					break;
				default:
					if( ( t.flags & eTextEdit ) )
						printf_text( w.text, "%c", ch );
				}

				gotoxy( window, t.position % t.w, t.position / t.w );
			}

			return 0;
		}

		// 获取控制台窗口宽度
		xgc_uint16	get_console_cols()
		{
			CONSOLE_SCREEN_BUFFER_INFOEX ConsoleInfo;
			ZeroMemory( &ConsoleInfo, sizeof( ConsoleInfo ) );
			ConsoleInfo.cbSize = sizeof( ConsoleInfo );

			::GetConsoleScreenBufferInfoEx( gstdio[1].cout, &ConsoleInfo );

			return ConsoleInfo.dwSize.X;
		}

		// 获取控制台窗口高度
		xgc_uint16 get_console_rows()
		{
			CONSOLE_SCREEN_BUFFER_INFOEX ConsoleInfo;
			ZeroMemory( &ConsoleInfo, sizeof( ConsoleInfo ) );
			ConsoleInfo.cbSize = sizeof( ConsoleInfo );

			::GetConsoleScreenBufferInfoEx( gstdio[1].cout, &ConsoleInfo );

			return ConsoleInfo.dwSize.Y;
		}

		// 隐藏光标
		xgc_void show_cursor( xgc_bool show )
		{
			CONSOLE_CURSOR_INFO CursorInfo;
			if( GetConsoleCursorInfo( gstdio[1].cout, &CursorInfo ) )
			{
				CursorInfo.bVisible = show;
				CursorInfo.dwSize = 100;
				SetConsoleCursorInfo( gstdio[1].cout, &CursorInfo );
			}
		}

		xgc_bool gotoxy( window_t window, xgc_int16 col, xgc_int16 row )
		{
			COORD cursor_position = { col, row };
			if( window != INVALID_WINDOW_INDEX )
			{
				text_window& w = _text_window[window];
				autolock window_locker( w.critical );

				w.cx = col;
				w.cy = row;

				XGC_ASSERT_RETURN( w.text != INVALID_BUFFER_INDEX, false );

				text_buffer& t = _text_buffer[w.text];
				autolock buffer_locker( t.critical );

				XGC_ASSERT_RETURN( t.buffer, false );

				// 根据所给坐标,计算text的光标位置.
				col = XGC_MAX( 0, col );
				row = XGC_MAX( 0, row );

				if( col >= t.w || row >= t.h )
					return false;

				t.position = ( row + w.sy ) * t.w + col + w.sx;

				// 根据光标位置对齐窗口
				adjust_window( w );

				// 重绘窗口文本
				draw_window( window );

				// 计算屏幕坐标
				xgc_int16 fix = w.border ? 1 : 0;
				cursor_position.X = w.x + col + fix;
				cursor_position.Y = w.y + row + fix;
			};

			return TRUE == SetConsoleCursorPosition( gstdio[1].cout, cursor_position );
		}

		buffer_t buffer( xgc_uint16 w, xgc_uint16 h )
		{
			buffer_t idx = 0;
			while( idx < XGC_COUNTOF( _text_buffer ) )
			{
				autolock locker( _text_buffer[idx].critical );
				if( _text_buffer[idx].buffer == xgc_nullptr )
				{
					_text_buffer[idx].buffer = (PCHAR_INFO) malloc( sizeof(CHAR_INFO) * w * h );
					if( _text_buffer[idx].buffer == xgc_nullptr )
						return -1;

					memset( _text_buffer[idx].buffer, 0, sizeof(CHAR_INFO) * w * h );

					_text_buffer[idx].w        = w;
					_text_buffer[idx].h        = h;
					_text_buffer[idx].position = 0;
					_text_buffer[idx].flags    = 0;

					for( int i = 0; i < w * h; ++i )
					{
						_text_buffer[idx].buffer[i].Attributes     = g.blank;
						_text_buffer[idx].buffer[i].Char.AsciiChar = ' ';
					}

					return idx;
				}
				++idx;
			}
			return INVALID_BUFFER_INDEX;
		}

		window_t window( xgc_int16 x, xgc_int16 y, xgc_int16 cols, xgc_int16 rows, buffer_t &t, xgc_bool border /*= true*/ )
		{
			window_t idx = 0;

			// 查找一个空的位置
			while( idx < XGC_COUNTOF( _text_window ) )
			{
				autolock locker( _text_window[idx].critical );
				if( _text_window[idx].text == INVALID_BUFFER_INDEX )
				{
					_text_window[idx].x = x;
					_text_window[idx].y = y;
					_text_window[idx].cols = cols;
					_text_window[idx].rows = rows;
					_text_window[idx].sx = 0;
					_text_window[idx].sy = 0;

					_text_window[idx].border = border ? (PCHAR_INFO) g.border : xgc_nullptr;
					_text_window[idx].title = xgc_nullptr;

					// 如果是一个无效的缓冲句柄，则创建一个。
					if( t == INVALID_BUFFER_INDEX )
					{
						if( border )
							t = buffer( cols - window_border_col_fix, rows - window_border_row_fix );
						else
							t = buffer( cols, rows );
					}

					XGC_ASSERT_RETURN( t != INVALID_BUFFER_INDEX, INVALID_WINDOW_INDEX );
					_text_window[idx].text = t;
					
					active_window( idx );

					return idx;
				}
				++idx;
			}

			return INVALID_WINDOW_INDEX;
		}

		buffer_t get_window_buffer( window_t idx )
		{
			XGC_ASSERT_RETURN( idx != INVALID_WINDOW_INDEX, INVALID_BUFFER_INDEX );
			autolock locker( _text_window[idx].critical );
			return _text_window[idx].text;
		}

		//-----------------------------------//
		// [12/24/2013 albert.xu]
		// 激活窗口
		//-----------------------------------//
		xgc_void active_window( window_t window )
		{
			autolock locker( _sort_window_critical );
			window_t cursor = window;

			for( xgc_size i = 0; i < XGC_COUNTOF( _sort_window ); ++i )
			{
				if( cursor == _sort_window[i] )
					break;

				std::swap( _sort_window[i], cursor );

				if( cursor == INVALID_WINDOW_INDEX )
					break;

				if( cursor == window )
					break;
			}
		}

		//-----------------------------------//
		// [12/24/2013 albert.xu]
		// 释放文本缓冲
		//-----------------------------------//
		xgc_void free_text( buffer_t text )
		{
			XGC_ASSERT_RETURN( text != INVALID_BUFFER_INDEX, void( 0 ) );
			autolock locker( _text_buffer[text].critical );

			free( _text_buffer[text].buffer );
			_text_buffer[text].w = 0;
			_text_buffer[text].h = 0;
			_text_buffer[text].position = 0;
			_text_buffer[text].buffer = xgc_nullptr;
		}

		xgc_void free_window( window_t window )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, void( 0 ) );
			text_window& wthis = _text_window[window];
			autolock locker( wthis.critical );

			wthis.text = INVALID_BUFFER_INDEX;
		}

		xgc_void draw_window( window_t window, xgc_bool border /*= true*/ )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, XGC_NONE );

			autolock locker( _text_window[window].critical );
			text_window& w = _text_window[window];

			if( w.redraw || border )
			{
				DWORD write = 0;

				for( int i = 1; w.border && i < w.cols; ++i )
				{
					COORD posTop = { i + w.x, w.y };

					WriteConsoleOutputAttribute( gstdio[1].cout, &w.border[text_top].Attributes, 1, posTop, &write );
					if( i % 10 == 0 )
						WriteConsoleOutputCharacterA( gstdio[1].cout, &w.border[text_number].Char.AsciiChar, 1, posTop, &write );
					else
						WriteConsoleOutputCharacterA( gstdio[1].cout, &w.border[text_top].Char.AsciiChar, 1, posTop, &write );

					COORD posBottom = { i + w.x, w.y + w.rows - 1 };

					WriteConsoleOutputAttribute( gstdio[1].cout, &w.border[text_bottom].Attributes, 1, posBottom, &write );
					WriteConsoleOutputCharacterA( gstdio[1].cout, &w.border[text_bottom].Char.AsciiChar, 1, posBottom, &write );
				}

				for( int i = 0; w.border && i < w.rows; ++i )
				{
					COORD posLeft = { w.x, i + w.y };

					WriteConsoleOutputAttribute( gstdio[1].cout, &w.border[text_left].Attributes, 1, posLeft, &write );
					WriteConsoleOutputCharacterA( gstdio[1].cout, &w.border[text_left].Char.AsciiChar, 1, posLeft, &write );

					COORD posRight = { w.x + w.cols - 1, i + w.y };

					WriteConsoleOutputAttribute( gstdio[1].cout, &w.border[text_right].Attributes, 1, posRight, &write );
					WriteConsoleOutputCharacterA( gstdio[1].cout, &w.border[text_right].Char.AsciiChar, 1, posRight, &write );
				}
			}

			XGC_ASSERT_RETURN( w.text != INVALID_BUFFER_INDEX, XGC_NONE );
			autolock buffer_locker( _text_buffer[w.text].critical );
			text_buffer &t = _text_buffer[w.text];

			if( w.redraw || t.flags & eTextDirty )
			{
				COORD BufferSize = { t.w, t.h };
				COORD BufferCoord = { w.sx, w.sy };
				SMALL_RECT WriteRect;
				if( w.border )
				{
					WriteRect.Left = w.x + 1;
					WriteRect.Top = w.y + 1;
					WriteRect.Right = w.x + w.cols - window_border_col_fix;
					WriteRect.Bottom = w.y + w.rows - window_border_row_fix;
				}
				else
				{
					WriteRect.Left = w.x;
					WriteRect.Top = w.y;
					WriteRect.Right = w.x + w.cols;
					WriteRect.Bottom = w.y + w.rows;
				}

				DWORD ret = WriteConsoleOutputA(
					gstdio[1].cout,
					t.buffer,
					BufferSize,
					BufferCoord,
					&WriteRect );

				t.flags = t.flags & ( ~eTextDirty );
			}

			w.redraw = false;
		}

		//-----------------------------------//
		// [12/25/2013 albert.xu]
		// 滚动窗口
		//-----------------------------------//
		xgc_void scroll_window( window_t window, xgc_int16 row, xgc_int16 col )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, XGC_NONE );

			autolock locker( _text_window[window].critical );
			text_window& w = _text_window[window];

			if( row < 0 && -row > w.sy )
				w.sy = 0;
			else if( row > 0 && w.sy + row > w.rows )
				w.sy = w.rows;
			else
				w.sy += row;

			if( col < 0 && -col > w.sx )
				w.sx = 0;
			else if( col > 0 && w.sx + col > w.cols )
				w.sx = w.cols;
			else
				w.sx += col;
		}

		static xgc_int32 output_buffer( buffer_t text, xgc_lpcstr str )
		{
			autolock locker( _text_buffer[text].critical );
			text_buffer& t = _text_buffer[text];

			t.flags |= eTextDirty;
			// format buffer
			xgc_lpcstr current = str;

			while( *current )
			{
				if( *current == '\n' )
				{
					while( t.position % t.w )
					{
						t.buffer[t.position].Attributes = g.blank;
						t.buffer[t.position].Char.AsciiChar = ' ';
						++t.position;
					}
				}
				else if( *current == '\t' )
				{
					while( t.position % t.w )
					{
						t.buffer[t.position].Attributes = g.blank;
						t.buffer[t.position].Char.AsciiChar = ' ';
						++t.position;

						if( t.position % t.w % 4 == 0 )
							break;
					}
				}
				else
				{
					t.buffer[t.position].Attributes = g.current;
					t.buffer[t.position].Char.AsciiChar = *current;
					++t.position;
				}

				if( t.position / t.w >= t.h )
				{
					scroll_buffer( t, t.position / t.w - t.h + 1 );
				}

				++current;
			}

			return (xgc_int32)( current - str );
		}

		xgc_int32 output_text( buffer_t text, xgc_lpcstr fmt, va_list ap )
		{
			char buffer[1024 * 4];
			char * pbuffer = buffer;

			int len = _vscprintf( fmt, ap );
			if( len >= XGC_COUNTOF( buffer ) )
				pbuffer = (char*)malloc( len + 1 );

			XGC_ASSERT_RETURN( pbuffer, 0 );

			vsprintf_s( pbuffer, len + 1, fmt, ap );

			len = output_buffer( text, pbuffer );

			if( pbuffer != buffer )
				free( pbuffer );

			return len;
		}

		xgc_int32 printf_text( buffer_t text, xgc_lpcstr fmt, ... )
		{
			va_list ap;
			va_start( ap, fmt );

			int copy_siz = output_text( text, fmt, ap );
			va_end( ap );

			return copy_siz;
		}
	}
}