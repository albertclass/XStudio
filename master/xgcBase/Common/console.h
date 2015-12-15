#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "defines.h"
#include "exports.h"

#define INVALID_WINDOW_INDEX xgc_uint8(0xffU)
#define INVALID_BUFFER_INDEX xgc_uint8(0xffU)

namespace XGC
{
	namespace console
	{
		typedef xgc_uint8	window_t;
		typedef xgc_uint8	buffer_t;

		///
		/// [12/24/2013 albert.xu]
		/// 初始化控制台
		///
		COMMON_API xgc_bool	initialize_console_library( xgc_int16 rows, xgc_int16 cols, PHANDLER_ROUTINE pfnRoutine );

		///
		/// [12/24/2013 albert.xu]
		/// 释放控制台
		///
		COMMON_API xgc_void	finialize_console_library();

		///
		/// 屏幕是否被初始化了.
		/// [6/4/2015] create by albert.xu
		///
		COMMON_API xgc_bool is_console_library();

		///
		/// [3/4/2014 albert.xu]
		/// 重定向标准输出
		///
		COMMON_API xgc_bool	redirect( FILE* stdfile, window_t window );

		///
		/// [3/5/2014 albert.xu]
		/// 设置输出颜色
		///
		COMMON_API xgc_void	setcolor( xgc_word color );

		///
		/// [1/20/2014 albert.xu]
		/// 输入更新
		///
		COMMON_API xgc_size update_input( window_t window, xgc_lpstr buf, xgc_size len );

		///
		/// [12/24/2013 albert.xu]
		/// 获取控制台窗口宽度
		///
		COMMON_API xgc_uint16 get_console_cols();

		///
		/// [12/24/2013 albert.xu]
		/// 获取控制台窗口高度
		///
		COMMON_API xgc_uint16 get_console_rows();

		///
		/// [12/24/2013 albert.xu]
		/// 隐藏光标
		///
		COMMON_API xgc_void	show_cursor( xgc_bool show = true );

		///
		/// [12/24/2013 albert.xu]
		/// 定位到坐标
		/// 屏幕坐标最小值为 1
		///
		COMMON_API xgc_bool gotoxy( window_t window, xgc_int16 col, xgc_int16 row );

		///
		/// [12/24/2013 albert.xu]
		/// 分配一个文本缓冲,宽度为w, 高度为h
		///
		COMMON_API buffer_t buffer( xgc_uint16 w, xgc_uint16 h );

		///
		/// [12/24/2013 albert.xu]
		/// 分配一个窗口, 并和text关联起来
		/// x, y : 窗口位置(最小值为零)
		/// cols, rows : 窗口大小
		/// text : 文本缓冲句柄
		///
		COMMON_API window_t	window( xgc_int16 x, xgc_int16 y, xgc_int16 cols, xgc_int16 rows, buffer_t &t, xgc_bool border = true );

		///
		/// [1/12/2014 albert.xu]
		/// 获取窗口关联的缓冲区句柄
		///
		COMMON_API buffer_t	get_window_buffer( window_t window );

		///
		/// [12/24/2013 albert.xu]
		/// 激活窗口
		///
		COMMON_API xgc_void	active_window( window_t window );

		///
		/// 根据缓冲区光标位置，重新计算窗口偏移
		/// [6/1/2015] create by albert.xu
		///
		COMMON_API xgc_void adjust_window( window_t window );

		///
		/// [12/24/2013 albert.xu]
		/// 绘制窗口(不包含窗口内容)
		///
		COMMON_API xgc_void	draw_window( window_t window, xgc_bool border = true );

		///
		/// [12/25/2013 albert.xu]
		/// 滚动窗口
		///
		COMMON_API xgc_void	scroll_window( window_t window, xgc_uint16 row, xgc_uint16 col );

		///
		/// [12/24/2013 albert.xu]
		/// 释放文本缓冲
		///
		COMMON_API xgc_void	free_text( buffer_t );

		///
		/// [12/24/2013 albert.xu]
		/// 释放窗口
		///
		COMMON_API xgc_void	free_window( window_t );

		///
		/// [12/24/2013 albert.xu]
		/// 输出文字到窗口
		///
		COMMON_API xgc_int32 printf_text( buffer_t text, xgc_lpcstr fmt, ... );

		///
		/// [12/24/2013 albert.xu]
		/// 输出文字到窗口
		///
		COMMON_API xgc_int32 output_text( buffer_t text, xgc_lpcstr fmt, va_list ap );
	}
}
#endif //_CONSOLE_H_