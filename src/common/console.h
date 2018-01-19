#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "defines.h"
#include "exports.h"

#ifdef _WINDOWS
#define INVALID_WINDOW_INDEX xgc_uint8(0xffU)
#define INVALID_BUFFER_INDEX xgc_uint8(0xffU)

namespace xgc
{
	namespace console
	{
		typedef xgc_uint8	window_t;
		typedef xgc_uint8	buffer_t;

		/// 边框
		#define WINDOW_STYLE_BORDER	0x0001
		/// 是否收起，只留标题栏
		#define WINDOW_STYLE_EXPAND	0x0002
		/// 是否有菜单相
		#define WINDOW_STYLE_MENU	0x0004
		/// 是否有标题栏
		#define WINDOW_STYLE_TITLE_BAR	0x0008
		/// 是否有状态栏
		#define WINDOW_STYLE_STATUS_BAR	0x0010
		/// 是否有滚动条
		#define WINDOW_STYLE_SCROLL_BAR	0x0020
		/// 是否自动滚屏
		#define WINDOW_STYLE_AUTOSCROLL	0x0040
		/// 是否能改变大小
		#define WINDOW_STYLE_RESIZE	0x0080
		/// 是否可以移动窗口
		#define WINDOW_STYLE_MOVING 0x0100
		/// 是否可以编辑
		#define WINDOW_STYLE_EDITOR	0x0200
		/// 默认的窗口风格
		#define WINDOW_STYLE_DEFAULT 0x01FB

		///
		/// \brief 初始化控制台
		///
		/// \param cols 窗口宽
		/// \param rows 窗口高
		///
		/// [12/24/2013 albert.xu]
		///
		COMMON_API xgc_bool	init_console( xgc_int16 cols, xgc_int16 rows );

		///
		/// [12/24/2013 albert.xu]
		/// 释放控制台
		///
		COMMON_API xgc_void	fini_console();

		///
		/// 屏幕是否被初始化了.
		/// [6/4/2015] create by albert.xu
		///
		COMMON_API xgc_bool console_is_init();

		///
		/// [3/4/2014 albert.xu]
		/// 重定向标准输出
		///
		COMMON_API xgc_bool	redirect( FILE* stdfile, buffer_t buffer );

		///
		/// \brief 获取控制台窗口宽度
		///
		COMMON_API xgc_bool get_console_size( xgc_int16 &cx, xgc_int16 &cy );

		///
		/// \brief 显示光标
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void	show_cursor( xgc_real32 percent = 0.1f );

		///
		/// \brief 隐藏光标
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void	hide_cursor();

		///
		/// \brief 定位到坐标
		/// 屏幕坐标最小值为 1
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void gotoxy( xgc_int16 col, xgc_int16 row, window_t window = INVALID_WINDOW_INDEX );

		///
		/// \brief 刷新窗口
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void refresh();

		///
		/// \brief 分配一个文本缓冲,宽度为w, 高度为h
		///
		/// \param w 缓冲区宽
		/// \param h 缓冲区高
		/// \return 缓冲区句柄
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API buffer_t buffer( xgc_uint16 w, xgc_uint16 h );

		///
		/// [1/12/2014 albert.xu]
		/// 获取窗口关联的缓冲区句柄
		///
		COMMON_API buffer_t	buffer( window_t window );

		///
		/// \brief 分配一个窗口, 并和text关联起来
		///
		/// \param x, y 窗口位置(最小值为零)
		/// \param cols, rows 窗口大小
		/// \param t 文本缓冲句柄
		/// \return 窗口句柄
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:35
		///
		COMMON_API window_t	window( xgc_int16 x, xgc_int16 y, xgc_int16 cols, xgc_int16 rows, buffer_t &t, xgc_uint16 style, xgc_lpcstr title = xgc_nullptr );

		///
		/// \brief 设置窗口样式
		///
		/// \author albert.xu
		/// \date 2015/12/14 10:35
		///

		COMMON_API xgc_void window_style( window_t window, xgc_uint16 style );

		///
		/// \brief 激活窗口
		///
		/// \author albert.xu
		/// \date 2016/01/05 16:43
		///

		COMMON_API xgc_void	active_window( window_t window );

		///
		/// \brief 是否活动窗口
		///
		/// \author albert.xu
		/// \date 2016/01/05 16:43
		///

		COMMON_API xgc_bool is_active_window( window_t window );

		///
		/// \breif 绘制窗口(不包含窗口内容)
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_void	redraw_window( window_t window, xgc_bool border );

		///
		/// \breif 滚动窗口
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///

		COMMON_API xgc_void	scroll_window( window_t window, xgc_int16 row, xgc_int16 col );

		///
		/// \brief 释放文本缓冲
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_void	free_buffer( buffer_t );

		///
		/// \brief 释放窗口
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_void	free_window( window_t );

		///
		/// \brief 输出文字到窗口
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_int32 printf_text( buffer_t text, xgc_lpcstr fmt, ... );

		///
		/// \brief 输出文字到窗口
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///

		COMMON_API xgc_int32 output_text( buffer_t text, xgc_lpcstr fmt, va_list ap );

		///
		/// \brief 从缓冲读取一行
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_int32 buffer_read( buffer_t text, xgc_lpstr buf, xgc_int16 len, xgc_bool wait = true );
	}
}
#endif //_WINDOWS
#endif //_CONSOLE_H_