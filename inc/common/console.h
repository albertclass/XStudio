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

		/// �߿�
		#define WINDOW_STYLE_BORDER	0x0001
		/// �Ƿ�����ֻ��������
		#define WINDOW_STYLE_EXPAND	0x0002
		/// �Ƿ��в˵���
		#define WINDOW_STYLE_MENU	0x0004
		/// �Ƿ��б�����
		#define WINDOW_STYLE_TITLE_BAR	0x0008
		/// �Ƿ���״̬��
		#define WINDOW_STYLE_STATUS_BAR	0x0010
		/// �Ƿ��й�����
		#define WINDOW_STYLE_SCROLL_BAR	0x0020
		/// �Ƿ��Զ�����
		#define WINDOW_STYLE_AUTOSCROLL	0x0040
		/// �Ƿ��ܸı��С
		#define WINDOW_STYLE_RESIZE	0x0080
		/// �Ƿ�����ƶ�����
		#define WINDOW_STYLE_MOVING 0x0100
		/// �Ƿ���Ա༭
		#define WINDOW_STYLE_EDITOR	0x0200
		/// Ĭ�ϵĴ��ڷ��
		#define WINDOW_STYLE_DEFAULT 0x01FB

		///
		/// \brief ��ʼ������̨
		///
		/// \param cols ���ڿ�
		/// \param rows ���ڸ�
		///
		/// [12/24/2013 albert.xu]
		///
		COMMON_API xgc_bool	init_console( xgc_int16 cols, xgc_int16 rows );

		///
		/// [12/24/2013 albert.xu]
		/// �ͷſ���̨
		///
		COMMON_API xgc_void	fini_console();

		///
		/// ��Ļ�Ƿ񱻳�ʼ����.
		/// [6/4/2015] create by albert.xu
		///
		COMMON_API xgc_bool console_is_init();

		///
		/// [3/4/2014 albert.xu]
		/// �ض����׼���
		///
		COMMON_API xgc_bool	redirect( FILE* stdfile, buffer_t buffer );

		///
		/// \brief ��ȡ����̨���ڿ��
		///
		COMMON_API xgc_bool get_console_size( xgc_int16 &cx, xgc_int16 &cy );

		///
		/// \brief ��ʾ���
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void	show_cursor( xgc_real32 percent = 0.1f );

		///
		/// \brief ���ع��
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void	hide_cursor();

		///
		/// \brief ��λ������
		/// ��Ļ������СֵΪ 1
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void gotoxy( xgc_int16 col, xgc_int16 row, window_t window = INVALID_WINDOW_INDEX );

		///
		/// \brief ˢ�´���
		///
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API xgc_void refresh();

		///
		/// \brief ����һ���ı�����,���Ϊw, �߶�Ϊh
		///
		/// \param w ��������
		/// \param h ��������
		/// \return ���������
		/// \author albert.xu
		/// \date 2015/12/24 10:00
		///
		COMMON_API buffer_t buffer( xgc_uint16 w, xgc_uint16 h );

		///
		/// [1/12/2014 albert.xu]
		/// ��ȡ���ڹ����Ļ��������
		///
		COMMON_API buffer_t	buffer( window_t window );

		///
		/// \brief ����һ������, ����text��������
		///
		/// \param x, y ����λ��(��СֵΪ��)
		/// \param cols, rows ���ڴ�С
		/// \param t �ı�������
		/// \return ���ھ��
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:35
		///
		COMMON_API window_t	window( xgc_int16 x, xgc_int16 y, xgc_int16 cols, xgc_int16 rows, buffer_t &t, xgc_uint16 style, xgc_lpcstr title = xgc_nullptr );

		///
		/// \brief ���ô�����ʽ
		///
		/// \author albert.xu
		/// \date 2015/12/14 10:35
		///

		COMMON_API xgc_void window_style( window_t window, xgc_uint16 style );

		///
		/// \brief �����
		///
		/// \author albert.xu
		/// \date 2016/01/05 16:43
		///

		COMMON_API xgc_void	active_window( window_t window );

		///
		/// \brief �Ƿ�����
		///
		/// \author albert.xu
		/// \date 2016/01/05 16:43
		///

		COMMON_API xgc_bool is_active_window( window_t window );

		///
		/// \breif ���ƴ���(��������������)
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_void	redraw_window( window_t window, xgc_bool border );

		///
		/// \breif ��������
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///

		COMMON_API xgc_void	scroll_window( window_t window, xgc_int16 row, xgc_int16 col );

		///
		/// \brief �ͷ��ı�����
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_void	free_buffer( buffer_t );

		///
		/// \brief �ͷŴ���
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_void	free_window( window_t );

		///
		/// \brief ������ֵ�����
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_int32 printf_text( buffer_t text, xgc_lpcstr fmt, ... );

		///
		/// \brief ������ֵ�����
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///

		COMMON_API xgc_int32 output_text( buffer_t text, xgc_lpcstr fmt, va_list ap );

		///
		/// \brief �ӻ����ȡһ��
		///
		/// \author albert.xu
		/// \date 2015/12/07 20:31
		///
		COMMON_API xgc_int32 buffer_read( buffer_t text, xgc_lpstr buf, xgc_int16 len, xgc_bool wait = true );
	}
}
#endif //_WINDOWS
#endif //_CONSOLE_H_