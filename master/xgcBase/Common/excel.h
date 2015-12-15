#pragma once
#ifndef _EXCEL_H_
#define _EXCEL_H_
#include <vector>
#include <regex>

#include "defines.h"
#include "exports.h"

namespace XGC
{
	namespace common
	{
#define EXCEL_BUFFER_SIZE	512
		class COMMON_API excel
		{
		public:
			excel( xgc_void );
			~excel( xgc_void );
		private:
			///
			/// 获取单元格内容 
			/// [6/10/2014] create by albert.xu
			/// @param nRow 文件的行号
			/// @param nCol 表格的列号
			///
			xgc_lpcstr getCell( xgc_size nRow, xgc_size nCol );

			///
			/// 获取单元格内容 
			/// [6/10/2014] create by albert.xu
			/// @param nRow 文件的行号
			/// @param nCol 表格的列号
			///
			xgc_bool setCell( xgc_size nRow, xgc_size nCol, xgc_lpstr pStr );

			///
			/// 获取标题所在的列 
			/// [6/10/2014] create by albert.xu
			/// @param pTitle 标题
			///
			xgc_int32 getColByTitle( xgc_lpcstr pTitle );

			///
			/// 分析整个文件 
			/// [6/10/2014] create by albert.xu
			/// @param buffer 缓冲区首地址
			/// @param buffer_size 缓冲区大小
			///
			xgc_bool AnalyzeFile( xgc_lpstr buffer, xgc_size buffer_size );

		public:
			//	读取指定的表格文件
			//	strFileName	:	文件名
			xgc_bool ReadExcelFile( xgc_lpcstr strFileName );

			///
			/// 读取单元格数据
			/// [7/28/2015] create by albert.xu
			///
			xgc_lpcstr GetCellValue( xgc_size nLine, xgc_size nCol )
			{
				return getCell( nLine + 1, nCol );
			}

			///
			/// 读取单元格数据
			/// [7/28/2015] create by albert.xu
			///
			xgc_lpcstr GetCellValue( xgc_size nLine, xgc_lpcstr lpszTitle )
			{
				if( lpszTitle == xgc_nullptr )
					return xgc_nullptr;

				return getCell( nLine + 1, getColByTitle( lpszTitle ) );
			}

			template< size_t S >
			xgc_bool GetCellValue( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_char( &szValue )[S] )
			{
				xgc_lpcstr pValue = GetCellValue( nLine, lpszTitle );
				if( pValue )
				{
					strcpy_s( szValue, S, pValue );
					return true;
				}

				return false;
			}

			template< size_t S >
			xgc_bool GetCellValue( xgc_size nLine, xgc_size nCol, xgc_char( &szValue )[S] )
			{
				xgc_lpcstr pValue = GetCellValue( nLine, nCol );
				if( pValue )
				{
					strcpy_s( szValue, S, pstrValue );
					return true;
				}

				return false;
			}

			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_real32 *pVal, xgc_real32 fDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_int16  *pVal, xgc_int16  nDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_int32  *pVal, xgc_int32  nDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_int64  *pVal, xgc_int64  nDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_uint16 *pVal, xgc_uint16 nDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_uint32 *pVal, xgc_uint32 nDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_uint64 *pVal, xgc_uint64 nDefault );
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_bool   *pVal, xgc_bool	 nDefault );

			//template< class T, typename std::enable_if< std::_Is_numeric< T >::value, xgc_bool >::type = true >
			//T GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, T _Default )
			//{
			//	XGC_ASSERT_THROW( lpszTitle, std::logic_error( "title is nullptr." ) );
			//	xgc_lpcstr pValue = xgc_nullptr;
			//	if( !GetElement( nLine, lpszTitle, &pValue ) )
			//		return _Default;

			//	return str2numeric< T >( pValue );
			//}

			//template<>
			//xgc_bool GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_bool _Default )
			//{
			//	XGC_ASSERT_RETURN( lpszTitle, false );
			//	xgc_lpcstr pValue = xgc_nullptr;
			//	if( !GetElement( nLine, lpszTitle, &pValue ) )
			//		return _Default;

			//	if( _stricmp( pValue, "0" ) == 0 )
			//		return false;

			//	if( _stricmp( pValue, "1" ) == 0 )
			//		return true;

			//	if( _stricmp( pValue, "false" ) == 0 )
			//		return false;

			//	if( _stricmp( pValue, "true" ) == 0 )
			//		return true;

			//	return false;
			//}

			//////////////////////////////////////////////////////////////////////////
			template< class T, typename std::enable_if< std::is_integral< T >::value || std::is_floating_point< T >::value, xgc_bool >::type = true >
			xgc_bool GetCellValue( xgc_size nLine, xgc_lpcstr lpszTitle, T& _Val, T _Default = 0 )
			{
				XGC_ASSERT_THROW( lpszTitle, std::logic_error( "title is nullptr." ) );
				xgc_lpcstr pValue = GetCellValue( nLine, lpszTitle );
				if( xgc_nullptr == pValue )
				{
					_Val = _Default;
					return false;
				}

				if( pValue[0] == 0 )
				{
					_Val = _Default;
					return false;
				}

				_Val = str2numeric< T >( pValue );
				return true;
			}

			template< class T, typename std::enable_if< std::is_integral< T >::value || std::is_floating_point< T >::value, xgc_bool >::type = true >
			xgc_bool GetCellValue( xgc_size nLine, xgc_size nCol, T& _Val, T _Default = 0 )
			{
				xgc_lpcstr pValue = GetCellValue( nLine, nCol );
				if( xgc_nullptr == pValue )
				{
					_Val = _Default;
					return false;
				}

				if( pValue[0] == 0 )
				{
					_Val = _Default;
					return false;
				}

				_Val = str2numeric< T >( pValue );
				return true;
			}

			template<>
			xgc_bool GetCellValue( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_bool& _Val, xgc_bool _Default )
			{
				XGC_ASSERT_RETURN( lpszTitle, false );
				xgc_lpcstr pValue = GetCellValue( nLine, lpszTitle );
				if( xgc_nullptr == pValue )
				{
					_Val = _Default;
					return false;
				}

				if( _stricmp( pValue, "0" ) == 0 )
					_Val = false;
				else if( _stricmp( pValue, "1" ) == 0 )
					_Val = true;
				else if( _stricmp( pValue, "false" ) == 0 )
					_Val = false;
				else if( _stricmp( pValue, "true" ) == 0 )
					_Val = true;
				else
				{
					_Val = _Default;
					return false;
				}

				return true;
			}

			template<>
			xgc_bool GetCellValue( xgc_size nLine, xgc_size nCol, xgc_bool& _Val, xgc_bool _Default )
			{
				xgc_lpcstr pValue = GetCellValue( nLine, nCol );
				if( xgc_nullptr == pValue )
				{
					_Val = _Default;
					return false;
				}

				if( _stricmp( pValue, "0" ) == 0 )
					_Val = false;
				else if( _stricmp( pValue, "1" ) == 0 )
					_Val = true;
				else if( _stricmp( pValue, "false" ) == 0 )
					_Val = false;
				else if( _stricmp( pValue, "true" ) == 0 )
					_Val = true;
				else
				{
					_Val = _Default;
					return false;
				}

				return true;
			}

			xgc_bool GetCellValue( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_string &_Val )
			{
				xgc_lpcstr pValue = GetCellValue( nLine, lpszTitle );
				if( pValue )
				{
					_Val = pValue;
					return true;
				}

				return false;
			}

			xgc_bool GetCellValue( xgc_size nLine, xgc_size nCol, xgc_string &_Val )
			{
				xgc_lpcstr pValue = GetCellValue( nLine, nCol );
				if( pValue )
				{
					_Val = pValue;
					return true;
				}

				return false;
			}

			xgc_int32	 GetColCount() { return mColCount; }
			xgc_int32	 GetRowCount() { return mRowCount - 1; }
		protected:

			struct	impl;
			impl*	m_impl;

			xgc_lpvoid		mChunk[128]; ///< 每一个块都指向一块内存区域，其中保存了4096行数据。

			xgc_uint32		mColCount;
			xgc_uint32		mRowCount;

			xgc_lpstr		mBuffer;
			xgc_uint32		mTitleNext;

		};

		//////////////////////////////////////////////////////////////////////////
		// [1/6/2014 jianglei.kinly]
		// 用于类型转换的模板函数
		//////////////////////////////////////////////////////////////////////////
		static xgc_lpcstr RawPoint( xgc_lpcstr pstr )
		{
			return pstr;
		}

		static xgc_lpcstr RawPoint( std::string &str )
		{
			return str.c_str();
		}

		template< class T, class TSTR, typename std::enable_if< std::is_integral< T >::value, xgc_bool >::type = true >
		T StringConvert( typename std::remove_reference< TSTR >::type& pStr )
		{
			return (T) atoi( RawPoint( pStr ) );
		}
		template< class T, class TSTR, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
		T StringConvert( typename std::remove_reference< TSTR >::type& pStr )
		{
			return (T) atof( RawPoint( pStr ) );
		}

		//////////////////////////////////////////////////////////////////////////
		// [1/6/2014 jianglei.kinly]
		// 基础类
		//////////////////////////////////////////////////////////////////////////
		class CCheckArgBasic
		{
		public:
			virtual xgc_bool check( xgc_lpcstr str ) = 0;
		};

		//////////////////////////////////////////////////////////////////////////
		// [1/6/2014 jianglei.kinly]
		// 一般类型区间检查 int longlong float
		//////////////////////////////////////////////////////////////////////////
		template<class _T>
		class CValueCheck : public CCheckArgBasic
		{
		private:
			xgc_bool m_left_close;  // [ true ( false
			xgc_bool m_right_close; // ] true ) false
			_T m_value_min;
			_T m_value_max;
		public:
			CValueCheck( xgc_bool left, xgc_bool right, _T min, _T max )
				: m_left_close( left )
				, m_right_close( right )
				, m_value_min( min )
				, m_value_max( max )
			{
			}

			virtual xgc_bool check( xgc_lpcstr str )
			{
				_T tmp = StringConvert<_T, xgc_lpcstr>( str );
				if( m_left_close )
				{
					if( tmp >= m_value_min )
					{
						if( m_right_close )
						{
							if( tmp <= m_value_max )
								return true;
						}
						else if( tmp < m_value_max )
						{
							return true;
						}
					}
				}  // end if (m_left_close)
				else if( tmp > m_value_min )
				{
					if( m_right_close )
					{
						if( tmp <= m_value_max )
							return true;
					}
					else if( tmp < m_value_max )
						return true;
				}  // end else if 
				return false;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		// [1/6/2014 jianglei.kinly]
		// 正则检查
		//////////////////////////////////////////////////////////////////////////
		class CStringCheck : public CCheckArgBasic
		{
		private:
			std::string m_regex;

		public:
			CStringCheck( const char* str )
				: m_regex( str )
			{
			}

			virtual xgc_bool check( xgc_lpcstr str )
			{
				const std::regex pattern( m_regex.c_str() );
				if( std::regex_match( str, pattern ) )
					return true;
				return false;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		// [1/6/2014 jianglei.kinly]
		// 返回检查结果
		//////////////////////////////////////////////////////////////////////////
		xgc_bool Check( xgc_lpcstr match, xgc_lpcstr check_string );
	}
}
#endif // _EXCEL_H_