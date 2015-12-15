#include "excel.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <map>
#pragma push_macro("_CRT_SECURE_NO_WARNINGS")

namespace XGC
{
	namespace common
	{
		struct excel::impl
		{
			std::map< xgc_string, xgc_int32 >			ColumnMap;
		};

		excel::excel( xgc_void )
			: m_impl( XGC_NEW excel::impl )
			, mRowCount( 0 )
			, mColCount( 0 )
			, mTitleNext( 0 )
			, mBuffer( xgc_nullptr )
		{
			memset( mChunk, 0, sizeof( mChunk ) );
		}

		excel::~excel( xgc_void )
		{
			SAFE_DELETE( m_impl );
			free( mBuffer );
			for each( auto pChunk in mChunk )
				free( pChunk );
		}

		xgc_bool excel::ReadExcelFile( xgc_lpcstr strFileName )
		{
			// 重入清理
			mColCount = 0;
			mRowCount = 0;

			free( mBuffer );
			mBuffer = xgc_nullptr;

			for( auto i = 0; i < XGC_COUNTOF( mChunk ); ++i )
			{
				free( mChunk[i] );
				mChunk[i] = xgc_nullptr;
			}

			// 读取文件数据
			int fd = -1;
			_sopen_s( &fd, strFileName, O_BINARY | O_RDONLY, SH_DENYWR, S_IREAD );
			if( fd == -1 )
			{
				DWORD dwErr = GetLastError();
				return false;
			}

			struct _stat fst;
			_fstat( fd, &fst );

			xgc_size buffer_size = fst.st_size;
			xgc_lpstr buffer = (xgc_lpstr) realloc( mBuffer, fst.st_size + 1 );
			XGC_ASSERT_RETURN( buffer, false );

			mBuffer = buffer;
			int rd = _read( fd, buffer, (unsigned int)buffer_size );
			_close( fd );

			if( rd != buffer_size )
			{
				return false;
			}

			buffer[fst.st_size] = 0;
			for( xgc_size i = 0; i < buffer_size; ++i )
			{
				if( buffer[i] == '\t' ) ++mColCount;
				if( buffer[i] == '\n' )
				{
					++mColCount;
					break;
				}
			}

			AnalyzeFile( buffer, buffer_size );
			return true;
		}

		xgc_lpcstr excel::getCell( xgc_size nRow, xgc_size nCol )
		{
			if( nCol >= mColCount ) return xgc_nullptr;

			xgc_size idx = ( nRow >> 12 );
			if( idx < XGC_COUNTOF( mChunk ) )
			{
				xgc_lpvoid chunk = mChunk[idx];
				if( chunk )
				{
					xgc_size offset = ( nRow & 0x0fff );
					xgc_lpstr* cell = (xgc_lpstr*) chunk;

					return cell[offset*mColCount + nCol];
				}
			}

			return xgc_nullptr;
		}

		xgc_bool excel::setCell( xgc_size nRow, xgc_size nCol, xgc_lpstr pStr )
		{
			XGC_ASSERT_RETURN( nCol < mColCount, false );

			xgc_size idx = ( nRow >> 12 ); // 右移12位 每过4096,重新分配一次内存
			XGC_ASSERT_RETURN( idx < XGC_COUNTOF( mChunk ), false );

			xgc_lpvoid chunk = mChunk[idx];
			if( chunk == xgc_nullptr )
			{
				// 一次分配4096行
				mChunk[idx] = chunk = (xgc_lpvoid) malloc( 0x1000 * sizeof( xgc_lpvoid ) * mColCount );
				if( chunk == xgc_nullptr )
					return false;
				memset( chunk, 0, _msize( chunk ) );
			}
			xgc_size offset = ( nRow & 0x0fff );
			xgc_lpstr* cell = (xgc_lpstr*) chunk;

			cell[offset*mColCount + nCol] = pStr;
			return true;
		}

		xgc_int32 excel::getColByTitle( xgc_lpcstr pTitle )
		{
			for( xgc_uint32 i = 0; i < mColCount; ++i )
			{
				xgc_lpcstr pValue = getCell( 0, mTitleNext % mColCount );
				if( pValue && _stricmp( pTitle, pValue ) == 0 )
					return mTitleNext++ % mColCount;
				++mTitleNext;
			}

			return mColCount;
		}

		xgc_bool excel::AnalyzeFile( xgc_lpstr buffer, xgc_size buffer_size )
		{
			xgc_bool	is_string = false;
			xgc_uint32&	n = mRowCount; // 行号
			xgc_uint32	c = 0; // 当前是第几列
			xgc_uint32	i = 0; // 读取位置

			setCell( n, c, buffer );
			while( i < buffer_size )
			{
				switch( buffer[i] )
				{
					case 0: return true;
					case '\t':
					{
						if( !is_string && i > 0 )
						{
							buffer[i] = 0;
							setCell( n, ++c, buffer + i + 1 );
						}
					}
					break;
					case '\r':
					{
						buffer[i] = 0;
						c = 0;
					}
					break;
					case '\n':
					{
						if( !is_string && i > 0 )
						{
							buffer[i] = 0;
							setCell( ++n, c, buffer + i + 1 );
						}
					}
					break;
					case '"':
					{
						if( !is_string )
						{
							setCell( n, c, buffer + i + 1 );
						}
						buffer[i] = 0;
						is_string = !is_string;
					}
					break;
				}

				if( c >= mColCount )
					return false;
				++i;
			}

			// 字符串未完结,则认为错误
			if( is_string )
				return false;

			if( c + 1 == mColCount )
				++n;

			return true;
		}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, float *pFloatVal, float fDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	float fValue = fDefault;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if (0 != pValue[0])
		//		{
		//			sscanf_s( pValue, "%f", &fValue );
		//		}
		//		
		//		if( pFloatVal )
		//			*pFloatVal = fValue;

		//		return true;
		//	}
		//	else if( pFloatVal )
		//	{
		//		fValue = *pFloatVal = fDefault;
		//	}
		//	return false;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_int32 *pIntVal, xgc_int32 nDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	xgc_int32 nValue = nDefault;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if( 0 != pValue[0] )
		//		{
		//			sscanf_s( pValue, "%d", &nValue );
		//		}

		//		if( pIntVal )
		//			*pIntVal = nValue;

		//		return true;
		//	}
		//	else if( pIntVal )
		//	{
		//		nValue = *pIntVal = nDefault;

		//	}
		//	return false;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, short *pShortVal, short nDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	xgc_int32 nValue = nDefault;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if (0 != pValue[0])
		//		{
		//			sscanf_s( pValue, "%d", &nValue );
		//		}
		//		
		//		if( pShortVal )
		//			*pShortVal = (short) nValue;

		//		return true;
		//	}
		//	else if( pShortVal )
		//	{
		//		nValue = *pShortVal = nDefault;

		//	}
		//	return false;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_uint16 *pVal, xgc_uint16 nDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		xgc_uint32 nValue = nDefault;
		//		if (0 != pValue[0])
		//		{
		//			sscanf_s( pValue, "%u", &nValue );
		//		}
		//		
		//		if( pVal )
		//			*pVal = (xgc_uint16) nValue;

		//		return true;
		//	}
		//	else if( pVal )
		//	{
		//		*pVal = nDefault;
		//	}
		//	return false;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_uint32 *pVal, xgc_uint32 nDefault )
		//{
		//	XGC_ASSERT_RETURN( pVal, false );
		//	xgc_lpcstr pValue = xgc_nullptr;
	
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if( 0 != pValue[0] )
		//		{
		//			nDefault = strtoul( pValue, xgc_nullptr, 10 );
		//		}

		//		if( pVal )
		//			*pVal = nDefault;
		//		
		//		return true;
		//	}
		//	else
		//	{
		//		*pVal = nDefault;
		//	}
		//	return false;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_bool *pxgc_boolVal, xgc_bool bDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	xgc_bool bValue = bDefault;
		//	xgc_int32 nTmpValue = 0;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if (0 != pValue[0])
		//		{
		//			sscanf_s( pValue, "%d", &nTmpValue );

		//			bValue = ( 0 == nTmpValue ) ? FALSE : TRUE;
		//		}
		//		
		//		if( pxgc_boolVal )
		//			*pxgc_boolVal = bValue;
		//	}
		//	else if( pxgc_boolVal )
		//	{
		//		bValue = *pxgc_boolVal = bDefault;
		//	}
		//	return bValue;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_int64 *pVal, xgc_int64 nDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	xgc_int64 nValue = nDefault;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if( 0 != pValue[0] )
		//		{
		//			nValue = strtoll( pValue, xgc_nullptr, 10 );
		//		}

		//		if( pVal )
		//			*pVal = nValue;

		//		return true;
		//	}
		//	else if( pVal )
		//	{
		//		nValue = *pVal = nDefault;

		//	}
		//	return false;
		//}

		//xgc_bool excel::GetElement( xgc_size nLine, xgc_lpcstr lpszTitle, xgc_uint64 *pVal, xgc_uint64 nDefault )
		//{
		//	xgc_lpcstr pValue = xgc_nullptr;
		//	xgc_uint64 nValue = nDefault;
		//	if( GetElement( nLine, lpszTitle, &pValue ) )
		//	{
		//		if( 0 != pValue[0] )
		//		{
		//			nValue = strtoull( pValue, xgc_nullptr, 10 );
		//		}

		//		if( pVal )
		//			*pVal = nValue;

		//		return true;
		//	}
		//	else if( pVal )
		//	{
		//		nValue = *pVal = nDefault;

		//	}
		//	return false;
		//}

		xgc_bool Check( xgc_lpcstr match, xgc_lpcstr check_string )
		{
			xgc_string str = match;
			{
				const std::regex patternValue( "(I|L|F)(\\[|\\()(-?\\d*\\.?\\d+),(-?\\d*\\.?\\d+)(\\]|\\))" );
				std::match_results<xgc_string::const_iterator> result;
				bool valid = std::regex_match( str, result, patternValue );
				if( valid )
				{
					xgc_string str1 = result[1];
					xgc_string str2 = result[2];
					xgc_string str3 = result[3];
					xgc_string str4 = result[4];
					xgc_string str5 = result[5];
					if( str1 == "I" )
					{
						CValueCheck<xgc_int32> cvc( str2 == "[" ? true : false, str5 == "]" ? true : false, atoi( str3.c_str() ), atoi( str4.c_str() ) );
						return cvc.check( check_string );
					}
					else if( str1 == "L" )
					{
						CValueCheck<xgc_int64> cvc( str2 == "[" ? true : false, str5 == "]" ? true : false, atoll( str3.c_str() ), atoll( str4.c_str() ) );
						return cvc.check( check_string );
					}
					else if( str1 == "F" )
					{
						CValueCheck<xgc_real64> cvc( str2 == "[" ? true : false, str5 == "]" ? true : false, atof( str3.c_str() ), atof( str4.c_str() ) );
						return cvc.check( check_string );
					}
					return false;
				}
			}
			{
				const std::regex patternString( "S(.+)" );
				std::match_results<xgc_string::const_iterator> result;
				xgc_bool valid = std::regex_match( str, result, patternString );
				if( valid )
				{
					xgc_string str1 = result[1];
					CStringCheck csc( str1.c_str() );
					return csc.check( check_string );
				}
				return false;
			}
			return false;
		}
	}
}
#pragma pop_macro("_CRT_SECURE_NO_WARNINGS")

