#ifdef WINDOWS
#include <stdio.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <process.h>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "console.h"
#include "logger.h"
#include "geometry.h"

namespace xgc
{
	namespace console
	{
		using mutex = std::recursive_mutex;
		using guard = std::unique_lock< mutex >;
		using event = std::condition_variable_any;

		template<class _Ty>
		using atomic = std::atomic< _Ty >;

		using sRect	= xgc::xRect< xgc_int16 >;

		enum e_border
		{
			e_border_t,
			e_border_l,
			e_border_r,
			e_border_b,

			e_border_lt,
			e_border_rt,
			e_border_lb,
			e_border_rb,

			e_border_tick,
			e_border_count,
		};

		static xgc_word border_attr[e_border_count] =
		{
			COMMON_LVB_GRID_HORIZONTAL,
			COMMON_LVB_GRID_LVERTICAL,
			COMMON_LVB_GRID_RVERTICAL,
			COMMON_LVB_GRID_HORIZONTAL,

			COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_GRID_LVERTICAL,
			COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_GRID_RVERTICAL,
			COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_GRID_LVERTICAL,
			COMMON_LVB_GRID_HORIZONTAL | COMMON_LVB_GRID_RVERTICAL,

			FOREGROUND_INTENSITY | BACKGROUND_BLUE,
		};

		enum buffer_flags : xgc_uint16
		{
			eTextEdit  = 1,
			eTextDirty = 2,
			eTextFlagCount,
		};

		/// 文本缓冲区
		struct text_buffer
		{
			/// 缓冲区大小
			xgc_int16	w, h;
			/// 输入位置
			xgc_uint16	position;
			/// 颜色属性
			xgc_uint16  attr;
			/// 缓冲区标志
			xgc_uint16	flags;
			/// 缓冲区指针
			PCHAR_INFO	buffer;

			/// 互斥锁，用于线程同步
			mutex mtx;

			/// 输入缓冲区
			struct text_input_buffer
			{
				/// 互斥锁，用于线程同步
				mutex mtx;
				/// 输入数据的互斥锁
				event evt;
				/// 输入缓冲区
				xgc_lpstr input_buf;
				/// 输入缓冲区长度
				xgc_int16 input_len;
				/// 分配缓冲区长度
				xgc_int16 input_alc;
				/// 输入缓冲写指针
				xgc_int16 input_cur;
				/// 文本缓冲起始读位置
				xgc_int16 input_pos;
				/// 输入状态
				xgc_bool input_complate;
			} input;
		};

		struct text_window
		{
			/// 窗口位置
			xgc_int16 x, y;
			/// 窗口大小
			xgc_int16 cols, rows;
			/// 窗口滚动位置
			xgc_int16 sx, sy;
			/// 光标位置
			xgc_int16 cx, cy;
			/// 窗口标题
			xgc_lpcstr title;
			union
			{
				struct
				{
					/// 边框
					xgc_uint16 border : 1;
					/// 是否收起，只留标题栏
					xgc_uint16 expand : 1;
					/// 是否有菜单相
					xgc_uint16 menu : 1;
					/// 是否有状态栏
					xgc_uint16 title_bar : 1;
					/// 是否有状态栏
					xgc_uint16 status_bar : 1;
					/// 是否有滚动条
					xgc_uint16 scroll_bar : 1;
					/// 是否自动滚屏
					xgc_uint16 autoscroll : 1;
					/// 是否改变窗口尺寸
					xgc_uint16 resize : 1;
					/// 是否可以移动窗口
					xgc_uint16 moving : 1;
					/// 是否可以编辑
					xgc_uint16 editor : 1;
				};
				xgc_uint16 mask;
			}style;

			/// 当前窗口状态 0 - 正常， 1 - 移动窗口， 2 - 调整大小
			xgc_uint16 status;
			/// 窗口是否扩展
			xgc_bool expand;
			/// 滑动条
			struct
			{
				/// 滑动条 - 垂直长度
				xgc_int16 v_len;
				/// 滑动条 - 水平长度
				xgc_int16 h_len;
				/// 滑动条 - 垂直位置
				xgc_int16 v_pos;
				/// 滑动条 - 水平位置
				xgc_int16 h_pos;
			}scroll;

			/// 关联缓冲区
			buffer_t buffer;
			/// 缓冲窗口位置
			xgc_int16 tx, ty;
			/// 缓冲窗口大小
			xgc_int16 tw, th;
			/// 缓冲大小
			xgc_int16 bw, bh;
			/// 互斥锁，用于线程同步
			mutex	mtx;
		};

		/// 窗口对象数组
		static std::atomic< text_window* > _text_window[0xFF];
		/// 缓冲对象数组
		static std::atomic< text_buffer* > _text_buffer[0xFF];
		/// 窗口排序数组 - 第一个元素存储窗口个数
		static std::atomic< window_t > _sort_window[0xFF];
		/// 窗口计数
		static std::atomic< window_t > _text_window_count = 0;
		/// 文本双缓冲
		static xgc_handle	active_buffer[2];
		static xgc_lpvoid	active_buffer_exchange = xgc_nullptr;

		/// 重定向信息
		struct redirect
		{
			/// 重定向句柄
			int fd[2];
			/// 老的句柄
			int old;
			/// 重定向缓冲区
			buffer_t buffer;
			/// 显示的颜色
			xgc_word color;
		};

		/// 标准重定向
		static struct redirect gstdio[3] =
		{
			{ { -1, -1 }, -1, INVALID_BUFFER_INDEX },
			{ { -1, -1 }, -1, INVALID_BUFFER_INDEX },
			{ { -1, -1 }, -1, INVALID_BUFFER_INDEX },
		};

		/// 库初始化存储
		static std::atomic< xgc_bool > g_console_library = 0;
		static xgc_handle g_console_thread = 0;

		/// 鼠标位置
		static xgc_int16 mouse_x = 0;
		static xgc_int16 mouse_y = 0;

		/// 按钮状态
		static xgc_int16 mouse_s = 0;

		/// 当前的焦点窗口
		static window_t focus_window = INVALID_WINDOW_INDEX;

		/// 根据缓冲区光标位置，重新计算窗口偏移
		static xgc_void adjust_window( text_window* w, xgc_bool update )
		{
			text_buffer *t = _text_buffer[w->buffer];
			XGC_ASSERT_RETURN( t, XGC_NONE );
			guard lock_buffer( t->mtx );

			XGC_ASSERT_RETURN( t->buffer, XGC_NONE );

			// 根据缓冲中的光标位置计算窗口光标位置和滚轴位置
			if( update )
			{
				// 计算缓冲区中的光标位置
				xgc_int16 x = t->position % t->w;
				xgc_int16 y = t->position / t->w;

				// 计算滚轴的位置 - 光标应在右下位置
				w->sx = x > w->tw - 1 ? x - (w->tw - 1) : 0;
				w->sy = y > w->th - 1 ? y - (w->th - 1) : 0;

				// 计算光标位置
				w->cx = x - w->sx;
				w->cy = y - w->sy;
			}
			// 根据窗口光标位置计算缓冲光标位置。
			else
			{
				// 不能超出缓冲区最大值
				xgc_int16 y = w->cy + w->sy;
				xgc_int16 x = w->cx + w->sx;

				y = XGC_RNG( y, 0, t->h );
				x = XGC_RNG( x, 0, t->w );

				// 设置缓冲区光标位置
				t->position = ( w->cy + w->sy ) * t->w + ( w->cx + w->sx );
			}
		}

		static xgc_void adjust_cursor( text_window *w )
		{
			SetConsoleCursorPosition( active_buffer[0], { w->x + w->tx + w->cx, w->y + w->ty + w->cy } );
		}

		/// 重绘窗口
		static xgc_void redraw_window()
		{
			// 窗口重绘
			DWORD  nNum = 0;


			xgc_int16 cols = 0;
			xgc_int16 rows = 0;
			get_console_size( cols, rows );

			FillConsoleOutputAttribute( active_buffer[1], 0, cols * rows, COORD { 0, 0 }, &nNum );

			for( window_t i = 0; _sort_window[i] != INVALID_WINDOW_INDEX && i < XGC_COUNTOF( _sort_window ); ++i )
			{
				redraw_window( _sort_window[i], active_buffer[1], true );
			}

			SMALL_RECT rc = { 0, 0, cols-1, rows-1 };

			if( ReadConsoleOutput( active_buffer[1], (PCHAR_INFO)active_buffer_exchange, {cols, rows}, {0, 0}, &rc ) )
			{
				WriteConsoleOutput( active_buffer[0], (PCHAR_INFO)active_buffer_exchange, {cols, rows}, {0, 0}, &rc );
			}
		}

		static xgc_bool process_keyboard_event( KEY_EVENT_RECORD &Input )
		{
			//printf( "key event char=%c,%02x key=%s, \n"
			//		, Input.uChar.AsciiChar
			//		, Input.uChar.AsciiChar
			//		, Input.bKeyDown ? "down" : "up" );

			if( focus_window == INVALID_WINDOW_INDEX )
				return false;

			// 获取窗口对象
			text_window *w = _text_window[focus_window];
			XGC_ASSERT_RETURN( w, false );
			// 锁定窗口
			guard lock( w->mtx );

			if( w->style.editor && Input.bKeyDown )
			{
				// 是否控制字符
				if( !iscntrl( Input.uChar.AsciiChar ) )
				{
					// 可编辑的窗口
					printf_text( w->buffer, "%c", Input.uChar.AsciiChar );
				}
				else switch( Input.uChar.AsciiChar )
				{
				case '\r':
				case '\n':
					printf_text( w->buffer, "\n" );
					break;
				case '\b':
					printf_text( w->buffer, "\b" );
					break;
				}

				adjust_window( w, true );
				adjust_cursor( w );

				return true;
			}

			return false;
		}

		// 处理鼠标事件
		static xgc_bool process_mouse_event( MOUSE_EVENT_RECORD &Input )
		{
			//printf( "mouse event status=%04x flags=%04x pos_x=%d, pos_y=%d\n"
			//		, Input.dwButtonState
			//		, Input.dwEventFlags
			//		, Input.dwMousePosition.X
			//		, Input.dwMousePosition.Y );

			// 设置按钮状态
			mouse_s = Input.dwButtonState & 0xFFFF;

			xgc_bool bBusy = false;
			switch( Input.dwEventFlags )
			{
			case 0: // 按键消息
				if( mouse_s & FROM_LEFT_1ST_BUTTON_PRESSED )
				{
					window_t found = INVALID_WINDOW_INDEX;
					// 按下鼠标左键，则激活可寻得的窗口
					for( int i = 0; _sort_window[i] != INVALID_WINDOW_INDEX; ++i )
					{
						text_window *w = _text_window[_sort_window[i]];

						if( xgc_nullptr == w )
							continue;

						// 设置窗口状态，1 - 点击在标题栏， 2 - 点击在Sizer
						guard lock( w->mtx );

						if( Input.dwMousePosition.X >= w->x && Input.dwMousePosition.X < w->x + w->cols )
						{
							if( Input.dwMousePosition.Y < w->y || Input.dwMousePosition.Y >= w->y + ( w->expand ? w->rows : w->style.title_bar ) )
								continue;

							if( w->expand && Input.dwMousePosition.Y == w->y + w->rows - 1 )
								w->status = 2; // 点击在Sizer上
							else
								w->status = 1; // 其他位置，移动窗口

							found = _sort_window[i];
						}
					}

					// 找到合适的则激活该窗口
					active_window( found );
					hide_cursor();

					bBusy = true;
				}
				else if( focus_window != INVALID_WINDOW_INDEX )
				{
					text_window *w = _text_window[focus_window];
					if( w )
					{
						guard lock( w->mtx );

						// 重新设置光标位置
						adjust_cursor( w );

						// 没有移动窗口
						if( !(w->status & 0x8000) )
						{
							// 扩展状态
							if( w->expand )
							{
								if( w->cx >= w->tw || w->cy >= w->th )
									// 关闭光标显示
									hide_cursor();
								else
									// 打开光标显示
									show_cursor();
							}
							else
							{
								// 关闭光标显示
								hide_cursor();
							}
						}

						w->status = 0;
					}

					bBusy = true;
				}
				break;
			case MOUSE_MOVED:
				if( focus_window != INVALID_WINDOW_INDEX )
				{
					xgc_int16 offset_x = Input.dwMousePosition.X - mouse_x;
					xgc_int16 offset_y = Input.dwMousePosition.Y - mouse_y;

					if( offset_x || offset_y )
					{
						text_window *w = _text_window[focus_window];
						if( w )
						{
							guard window_lock( w->mtx );

							bBusy = true;

							switch( w->status & 0x7fff )
							{
							case 1: // 移动窗口位置
								if( w->style.moving )
								{
									w->x += offset_x;
									w->y += offset_y;

									// 设置移动标志
									w->status |= 0x8000;
								}
								break;
							case 2: // 改变窗口大小
								if( w->style.resize )
								{
									// 计算窗口区域
									w->cols = w->cols + offset_x;
									w->rows = w->rows + offset_y;

									w->cols = XGC_RNG( w->cols, 8, w->bw + w->style.border * 2 );
									w->rows = XGC_RNG( w->rows, 8, w->bh + w->style.title_bar + w->style.status_bar + w->style.menu );

									// 计算文本区域
									w->tw = w->cols - w->style.border * 2;
									w->th = w->rows - w->style.title_bar - w->style.menu - w->style.status_bar;

									w->tw = XGC_RNG( w->tw, 1, w->bw );
									w->th = XGC_RNG( w->th, 1, w->bh );

									// 设置移动标志
									w->status |= 0x8000;
								}
								break;
							default:
								bBusy = false;
								break;
							}
						}
					}
				}
				mouse_x = Input.dwMousePosition.X;
				mouse_y = Input.dwMousePosition.Y;
				break;
			case DOUBLE_CLICK:
				if( mouse_s & FROM_LEFT_1ST_BUTTON_PRESSED )
				{
					window_t found = INVALID_WINDOW_INDEX;
					for( int i = 0; _sort_window[i] != INVALID_WINDOW_INDEX; ++i )
					{
						text_window *w = _text_window[_sort_window[i]];
						if( xgc_nullptr == w )
							continue;

						guard lock( w->mtx );
						if( Input.dwMousePosition.X >= w->x &&
							Input.dwMousePosition.X < w->x + w->cols &&
							Input.dwMousePosition.Y == w->y )
						{
							found = _sort_window[i];
						}
					}

					if( found != INVALID_WINDOW_INDEX )
					{
						text_window *w = _text_window[found];
						if( w )
						{
							guard lock( w->mtx );
							if( w->style.expand )
							{
								w->expand = !w->expand;
								bBusy = true;
							}
						}
					}
				}
				break;
			case MOUSE_WHEELED:
				break;
			}

			return true;
		}

		static xgc_bool process_event()
		{
			INPUT_RECORD Input[64] = { 0 };

			DWORD nInput = 0;

			// dispatch command
			HANDLE hStdInput = GetStdHandle( STD_INPUT_HANDLE );

			xgc_bool bBusy = false;
			if( FALSE == GetNumberOfConsoleInputEvents( hStdInput, &nInput ) || 0 == nInput )
				return false;

			if( ReadConsoleInput( hStdInput, Input, XGC_MIN( nInput, XGC_COUNTOF( Input ) ), &nInput ) )
			{
				for( auto i = 0U; i < nInput && i < XGC_COUNTOF( Input ); ++i )
				{
					switch( Input[i].EventType )
					{
					case KEY_EVENT:
						bBusy = bBusy || process_keyboard_event( Input[i].Event.KeyEvent );
						break;
					case MOUSE_EVENT:
						bBusy = bBusy || process_mouse_event( Input[i].Event.MouseEvent );
						break;
					case WINDOW_BUFFER_SIZE_EVENT:
						break;
					case MENU_EVENT:
						break;
					case FOCUS_EVENT:
						break;
					}
				}
			}

			if( bBusy )
			{
				// 重绘窗口
				redraw_window();
			}

			return bBusy;
		}

		/// 控制台更新线程
		static unsigned int _stdcall console_update_thread( xgc_lpvoid _param )
		{
			char buf[4096];
			g_console_library = 1UL;
			clock_t tick = clock();
			while( g_console_library )
			{
				// 处理事件消息
				process_event();

				// 处理重定向
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

				Sleep( 1 );
			}

			return 0;
		}

		///
		/// \brief 初始化控制台
		///
		/// \param cols 窗口宽
		/// \param rows 窗口高
		///
		/// [12/24/2013 albert.xu]
		///

		xgc_bool console_init( xgc_int16 cols, xgc_int16 rows, PHANDLER_ROUTINE pfnHandler )
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

			memset( _sort_window, INVALID_WINDOW_INDEX, sizeof( _sort_window ) );

			COORD dwConsoleBufferSize = { cols, rows };
			SMALL_RECT rcWindow = { 0, 0, cols - 1, rows - 1 };

			BOOL bRet = FALSE;
			// 创建双缓冲
			active_buffer[0] = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL );
			XGC_ASSERT_RETURN( active_buffer[0] != INVALID_HANDLE_VALUE, false );
			active_buffer[1] = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL );
			XGC_ASSERT_RETURN( active_buffer[1] != INVALID_HANDLE_VALUE, false );

			bRet = SetConsoleScreenBufferSize( active_buffer[0], dwConsoleBufferSize );
			XGC_ASSERT_RETURN( bRet, false );
			bRet = SetConsoleScreenBufferSize( active_buffer[1], dwConsoleBufferSize );
			XGC_ASSERT_RETURN( bRet, false );

			bRet = SetConsoleWindowInfo( active_buffer[0], true, &rcWindow );
			XGC_ASSERT_RETURN( bRet, false );
			bRet = SetConsoleWindowInfo( active_buffer[1], true, &rcWindow );
			XGC_ASSERT_RETURN( bRet, false );

			bRet = SetConsoleActiveScreenBuffer( active_buffer[0] );
			XGC_ASSERT_RETURN( bRet, false );

			SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT );

			show_cursor();
			active_buffer_exchange = malloc( cols * rows * sizeof(CHAR_INFO) );
			g_console_thread = (xgc_handle)_beginthreadex( xgc_nullptr, 0, console_update_thread, xgc_nullptr, 0, xgc_nullptr );
			return true;
		}

		xgc_void console_fini()
		{
			// 退出
			g_console_library = 0UL;

			for( xgc_size i = 0; i < XGC_COUNTOF( gstdio ); ++i )
			{
				if( -1 != gstdio[i].fd[1] )
					_write( gstdio[i].fd[1], "", 1 );
			}

			WaitForSingleObject( (HANDLE) g_console_thread, INFINITE );

			for( window_t i = 0; i < XGC_COUNTOF( _text_window ); ++i )
			{
				free_window( i );
			}

			for( buffer_t i = 0; i < XGC_COUNTOF( _text_buffer ); ++i )
			{
				free_buffer( i );
			}

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

			SetConsoleActiveScreenBuffer( GetStdHandle( STD_OUTPUT_HANDLE ) );
			CloseHandle( active_buffer[0] );
			CloseHandle( active_buffer[1] );

			free( active_buffer_exchange );
		}

		///
		/// 屏幕是否被初始化了.
		/// [6/4/2015] create by albert.xu
		///
		xgc_bool console_is_init()
		{
			return g_console_library != 0;
		}

		/// 重定向标准输出
		xgc_bool redirect( FILE* file, buffer_t buffer )
		{
			int stdfd = _fileno( file );
			XGC_ASSERT_RETURN( stdfd >= 0 && stdfd < 3, false );

			int fd = _dup( stdfd );
			if( -1 == fd )
				return false;

			gstdio[stdfd].old = fd;

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

		/// 获取控制台窗口宽度
		xgc_bool get_console_size( xgc_int16 &cx, xgc_int16 &cy )
		{
			CONSOLE_SCREEN_BUFFER_INFOEX ConsoleInfo;
			ZeroMemory( &ConsoleInfo, sizeof( ConsoleInfo ) );
			ConsoleInfo.cbSize = sizeof( ConsoleInfo );

			BOOL bRet = GetConsoleScreenBufferInfoEx( active_buffer[0], &ConsoleInfo );

			cx = ConsoleInfo.dwSize.X;
			cy = ConsoleInfo.dwSize.Y;

			return bRet == TRUE;
		}

		// 隐藏光标
		xgc_void show_cursor( xgc_real32 percent )
		{
			CONSOLE_CURSOR_INFO CursorInfo;
			if( GetConsoleCursorInfo( active_buffer[0], &CursorInfo ) )
			{
				CursorInfo.bVisible = true;
				CursorInfo.dwSize	= (DWORD)(100.0f * percent);

				SetConsoleCursorInfo( active_buffer[0], &CursorInfo );
			}
		}

		xgc_void hide_cursor()
		{
			CONSOLE_CURSOR_INFO CursorInfo;
			if( GetConsoleCursorInfo( active_buffer[0], &CursorInfo ) )
			{
				CursorInfo.bVisible = false;
				SetConsoleCursorInfo( active_buffer[0], &CursorInfo );
			}
		}

		xgc_void gotoxy( xgc_int16 col, xgc_int16 row, window_t window )
		{
			if( window != INVALID_WINDOW_INDEX )
			{
				text_window *w = _text_window[window];
				XGC_ASSERT_RETURN( w, XGC_NONE );

				guard lock_window( w->mtx );
				w->cx = col;
				w->cy = row;

				// 根据光标位置对齐窗口
				adjust_window( w, false );

				if( window == focus_window )
					adjust_cursor( w );
			};
		}

		buffer_t buffer( xgc_uint16 w, xgc_uint16 h )
		{
			buffer_t idx = 0;
			while( idx < XGC_COUNTOF( _text_buffer ) )
			{
				if( _text_buffer[idx].load() == xgc_nullptr )
				{
					text_buffer* t = _text_buffer[idx] = XGC_NEW text_buffer;
					guard lock( t->mtx );

					t->buffer = (PCHAR_INFO) malloc( sizeof(CHAR_INFO) * w * h );
					if( t->buffer == xgc_nullptr )
						return -1;

					t->w        = w;
					t->h        = h;
					t->attr		= 0x0007;
					t->position = 0;
					t->flags    = 0;

					t->input.input_buf = xgc_nullptr;
					t->input.input_len = 0;
					t->input.input_alc = 0;
					t->input.input_cur = 0;
					t->input.input_pos = 0;
					t->input.input_complate = false;

					for( int i = 0; i < w * h; ++i )
					{
						t->buffer[i].Attributes     = t->attr;
						t->buffer[i].Char.AsciiChar = ' ';
					}

					return idx;
				}
				++idx;
			}
			return INVALID_BUFFER_INDEX;
		}

		window_t window( xgc_int16 x, xgc_int16 y, xgc_int16 cols, xgc_int16 rows, buffer_t &t, xgc_uint16 style, xgc_lpcstr title )
		{
			window_t idx = 0;

			// 如果是一个无效的缓冲句柄，则创建一个。
			if( t == INVALID_BUFFER_INDEX )
			{
				xgc_int16 cx = cols;
				xgc_int16 cy = rows;

				if( XGC_CHK_FLAGS( style, WINDOW_STYLE_BORDER ) )
					cx -= 2;

				if( XGC_CHK_FLAGS( style, WINDOW_STYLE_TITLE_BAR ) )
					cy -= 1;

				if( XGC_CHK_FLAGS( style, WINDOW_STYLE_MENU ) )
					cy -= 1;

				if( XGC_CHK_FLAGS( style, WINDOW_STYLE_STATUS_BAR ) )
					cy -= 1;

				XGC_ASSERT_RETURN( cx > 0, INVALID_WINDOW_INDEX );
				XGC_ASSERT_RETURN( cy > 0, INVALID_WINDOW_INDEX );

				t = buffer( cx, cy );
				XGC_ASSERT_RETURN( t != INVALID_BUFFER_INDEX, INVALID_WINDOW_INDEX );
			}

			// 查找一个空的位置
			while( idx < XGC_COUNTOF( _text_window ) )
			{
				if( _text_window[idx].load() == xgc_nullptr )
				{
					text_window *w = _text_window[idx] = XGC_NEW text_window;
					guard lock_window( w->mtx );

					text_buffer *b = _text_buffer[t];
					guard lock_buffer( b->mtx );

					// 初始化窗口位置
					w->x = x;
					w->y = y;

					// 初始化窗口大小
					w->cols = cols;
					w->rows = rows;

					// 初始化滚轴位置
					w->sx = 0;
					w->sy = 0;

					// 初始化光标位置
					w->cx = 0;
					w->cy = 0;

					w->title = _strdup( title );
					// 窗口风格
					w->style.mask = style;
					// 当前窗口状态 0 - 正常， 1 - 移动窗口， 2 - 调整大小
					w->status = 0;
					// 窗口是否扩展
					w->expand = true;

					// 滑动条 - 垂直长度
					w->scroll.v_len = 0;
					// 滑动条 - 水平长度
					w->scroll.h_len = 0;
					// 滑动条 - 垂直位置
					w->scroll.v_pos = 0;
					// 滑动条 - 水平位置
					w->scroll.h_pos = 0;

					w->buffer = t;

					// 计算窗口行列数
					w->tw = w->cols - w->style.border * 2;
					w->th = w->rows - w->style.title_bar - w->style.menu - w->style.status_bar;

					// 设置缓冲大小
					w->bw = b->w;
					w->bh = b->h;

					// 计算窗口位置
					w->tx = w->style.border;
					w->ty = w->style.title_bar + w->style.menu;
					// 将新建的窗口加入到窗口列表中
					active_window( idx );

					++_text_window_count;
					return idx;
				}
				++idx;
			}

			return INVALID_WINDOW_INDEX;
		}

		buffer_t window_buffer( window_t idx )
		{
			XGC_ASSERT_RETURN( idx != INVALID_WINDOW_INDEX, INVALID_BUFFER_INDEX );
			text_window *w = _text_window[idx];
			XGC_ASSERT_RETURN( w, INVALID_BUFFER_INDEX );
			guard lock( w->mtx );
			return w->buffer;
		}

		xgc_void window_style( window_t window, xgc_uint16 style )
		{
			text_window *w = _text_window[window];
			XGC_ASSERT_RETURN( w, XGC_NONE );
			guard lock( w->mtx );
			w->style.mask = style;
		}

		xgc_void active_window( window_t window )
		{
			if( window != INVALID_WINDOW_INDEX )
			{
				xgc_bool found = false;

				// 窗口激活算法， 窗口在_sort_window数组中最后一个会显示在最上层
				for( int i = 0; i < XGC_COUNTOF( _sort_window ); ++i )
				{
					if( found )
					{
						if( _sort_window[i] == INVALID_WINDOW_INDEX )
							break;

						// 交替往复，将窗口移到最后一个元素，注意，这里需要使用exchange，用于确保多线环境下数据安全。
						_sort_window[i - 1] = _sort_window[i].exchange( _sort_window[i - 1] );
					}
					else
					{
						// 仅查找
						if( _sort_window[i] == INVALID_WINDOW_INDEX )
							_sort_window[i] = window;

						if( _sort_window[i] == window )
							found = true;
					}
				}
			}

			focus_window = window;
		}

		///
		/// \brief 是否活动窗口
		///
		/// \author albert.xu
		/// \date 2016/01/05 16:43
		///

		COMMON_API xgc_bool is_active_window( window_t window )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, false );

			if( _text_window_count == 0 )
				return false;

			if( _text_window_count > XGC_COUNTOF( _sort_window ) )
				return false;

			if( window != _sort_window[_text_window_count - 1] )
				return false;

			return true;
		}

		///
		/// \brief 释放文本缓冲
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///

		xgc_void free_buffer( buffer_t text )
		{
			XGC_ASSERT_RETURN( text != INVALID_BUFFER_INDEX, XGC_NONE );
			text_buffer *t = _text_buffer[text].exchange( xgc_nullptr );
			if( xgc_nullptr == t )
				return;

			guard lock( t->mtx );
			free( t->buffer );
			t->w = 0;
			t->h = 0;
			t->position = 0;
			t->buffer = xgc_nullptr;

			if( t->input.input_alc )
				free( t->input.input_buf );

			t->input.input_buf = xgc_nullptr;
			t->input.input_len = 0;
			t->input.input_alc = 0;
			t->input.input_cur = 0;
			t->input.input_pos = 0;
			t->input.input_complate = false;

			lock.unlock();
			SAFE_DELETE( t );
		}

		xgc_void free_window( window_t window )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, XGC_NONE );
			text_window *w = _text_window[window].exchange( xgc_nullptr );
			if( xgc_nullptr == w )
				return;

			--_text_window_count;
			guard lock( w->mtx );

			w->x  = 0;
			w->y  = 0;
			w->cx = 0;
			w->cy = 0;
			w->sx = 0;
			w->sy = 0;

			free( (xgc_lpvoid)w->title );
			w->title  = xgc_nullptr;
			w->buffer = INVALID_BUFFER_INDEX;
			lock.unlock();
			SAFE_DELETE( w );
		}

		/// 重绘指定窗口
		xgc_void redraw_window( window_t window, xgc_handle output, xgc_bool border )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, XGC_NONE );
			XGC_ASSERT_RETURN( _text_window_count, XGC_NONE );

			// 取当前需要绘制的窗口
			text_window *w = _text_window[window];
			if( xgc_nullptr == w )
				return;

			guard lock_window( w->mtx );

			DWORD dwNum = 0;

			if( w->expand )
			{
				XGC_ASSERT_RETURN( w->buffer != INVALID_BUFFER_INDEX, XGC_NONE );
				text_buffer *t = _text_buffer[w->buffer];
				XGC_ASSERT_RETURN( t, XGC_NONE );
				guard lock_buffer( t->mtx );

				if( t->flags & eTextDirty )
				{
					if( w->style.autoscroll )
					{
						adjust_window( w, true );
						if( window == focus_window )
							adjust_cursor( w );
					}

					t->flags &= ~eTextDirty;
				}

				COORD BufferSize  = { t->w,  t->h  };
				COORD BufferCoord = { w->sx, w->sy };

				SMALL_RECT rc =
				{
					w->x + w->tx,
					w->y + w->ty,
					w->x + w->tx + w->tw - 1,
					w->y + w->ty + w->th - 1,
				};

				DWORD ret = WriteConsoleOutputA(
					output,
					t->buffer,
					BufferSize,
					BufferCoord,
					&rc );
			}

			if( false == w->style.border || false == border )
				return;

			// draw border
			xgc_uint16 COLOR_TITLE_BAR  = BACKGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | ( focus_window == window ? FOREGROUND_INTENSITY : 0 );
			xgc_uint16 COLOR_STATUS_BAR = BACKGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | ( focus_window == window ? FOREGROUND_INTENSITY : 0 );
			xgc_uint16 COLOR_SCROLL_BAR = w->style.scroll_bar ? (BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE) : FOREGROUND_RED;

			// 计算绘制的区域
			xgc_int16 cols, rows;
			get_console_size( cols, rows );

			xgc_int16 x0 = XGC_MAX( w->x, 0 );
			xgc_int16 x1 = XGC_MIN( w->x + w->cols, cols );

			// 当前绘制的行
			xgc_int16 y0 = w->y;
			// 最后绘制的行
			xgc_int16 y1 = w->y + w->rows - w->style.status_bar;

			// draw title border
			if( w->style.title_bar && y0 >= 0 && y0 < rows )
			{
				FillConsoleOutputAttribute( output, border_attr[e_border_t] | COLOR_TITLE_BAR, x1 - x0, COORD { x0, y0 }, &dwNum );
				FillConsoleOutputCharacter( output, ' ', x1 - x0, COORD { x0, y0 }, &dwNum );

				if( w->x >= 0 && w->x < cols )
					FillConsoleOutputAttribute( output, border_attr[e_border_lt] | COLOR_TITLE_BAR, 1, COORD { w->x, y0 }, &dwNum );
				if( w->x + w->cols - 1 >= 0 && w->x + w->cols - 1 < cols )
					FillConsoleOutputAttribute( output, border_attr[e_border_rt] | COLOR_TITLE_BAR, 1, COORD { w->x + w->cols - 1, y0 }, &dwNum );

				// draw title
				xgc_int16 title_len = (xgc_int16) strlen( w->title );
				xgc_int16 title_bar = w->cols - 4;

				xgc_int16 title_pos = title_bar - (title_bar + title_len) / 2;
				if( title_pos > 1 )
				{
					WriteConsoleOutputCharacter( output, w->title, title_len, { w->x + 2 + title_pos, w->y }, &dwNum );
				}
				else
				{
					WriteConsoleOutputCharacter( output, "...", 3, { w->x + 2, w->y }, &dwNum );
					WriteConsoleOutputCharacter( output
												 , w->title + title_pos
												 , title_bar - 3 - w->style.border * 2
												 , { w->x + 1 + 3 + w->style.border * 2, w->y + w->style.border - 1 }, &dwNum );
				}

				++y0;
			}

			// draw expand
			if( w->expand )
			{
				// set grid attribute on first line
				if( y0 >= 0 && y0 < rows )
				{
					SMALL_RECT rc = { x0, y0, x1, y0 };
					PCHAR_INFO lp = (PCHAR_INFO)active_buffer_exchange;
					if( ReadConsoleOutput( output, lp, { x1 - x0, 1 }, { 0, 0 }, &rc ) )
					{
						for( int i = 0; i < x1 - x0; ++i )
							lp[i].Attributes |= border_attr[e_border_t];

						WriteConsoleOutput( output, lp, { x1 - x0, 1 }, { 0, 0 }, &rc );
					}

					if( w->x >= 0 && w->x < cols )
						FillConsoleOutputAttribute( output, border_attr[e_border_lt], 1, { w->x, y0 }, &dwNum );

					if( w->x + w->cols - 1 >= 0 && w->x + w->cols - 1 < cols )
					{
						if( w->style.scroll_bar )
						{
							FillConsoleOutputAttribute( output, border_attr[e_border_rt] | COLOR_SCROLL_BAR, 1, { w->x + w->cols - 1, y0 }, &dwNum );
							FillConsoleOutputCharacter( output, '\x1e', 1, { w->x + w->cols - 1, y0 }, &dwNum );
						}
						else
						{
							FillConsoleOutputAttribute( output, border_attr[e_border_rt], 1, { w->x + w->cols - 1, y0 }, &dwNum );
							FillConsoleOutputCharacter( output, '\x20', 1, { w->x + w->cols - 1, y0 }, &dwNum );
						}
					}
					++y0;
				}

				if( w->style.border )
				{
					// draw left and right
					while( y0 < y1 )
					{
						// draw left
						if( w->x >= 0 && w->x < cols )
						{
							FillConsoleOutputAttribute( output, border_attr[e_border_l], 1, { w->x, y0 }, &dwNum );
							FillConsoleOutputCharacter( output, ' ', 1, { w->x, y0 }, &dwNum );
						}

						// draw right
						if( w->x + w->cols - 1 >= 0 && w->x + w->cols - 1 < cols )
						{
							if( w->style.scroll_bar )
							{
								FillConsoleOutputAttribute( output, border_attr[e_border_r] | COLOR_SCROLL_BAR, 1, { w->x + w->cols - 1, y0 }, &dwNum );
								FillConsoleOutputCharacter( output, ' ', 1, { w->x + w->cols - 1, y0 }, &dwNum );
							}
							else
							{
								FillConsoleOutputAttribute( output, border_attr[e_border_r], 1, { w->x + w->cols - 1, y0 }, &dwNum );
								FillConsoleOutputCharacter( output, ' ', 1, { w->x + w->cols - 1, y0 }, &dwNum );
							}
						}

						if( w->style.scroll_bar && y0 == y1 - w->style.status_bar )
							FillConsoleOutputCharacter( output, '\x1f', 1, { w->x + w->cols - 1, y0 }, &dwNum );

						++y0;
					}
				}
				else
				{
					y0 = y1;
				}

				// draw status bar
				if( w->style.status_bar && y0 >= 0 && y0 < rows )
				{
					// draw bar
					FillConsoleOutputAttribute( output, border_attr[e_border_b] | COLOR_TITLE_BAR, x1 - x0, COORD { x0, y0 }, &dwNum );
					FillConsoleOutputCharacter( output, ' ', x1 - x0, COORD { x0, y0 }, &dwNum );

					// draw left bottom corner
					if( w->x >= 0 && w->x < cols )
						FillConsoleOutputAttribute( output, border_attr[e_border_lb] | COLOR_TITLE_BAR, 1, COORD { w->x, y0 }, &dwNum );

					// draw right bottom corner
					if( w->x + w->cols - 1 >= 0 && w->x + w->cols - 1 < cols )
						FillConsoleOutputAttribute( output, border_attr[e_border_rb] | COLOR_TITLE_BAR, 1, COORD { w->x + w->cols - 1, y0 }, &dwNum );

					if( w->style.scroll_bar )
					{
						// draw left horizontal scroll button
						if( w->x >= 0 && w->x < cols )
							FillConsoleOutputCharacter( output, '\x11', 1, COORD { w->x, y0 }, &dwNum );

						// draw right horizontal scroll button
						if( w->x + w->cols - 2 >= 0 && w->x + w->cols - 2 < cols )
							FillConsoleOutputCharacter( output, '\x10', 1, COORD { w->x + w->cols - 2, y0 }, &dwNum );
					}

					++y0;
				}
			}

			// set last line grid attribute
			if( y0 >= 0 && y0 < rows )
			{
				SMALL_RECT rc = { x0, y0, x1, y0 };
				PCHAR_INFO lp = (PCHAR_INFO)active_buffer_exchange;
				if( ReadConsoleOutput( output, lp, { x1 - x0, 1 }, { 0, 0 }, &rc ) )
				{
					for( int i = 0; i < x1 - x0; ++i )
						lp[i].Attributes |= border_attr[e_border_t];

					WriteConsoleOutput( output, lp, { x1 - x0, 1 }, { 0, 0 }, &rc );
				}
			}
		}

		/// 滚动窗口
		xgc_void scroll_window( window_t window, xgc_int16 row, xgc_int16 col )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, XGC_NONE );

			text_window *w = _text_window[window];
			XGC_ASSERT_RETURN( w, XGC_NONE );
			guard lock( w->mtx );

			if( row < 0 && -row > w->sy )
				w->sy = 0;
			else if( row > 0 && w->sy + row > w->rows )
				w->sy = w->rows;
			else
				w->sy += row;

			if( col < 0 && -col > w->sx )
				w->sx = 0;
			else if( col > 0 && w->sx + col > w->cols )
				w->sx = w->cols;
			else
				w->sx += col;

			redraw_window( window, active_buffer[0], true );
		}

		/// 向上滚动窗口row行
		/// 可进行环形缓冲的优化
		static xgc_void scroll_buffer( text_buffer* t, xgc_uint16 row )
		{
			if( row == 0 )
				return;

			guard lock( t->mtx );

			row = XGC_MIN( row, t->h );
			memmove( t->buffer, t->buffer + ( row * t->w ), ( t->h - row ) * t->w * sizeof( CHAR_INFO ) );
			CHAR_INFO chBlank;
			chBlank.Attributes = 0;
			chBlank.Char.AsciiChar = 0;

			for( int i = ( t->h - row ) * t->w; i < t->w * t->h; ++i )
				t->buffer[i] = chBlank;

			if( t->position > row * t->w )
				t->position -= row * t->w;
			else
				t->position = 0;

			t->flags |= eTextDirty;
		}

		static xgc_int32 output_buffer( text_buffer* t, xgc_lpcstr str )
		{
			// 设置脏标志
			t->flags |= eTextDirty;

			xgc_lpcstr current  = str;
			xgc_uint16 setcolor = 0;
			xgc_uint16 getcolor = t->attr;

			guard lock( t->input.mtx, std::try_to_lock );

			// 处理输入串
			while( *current )
			{
				// 设置显示颜色
				if( setcolor )
				{
					getcolor = ( getcolor & ( 0xf0 >> (setcolor-1)*4 ) ) | ( (*current % 0x10) << (setcolor-1)*4 );
					setcolor = 0;
				}
				else switch( *current )
				{
				case '\n': // 换行符
					t->buffer[t->position].Attributes = 0;
					t->buffer[t->position].Char.AsciiChar = '\n';
					++t->position;

					while( t->position % t->w )
					{
						t->buffer[t->position].Attributes = 0;
						t->buffer[t->position].Char.AsciiChar = 0;
						++t->position;
					}

					// 处理输入
					if( false == lock.owns_lock() )
					{
						if( t->input.input_cur < t->input.input_len )
							t->input.input_buf[t->input.input_cur++] = '\n';

						t->input.input_pos = 0;
						t->input.input_complate = true;
						// 通知完成输入
						t->input.evt.notify_one();
					}
					break;
				case '\b':
					// 处理输入
					if( false == lock.owns_lock() )
					{
						if( t->input.input_cur > 0 )
							t->input.input_buf[--t->input.input_cur] = 0;

						// 回写输入缓冲
						if( t->position == 0 && t->input.input_cur != 0 )
						{
							xgc_int16 len = XGC_MIN( t->w, t->input.input_cur );
							for( xgc_int16 i = 0; i < len; ++i, ++t->position )
							{
								t->buffer[i].Attributes		= t->attr;
								t->buffer[i].Char.AsciiChar = t->input.input_buf[i];
							}

							// 后面会删除一个字符，这里不需要了。
							break;
						}
					}

					// 清除占位符
					while( t->position > t->input.input_pos )
					{
						--t->position;

						if( t->buffer[t->position].Char.AsciiChar != 0 )
							break;
					}

					// 清除一个字符
					if( t->position >= t->input.input_pos )
					{
						t->buffer[t->position].Attributes = 0;
						t->buffer[t->position].Char.AsciiChar = 0;
					}
					break;
				case '\t': // 制表符
					if( lock.owns_lock() )
					{
						t->buffer[t->position].Attributes = getcolor;
						t->buffer[t->position].Char.AsciiChar = ' ';
						++t->position;

						while( t->position % t->w )
						{
							t->buffer[t->position].Attributes = getcolor;
							t->buffer[t->position].Char.AsciiChar = 0;
							++t->position;

							if( t->position % t->w % 4 == 0 )
								break;
						}
					}
					break;
				case '\x1': // 前景色
					setcolor = 1;
					break;
				case '\x2': // 背景色
					setcolor = 2;
					break;
				default: // 正常字符
					if( false == lock.owns_lock() )
					{
						// 不能超出缓冲大小
						if( t->input.input_cur >= t->input.input_len )
							break;

						t->input.input_buf[t->input.input_cur++] = *current;
					}

					t->buffer[t->position].Attributes     = getcolor;
					t->buffer[t->position].Char.AsciiChar = *current;
					++t->position;

					break;
				}

				// 判定是否需要滚屏
				if( t->position / t->w >= t->h )
				{
					scroll_buffer( t, t->position / t->w - t->h + 1 );
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

			text_buffer *t = _text_buffer[text];
			XGC_ASSERT_RETURN( t, -1 );
			guard lock_buffer( t->mtx );

			len = output_buffer( t, pbuffer );

			if( pbuffer != buffer )
				free( pbuffer );

			return len;
		}

		/// 缓冲输入对象，每线程一个
		xgc_int32 buffer_read( buffer_t text, xgc_lpstr buf, xgc_int16 len, xgc_bool wait )
		{
			memset( buf, 0, len );

			text_buffer *t = _text_buffer[text];
			XGC_ASSERT_RETURN( t, -1 );

			// 锁输入，不允许同时对一个缓冲做输入操作
			guard lock_input( t->input.mtx );

			// 锁缓冲
			guard lock_event( t->mtx );

			if( wait )
			{
				// 释放私有内存
				if( t->input.input_alc )
					free( t->input.input_buf );

				// 初始化输入参数
				t->input.input_buf = buf;
				t->input.input_len = len;
				t->input.input_alc = 0;
				t->input.input_cur = 0;
				t->input.input_pos = t->position;
				t->input.input_complate = false;

				// 等待输入完成
				t->input.evt.wait( lock_event );

				// 返回输入的长度
				return t->input.input_cur;
			}
			else
			{
				if( t->input.input_buf )
				{
					if( t->input.input_complate )
					{
						errno_t err = memcpy_s( buf, len, t->input.input_buf, t->input.input_cur );

						xgc_int16 ret = t->input.input_cur;
						if( t->input.input_alc )
							free( t->input.input_buf );

						t->input.input_buf = xgc_nullptr;
						t->input.input_alc = 0;
						t->input.input_complate = 0;
						t->input.input_cur = 0;
						t->input.input_pos = 0;

						t->input.mtx.unlock();

						if( err == 0 )
							return ret;
						else
							return -1;
					}
				}
				else
				{
					t->input.mtx.lock();

					// 初始化输入缓冲
					if( t->input.input_alc == 0 )
					{
						t->input.input_buf = (xgc_lpstr) malloc( len );
						t->input.input_alc = len;
					}
					else if( t->input.input_alc < len )
					{
						t->input.input_buf = (xgc_lpstr) realloc( t->input.input_buf, len );
						t->input.input_alc = len;
					}

					t->input.input_len = len;
					t->input.input_cur = 0;
					t->input.input_pos = t->position;
					t->input.input_complate = false;

					return 0;
				}
			}

			return -1;
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
#endif // WINDOWS