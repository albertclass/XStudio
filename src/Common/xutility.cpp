///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file xutility.cpp
/// \brief 文件简介
/// \author xufeng04
/// \date 二月 2016
///
/// 一些有用的辅助函数
///
///////////////////////////////////////////////////////////////
#include "xutility.h"

namespace xgc
{
	///
	/// \brief 根据给定的值向上获取最近的2的n次方值
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:21
	///
	xgc_size nearest_power( xgc_size val, xgc_size pow )
	{
		static const xgc_uint32 _max_size = std::numeric_limits< xgc_uint32 >::max();

		if( pow > _max_size / 2 )
		{
			return _max_size;
		}
		else
		{
			xgc_size n = val;

			while( n < pow )
				n <<= 1;

			return n;
		}
	}

	xgc_vector<xgc_string> string_split( xgc_lpcstr src, xgc_lpcstr delim )
	{
		xgc_vector<xgc_string> v;
		XGC_ASSERT_RETURN( src, v );

		xgc_lpcstr str = src + strspn( src, delim );;
		while( *str )
		{
			xgc_lpcstr brk = strpbrk( str, delim );
			if( xgc_nullptr == brk )
			{
				v.push_back( str );
				break;
			}

			v.push_back( xgc_string( str, brk ) );
			str = brk + strspn( brk, delim );
		}

		return v;
	}

	xgc_size bin2hex( xgc_lpstr data, xgc_size size, xgc_lpstr out, xgc_size out_size, xgc_size flags )
	{
		XGC_ASSERT_RETURN( data && out && data != out, -1 );

		xgc_int32 width = ( ( flags & 0x00000003 ) + 1 ) * 8;
		xgc_lpstr ascii = (xgc_char*) malloc( ( width + 1 )*sizeof( xgc_char ) );
		XGC_ASSERT_RETURN( ascii, 0 );

		xgc_lpstr ptr = ascii;
		xgc_lpstr p = out;
		xgc_lpstr end = p + out_size;

		xgc_int32 n = sprintf_s( p, (size_t) ( end - p ), "\n" );
		if( n < 0 )
			return 0;
		p += n;
		size_t i = 0;
		for( ; i < size && i < out_size - n; ++i )
		{
			if( p + 3 >= end ) break;
			xgc_byte uch = *( data + i );
			n = sprintf_s( p, (xgc_size) ( end - p ), "%02x ", uch );
			if( n < 0 )
			{
				*( p - 1 ) = 0;
				break;
			}
			if( isprint( uch ) )
			{
				*ascii++ = uch;
			}
			else
			{
				*ascii++ = '.';
			}
			p += n;

			if( ( i + 1 ) % width == 0 )
			{
				*ascii++ = 0;
				ascii -= width + 1;
				if( strlen( ascii ) < xgc_size( end - p ) )
					n = sprintf_s( p, (xgc_size) ( end - p ), "%s\n", ascii );
				else
					n = -1;
				if( n < 0 )
				{
					*( p - 1 ) = 0;
					break;
				}
				p += n;
			}
		}

		n = sprintf_s( p, (xgc_size) ( end - p ), "%*s", XGC_MIN( int( end - p ), ( width - int( i%width ) ) * 3 ), "" );
		if( n < 0 )
		{
			*( p - 1 ) = 0;
		}
		else
		{
			p += n;
			*ascii++ = 0;
			ascii -= ( i + 1 ) % width;
			if( strlen( ascii ) < xgc_size( end - p ) )
				n = sprintf_s( p, (xgc_size) ( end - p ), "%s\n", ascii );
			else
				n = -1;
			if( n < 0 )
			{
				*( p - 1 ) = 0;
			}
			else
			{
				p += n;
			}
		}

		free( ptr );
		return xgc_size( p - out );
	}

	xgc_bool hex2bin( xgc_lpcstr data, xgc_size size, xgc_lpstr out, xgc_size out_size/* = 0*/ )
	{
		xgc_size nHexLen = strlen( data );
		if( nHexLen % 2 != 0 )
		{
			return false;
		}

		if( out_size == 0 )
		{
			out_size = nHexLen / 2;
		}

		if( out_size > nHexLen / 2 )
		{
			return false;
		}

		for( auto i = 0; i < size; ++i )
		{
			if( *data >= '0' && *data <= '9' )
				*out &= *data - '0';
			else if( xgc_char chr = toupper(*data) && chr >='A' && chr <= 'F' )
				*out &= chr - 'A' + 10; //-'A' + 10
			else
				break;

			if( i & 1 )
				++out;
			else
				*out <<= 4;

			++data;
		}
		return true;
	}

}



