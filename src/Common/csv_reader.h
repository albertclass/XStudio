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
		/// \brief cvs表格读取类
		///
		/// \author albert.xu
		/// \date 2016/01/26 18:12
		///
		class COMMON_API csv_reader
		{
		protected:
			/// 每一个块都指向一块内存区域，其中保存了4096行数据。
			xgc_lpvoid		chunk[128];
			/// 缓冲区指针
			xgc_lpstr		buffer;
			/// 表格列数量
			xgc_size		cols;
			/// 表格行数量
			xgc_size		rows;
			/// 当前列指针
			mutable
			xgc_int32		fetch;
			/// 是否存在标题栏
			xgc_bool		title;
		public:
			///
			/// \brief 构造
			///
			/// \author albert.xu
			/// \date 2016/01/26 18:12
			///
			csv_reader( xgc_void );

			///
			/// \brief 析构，释放资源
			///
			/// \author albert.xu
			/// \date 2016/01/26 18:12
			///
			~csv_reader( xgc_void );
		private:
			///
			/// 获取单元格内容 
			/// [6/10/2014] create by albert.xu
			/// @param row 文件的行号
			/// @param col 表格的列号
			///
			xgc_lpcstr get_cell( xgc_size row, xgc_size col ) const throw();

			///
			/// 获取单元格内容 
			/// [6/10/2014] create by albert.xu
			/// @param row 文件的行号
			/// @param col 表格的列号
			///
			xgc_lpcstr set_cell( xgc_size row, xgc_size col, xgc_lpstr str );

			///
			/// 获取标题所在的列 
			/// [6/10/2014] create by albert.xu
			/// @param title 标题
			///
			xgc_int32 get_col( xgc_lpcstr title ) const throw();

			///
			/// 分析整个文件 
			/// [6/10/2014] create by albert.xu
			/// @param buffer 缓冲区首地址
			/// @param buffer_size 缓冲区大小
			/// @param split 分隔符
			///
			xgc_bool parse( xgc_lpstr buffer, xgc_size buffer_size, xgc_char split );

		public:
			///
			/// \brief 读取指定的表格文件
			///
			/// \param pathname 文件名
			/// 
			/// \author albert.xu
			/// \date 2015/12/16 17:39
			///
			xgc_bool load( xgc_lpcstr pathname, xgc_char split = ',', xgc_bool has_title = true );

			///
			/// \brief 读取单元格数据
			/// \param row 行号
			/// \param col 列号
			///
			xgc_lpcstr get_value( xgc_size row, xgc_size col, xgc_lpcstr default ) const throw();

			///
			/// 读取单元格数据
			/// \param row 行号
			/// \param title 表头
			///
			xgc_lpcstr get_value( xgc_size row, xgc_lpcstr title, xgc_lpcstr default ) const throw();

			///
			/// \brief 获取单元格数据，bool特化
			///
			/// \param row 行号
			/// \param col 列号
			/// \author albert.xu
			/// \date 2015/12/16 18:02
			///
			xgc_bool get_value( xgc_size row, xgc_size col, xgc_bool default ) const throw();

			///
			/// \brief 获取单元格数据，bool特化
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
			/// \brief 获取单元格数据，数组特化
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
			/// \brief 获取单元格数据 数值类型
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
			/// \brief 获取列数量
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:03
			///
			xgc_size get_cols() { return cols; }

			///
			/// \brief 获取行数
			///
			/// \author albert.xu
			/// \date 2015/12/16 18:03
			///
			xgc_size get_rows() { return title ? rows - 1 : rows; }
		};
	}
}
#endif // _EXCEL_H_