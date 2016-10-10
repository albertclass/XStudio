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

		/// �ı�������
		struct text_buffer
		{
			/// ��������С
			xgc_int16	w, h;
			/// ����λ��
			xgc_uint16	position;
			/// ��ɫ����
			xgc_uint16  attr;
			/// ��������־
			xgc_uint16	flags;
			/// ������ָ��
			PCHAR_INFO	buffer;

			/// �������������߳�ͬ��
			mutex mtx;

			/// ���뻺����
			struct text_input_buffer
			{
				/// �������������߳�ͬ��
				mutex mtx;
				/// �������ݵĻ�����
				event evt;
				/// ���뻺����
				xgc_lpstr input_buf;
				/// ���뻺��������
				xgc_int16 input_len;
				/// ���仺��������
				xgc_int16 input_alc;
				/// ���뻺��дָ��
				xgc_int16 input_cur;
				/// �ı�������ʼ��λ��
				xgc_int16 input_pos;
				/// ����״̬
				xgc_bool input_complate;
			} input;
		};

		struct text_window
		{
			/// ����λ��
			xgc_int16 x, y;
			/// ���ڴ�С
			xgc_int16 cols, rows;
			/// ���ڹ���λ��
			xgc_int16 sx, sy;
			/// ���λ��
			xgc_int16 cx, cy;
			/// ���ڱ���
			xgc_lpcstr title;
			union
			{
				struct
				{
					/// �߿�
					xgc_uint16 border : 1;
					/// �Ƿ�����ֻ��������
					xgc_uint16 expand : 1;
					/// �Ƿ��в˵���
					xgc_uint16 menu : 1;
					/// �Ƿ���״̬��
					xgc_uint16 title_bar : 1;
					/// �Ƿ���״̬��
					xgc_uint16 status_bar : 1;
					/// �Ƿ��й�����
					xgc_uint16 scroll_bar : 1;
					/// �Ƿ��Զ�����
					xgc_uint16 autoscroll : 1;
					/// �Ƿ�ı䴰�ڳߴ�
					xgc_uint16 resize : 1;
					/// �Ƿ�����ƶ�����
					xgc_uint16 moving : 1;
					/// �Ƿ���Ա༭
					xgc_uint16 editor : 1;
				};
				xgc_uint16 mask;
			}style;

			/// ��ǰ����״̬ 0 - ������ 1 - �ƶ����ڣ� 2 - ������С
			xgc_uint16 status;
			/// �����Ƿ���չ
			xgc_bool expand;
			/// ������
			struct
			{
				/// ������ - ��ֱ����
				xgc_int16 v_len;
				/// ������ - ˮƽ����
				xgc_int16 h_len;
				/// ������ - ��ֱλ��
				xgc_int16 v_pos;
				/// ������ - ˮƽλ��
				xgc_int16 h_pos;
			}scroll;

			/// ����������
			buffer_t buffer;
			/// ���崰��λ��
			xgc_int16 tx, ty;
			/// ���崰�ڴ�С
			xgc_int16 tw, th;
			/// �����С
			xgc_int16 bw, bh;
			/// �������������߳�ͬ��
			mutex	mtx;
		};

		/// ���ڶ�������
		static std::atomic< text_window* > _text_window[0xFF];
		/// �����������
		static std::atomic< text_buffer* > _text_buffer[0xFF];
		/// ������������ - ��һ��Ԫ�ش洢���ڸ���
		static std::atomic< window_t > _sort_window[0xFF];
		/// ���ڼ���
		static std::atomic< window_t > _text_window_count = 0;
		/// �ı�˫����
		static xgc_handle	active_buffer[2];
		static xgc_lpvoid	active_buffer_exchange = xgc_nullptr;

		/// �ض�����Ϣ
		struct redirect
		{
			/// �ض�����
			int fd[2];
			/// �ϵľ��
			int old;
			/// �ض��򻺳���
			buffer_t buffer;
			/// ��ʾ����ɫ
			xgc_word color;
		};

		/// ��׼�ض���
		static struct redirect gstdio[3] =
		{
			{ { -1, -1 }, -1, INVALID_BUFFER_INDEX },
			{ { -1, -1 }, -1, INVALID_BUFFER_INDEX },
			{ { -1, -1 }, -1, INVALID_BUFFER_INDEX },
		};

		/// ���ʼ���洢
		static std::atomic< xgc_bool > g_console_library = 0;
		static xgc_handle g_console_thread = 0;

		/// ���λ��
		static xgc_int16 mouse_x = 0;
		static xgc_int16 mouse_y = 0;

		/// ��ť״̬
		static xgc_int16 mouse_s = 0;

		/// ��ǰ�Ľ��㴰��
		static window_t focus_window = INVALID_WINDOW_INDEX;

		/// ���ݻ��������λ�ã����¼��㴰��ƫ��
		static xgc_void adjust_window( text_window* w, xgc_bool update )
		{
			text_buffer *t = _text_buffer[w->buffer];
			XGC_ASSERT_RETURN( t, XGC_NONE );
			guard lock_buffer( t->mtx );

			XGC_ASSERT_RETURN( t->buffer, XGC_NONE );

			// ���ݻ����еĹ��λ�ü��㴰�ڹ��λ�ú͹���λ��
			if( update )
			{
				// ���㻺�����еĹ��λ��
				xgc_int16 x = t->position % t->w;
				xgc_int16 y = t->position / t->w;

				// ��������λ�� - ���Ӧ������λ��
				w->sx = x > w->tw - 1 ? x - (w->tw - 1) : 0;
				w->sy = y > w->th - 1 ? y - (w->th - 1) : 0;

				// ������λ��
				w->cx = x - w->sx;
				w->cy = y - w->sy;
			}
			// ���ݴ��ڹ��λ�ü��㻺����λ�á�
			else
			{
				// ���ܳ������������ֵ
				xgc_int16 y = w->cy + w->sy;
				xgc_int16 x = w->cx + w->sx;

				y = XGC_RNG( y, 0, t->h );
				x = XGC_RNG( x, 0, t->w );

				// ���û��������λ��
				t->position = ( w->cy + w->sy ) * t->w + ( w->cx + w->sx );
			}
		}

		static xgc_void adjust_cursor( text_window *w )
		{
			SetConsoleCursorPosition( active_buffer[0], { w->x + w->tx + w->cx, w->y + w->ty + w->cy } );
		}

		/// �ػ洰��
		static xgc_void redraw_window()
		{
			// �����ػ�
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

			// ��ȡ���ڶ���
			text_window *w = _text_window[focus_window];
			XGC_ASSERT_RETURN( w, false );
			// ��������
			guard lock( w->mtx );

			if( w->style.editor && Input.bKeyDown )
			{
				// �Ƿ�����ַ�
				if( !iscntrl( Input.uChar.AsciiChar ) )
				{
					// �ɱ༭�Ĵ���
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

		// ��������¼�
		static xgc_bool process_mouse_event( MOUSE_EVENT_RECORD &Input )
		{
			//printf( "mouse event status=%04x flags=%04x pos_x=%d, pos_y=%d\n"
			//		, Input.dwButtonState
			//		, Input.dwEventFlags
			//		, Input.dwMousePosition.X
			//		, Input.dwMousePosition.Y );

			// ���ð�ť״̬
			mouse_s = Input.dwButtonState & 0xFFFF;

			xgc_bool bBusy = false;
			switch( Input.dwEventFlags )
			{
			case 0: // ������Ϣ
				if( mouse_s & FROM_LEFT_1ST_BUTTON_PRESSED )
				{
					window_t found = INVALID_WINDOW_INDEX;
					// �������������򼤻��Ѱ�õĴ���
					for( int i = 0; _sort_window[i] != INVALID_WINDOW_INDEX; ++i )
					{
						text_window *w = _text_window[_sort_window[i]];

						if( xgc_nullptr == w )
							continue;

						// ���ô���״̬��1 - ����ڱ������� 2 - �����Sizer
						guard lock( w->mtx );

						if( Input.dwMousePosition.X >= w->x && Input.dwMousePosition.X < w->x + w->cols )
						{
							if( Input.dwMousePosition.Y < w->y || Input.dwMousePosition.Y >= w->y + ( w->expand ? w->rows : w->style.title_bar ) )
								continue;

							if( w->expand && Input.dwMousePosition.Y == w->y + w->rows - 1 )
								w->status = 2; // �����Sizer��
							else
								w->status = 1; // ����λ�ã��ƶ�����

							found = _sort_window[i];
						}
					}

					// �ҵ����ʵ��򼤻�ô���
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

						// �������ù��λ��
						adjust_cursor( w );

						// û���ƶ�����
						if( !(w->status & 0x8000) )
						{
							// ��չ״̬
							if( w->expand )
							{
								if( w->cx >= w->tw || w->cy >= w->th )
									// �رչ����ʾ
									hide_cursor();
								else
									// �򿪹����ʾ
									show_cursor();
							}
							else
							{
								// �رչ����ʾ
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
							case 1: // �ƶ�����λ��
								if( w->style.moving )
								{
									w->x += offset_x;
									w->y += offset_y;

									// �����ƶ���־
									w->status |= 0x8000;
								}
								break;
							case 2: // �ı䴰�ڴ�С
								if( w->style.resize )
								{
									// ���㴰������
									w->cols = w->cols + offset_x;
									w->rows = w->rows + offset_y;

									w->cols = XGC_RNG( w->cols, 8, w->bw + w->style.border * 2 );
									w->rows = XGC_RNG( w->rows, 8, w->bh + w->style.title_bar + w->style.status_bar + w->style.menu );

									// �����ı�����
									w->tw = w->cols - w->style.border * 2;
									w->th = w->rows - w->style.title_bar - w->style.menu - w->style.status_bar;

									w->tw = XGC_RNG( w->tw, 1, w->bw );
									w->th = XGC_RNG( w->th, 1, w->bh );

									// �����ƶ���־
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
				// �ػ洰��
				redraw_window();
			}

			return bBusy;
		}

		/// ����̨�����߳�
		static unsigned int _stdcall console_update_thread( xgc_lpvoid _param )
		{
			char buf[4096];
			g_console_library = 1UL;
			clock_t tick = clock();
			while( g_console_library )
			{
				// �����¼���Ϣ
				process_event();

				// �����ض���
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
		/// \brief ��ʼ������̨
		///
		/// \param cols ���ڿ�
		/// \param rows ���ڸ�
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
			// ����˫����
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
			// �˳�
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
		/// ��Ļ�Ƿ񱻳�ʼ����.
		/// [6/4/2015] create by albert.xu
		///
		xgc_bool console_is_init()
		{
			return g_console_library != 0;
		}

		/// �ض����׼���
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

		/// ��ȡ����̨���ڿ��
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

		// ���ع��
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

				// ���ݹ��λ�ö��봰��
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

			// �����һ����Ч�Ļ��������򴴽�һ����
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

			// ����һ���յ�λ��
			while( idx < XGC_COUNTOF( _text_window ) )
			{
				if( _text_window[idx].load() == xgc_nullptr )
				{
					text_window *w = _text_window[idx] = XGC_NEW text_window;
					guard lock_window( w->mtx );

					text_buffer *b = _text_buffer[t];
					guard lock_buffer( b->mtx );

					// ��ʼ������λ��
					w->x = x;
					w->y = y;

					// ��ʼ�����ڴ�С
					w->cols = cols;
					w->rows = rows;

					// ��ʼ������λ��
					w->sx = 0;
					w->sy = 0;

					// ��ʼ�����λ��
					w->cx = 0;
					w->cy = 0;

					w->title = _strdup( title );
					// ���ڷ��
					w->style.mask = style;
					// ��ǰ����״̬ 0 - ������ 1 - �ƶ����ڣ� 2 - ������С
					w->status = 0;
					// �����Ƿ���չ
					w->expand = true;

					// ������ - ��ֱ����
					w->scroll.v_len = 0;
					// ������ - ˮƽ����
					w->scroll.h_len = 0;
					// ������ - ��ֱλ��
					w->scroll.v_pos = 0;
					// ������ - ˮƽλ��
					w->scroll.h_pos = 0;

					w->buffer = t;

					// ���㴰��������
					w->tw = w->cols - w->style.border * 2;
					w->th = w->rows - w->style.title_bar - w->style.menu - w->style.status_bar;

					// ���û����С
					w->bw = b->w;
					w->bh = b->h;

					// ���㴰��λ��
					w->tx = w->style.border;
					w->ty = w->style.title_bar + w->style.menu;
					// ���½��Ĵ��ڼ��뵽�����б���
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

				// ���ڼ����㷨�� ������_sort_window���������һ������ʾ�����ϲ�
				for( int i = 0; i < XGC_COUNTOF( _sort_window ); ++i )
				{
					if( found )
					{
						if( _sort_window[i] == INVALID_WINDOW_INDEX )
							break;

						// �����������������Ƶ����һ��Ԫ�أ�ע�⣬������Ҫʹ��exchange������ȷ�����߻��������ݰ�ȫ��
						_sort_window[i - 1] = _sort_window[i].exchange( _sort_window[i - 1] );
					}
					else
					{
						// ������
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
		/// \brief �Ƿ�����
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
		/// \brief �ͷ��ı�����
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

		/// �ػ�ָ������
		xgc_void redraw_window( window_t window, xgc_handle output, xgc_bool border )
		{
			XGC_ASSERT_RETURN( window != INVALID_WINDOW_INDEX, XGC_NONE );
			XGC_ASSERT_RETURN( _text_window_count, XGC_NONE );

			// ȡ��ǰ��Ҫ���ƵĴ���
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

			// ������Ƶ�����
			xgc_int16 cols, rows;
			get_console_size( cols, rows );

			xgc_int16 x0 = XGC_MAX( w->x, 0 );
			xgc_int16 x1 = XGC_MIN( w->x + w->cols, cols );

			// ��ǰ���Ƶ���
			xgc_int16 y0 = w->y;
			// �����Ƶ���
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

		/// ��������
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

		/// ���Ϲ�������row��
		/// �ɽ��л��λ�����Ż�
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
			// �������־
			t->flags |= eTextDirty;

			xgc_lpcstr current  = str;
			xgc_uint16 setcolor = 0;
			xgc_uint16 getcolor = t->attr;

			guard lock( t->input.mtx, std::try_to_lock );

			// �������봮
			while( *current )
			{
				// ������ʾ��ɫ
				if( setcolor )
				{
					getcolor = ( getcolor & ( 0xf0 >> (setcolor-1)*4 ) ) | ( (*current % 0x10) << (setcolor-1)*4 );
					setcolor = 0;
				}
				else switch( *current )
				{
				case '\n': // ���з�
					t->buffer[t->position].Attributes = 0;
					t->buffer[t->position].Char.AsciiChar = '\n';
					++t->position;

					while( t->position % t->w )
					{
						t->buffer[t->position].Attributes = 0;
						t->buffer[t->position].Char.AsciiChar = 0;
						++t->position;
					}

					// ��������
					if( false == lock.owns_lock() )
					{
						if( t->input.input_cur < t->input.input_len )
							t->input.input_buf[t->input.input_cur++] = '\n';

						t->input.input_pos = 0;
						t->input.input_complate = true;
						// ֪ͨ�������
						t->input.evt.notify_one();
					}
					break;
				case '\b':
					// ��������
					if( false == lock.owns_lock() )
					{
						if( t->input.input_cur > 0 )
							t->input.input_buf[--t->input.input_cur] = 0;

						// ��д���뻺��
						if( t->position == 0 && t->input.input_cur != 0 )
						{
							xgc_int16 len = XGC_MIN( t->w, t->input.input_cur );
							for( xgc_int16 i = 0; i < len; ++i, ++t->position )
							{
								t->buffer[i].Attributes		= t->attr;
								t->buffer[i].Char.AsciiChar = t->input.input_buf[i];
							}

							// �����ɾ��һ���ַ������ﲻ��Ҫ�ˡ�
							break;
						}
					}

					// ���ռλ��
					while( t->position > t->input.input_pos )
					{
						--t->position;

						if( t->buffer[t->position].Char.AsciiChar != 0 )
							break;
					}

					// ���һ���ַ�
					if( t->position >= t->input.input_pos )
					{
						t->buffer[t->position].Attributes = 0;
						t->buffer[t->position].Char.AsciiChar = 0;
					}
					break;
				case '\t': // �Ʊ��
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
				case '\x1': // ǰ��ɫ
					setcolor = 1;
					break;
				case '\x2': // ����ɫ
					setcolor = 2;
					break;
				default: // �����ַ�
					if( false == lock.owns_lock() )
					{
						// ���ܳ��������С
						if( t->input.input_cur >= t->input.input_len )
							break;

						t->input.input_buf[t->input.input_cur++] = *current;
					}

					t->buffer[t->position].Attributes     = getcolor;
					t->buffer[t->position].Char.AsciiChar = *current;
					++t->position;

					break;
				}

				// �ж��Ƿ���Ҫ����
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

		/// �����������ÿ�߳�һ��
		xgc_int32 buffer_read( buffer_t text, xgc_lpstr buf, xgc_int16 len, xgc_bool wait )
		{
			memset( buf, 0, len );

			text_buffer *t = _text_buffer[text];
			XGC_ASSERT_RETURN( t, -1 );

			// �����룬������ͬʱ��һ���������������
			guard lock_input( t->input.mtx );

			// ������
			guard lock_event( t->mtx );

			if( wait )
			{
				// �ͷ�˽���ڴ�
				if( t->input.input_alc )
					free( t->input.input_buf );

				// ��ʼ���������
				t->input.input_buf = buf;
				t->input.input_len = len;
				t->input.input_alc = 0;
				t->input.input_cur = 0;
				t->input.input_pos = t->position;
				t->input.input_complate = false;

				// �ȴ��������
				t->input.evt.wait( lock_event );

				// ��������ĳ���
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

					// ��ʼ�����뻺��
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