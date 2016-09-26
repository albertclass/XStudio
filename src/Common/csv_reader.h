#pragma once
#ifndef _EXCEL_H_
#define _EXCEL_H_
#include <fcntl.h>

#include "defines.h"
#include "exports.h"

namespace xgc
{
	namespace common
	{
		///
		/// \brief cvs����ȡ��
		///
		/// \author albert.xu
		/// \date 2016/01/26 18:12
		///
		class COMMON_API csv_reader
		{
		protected:
			/// ÿһ���鶼ָ��һ���ڴ��������б�����4096�����ݡ�
			xgc_lpvoid		chunk[128];
			/// ������ָ��
			xgc_lpstr		buffer;
			/// ���������
			xgc_size		cols;
			/// ���������
			xgc_size		rows;
			/// ��ǰ��ָ��
			mutable
			xgc_int32		fetch;
			/// �Ƿ���ڱ�����
			xgc_bool		title;
		public:
			///
			/// \brief ����
			///
			/// \author albert.xu
			/// \date 2016/01/26 18:12
			///
			csv_reader( xgc_void );

			///
			/// \brief �������ͷ���Դ
			///
			/// \author albert.xu
			/// \date 2016/01/26 18:12
			///
			~csv_reader( xgc_void );
		private:
			///
			/// ��ȡ��Ԫ������ 
			/// [6/10/2014] create by albert.xu
			/// @param row �ļ����к�
			/// @param col �����к�
			///
			xgc_lpcstr get_cell( xgc_size row, xgc_size col ) const throw();

			///
			/// ��ȡ��Ԫ������ 
			/// [6/10/2014] create by albert.xu
			/// @param row �ļ����к�
			/// @param col �����к�
			///
			xgc_lpcstr set_cell( xgc_size row, xgc_size col, xgc_lpstr str );

			///
			/// ��ȡ�������ڵ��� 
			/// [6/10/2014] create by albert.xu
			/// @param title ����
			///
			xgc_int32 get_col( xgc_lpcstr title ) const throw();

			///
			/// ���������ļ� 
			/// [6/10/2014] create by albert.xu
			/// @param buffer �������׵�ַ
			/// @param buffer_size ��������С
			/// @param split �ָ���
			///
			xgc_bool parse( xgc_lpstr buffer, xgc_size buffer_size, xgc_char split );

		public:
			///
			/// \brief ��ȡָ���ı���ļ�
			///
			/// \param pathname �ļ���
			/// 
			/// \author albert.xu
			/// \date 2015/12/16 17:39
			///
			xgc_bool load( xgc_lpcstr pathname, xgc_char split = ',', xgc_bool has_title = true );

			///
			/// \brief ��ȡ��Ԫ������
			/// \param row �к�
			/// \param col �к�
			///
			xgc_lpcstr get_value( xgc_size row, xgc_size col, xgc_lpcstr default ) const throw();

			///
			/// ��ȡ��Ԫ������
			/// \param row �к�
			/// \param title ��ͷ
			///
			xgc_lpcstr get_value( xgc_size row, xgc_lpcstr title, xgc_lpcstr default ) const throw();

			///
			/// \brief ��ȡ��Ԫ�����ݣ�bool�ػ�
			///
			/// \param row �к�
			/// \param col �к�
			/// \author albert.xu
			/// \date 2015/12/16 18:02
			///
			xgc_bool get_value( xgc_size row, xgc_size col, xgc_bool default ) const throw();

			///
			/// \brief ��ȡ��Ԫ�����ݣ�bool�ػ�
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:02
			///
			xgc_bool get_value( xgc_size row, xgc_lpcstr title, xgc_bool default ) const throw()
			{
				XGC_ASSERT_RETURN( title, false );
				return get_value( row, get_col(title), default );
			}

			///
			/// \brief ��ȡ��Ԫ�����ݣ������ػ�
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:02
			///
			template< size_t S >
			xgc_bool get_value( xgc_size row, xgc_lpcstr title, xgc_char( &ret )[S] ) const throw()
			{
				xgc_lpcstr pValue = get_value( row, title );
				if( pValue )
				{
					strcpy_s( ret, S, pValue );
					return true;
				}

				return false;
			}

			template< size_t S >
			xgc_bool get_value( xgc_size row, xgc_size col, xgc_char( &ret )[S] ) const throw()
			{
				xgc_lpcstr pValue = get_value( row, col );
				if( pValue )
				{
					strcpy_s( ret, S, pstrValue );
					return true;
				}

				return false;
			}

			///
			/// \brief ��ȡ��Ԫ������ ��ֵ����
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:03
			///
			template< class T, typename std::enable_if< std::is_integral< T >::value || std::is_floating_point< T >::value, xgc_bool >::type = true >
			T get_value( xgc_size row, xgc_size col, T default = 0 ) const throw()
			{
				xgc_lpcstr val = get_value( row, col, xgc_nullptr );
				if( xgc_nullptr == val )
					return default;

				if( val[0] == 0 )
					return default;

				return str2numeric< T >( val );
			}

			template< class T, typename std::enable_if< std::is_integral< T >::value || std::is_floating_point< T >::value, xgc_bool >::type = true >
			T get_value( xgc_size row, xgc_lpcstr title, T default = 0 ) const throw()
			{
				XGC_ASSERT_RETURN( title, false );
				return get_value( row, get_col(title), default );
			}

			///
			/// \brief ��ȡ������
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:03
			///
			xgc_size get_cols() { return cols; }

			///
			/// \brief ��ȡ����
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:03
			///
			xgc_size get_rows() { return title ? rows - 1 : rows; }
		};
	}
}
#endif // _EXCEL_H_