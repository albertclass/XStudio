///////////////////////////////////////////////////////////////
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

	xgc_long string_replace( char *buf, xgc_long len, xgc_lpcstr src, xgc_lpcstr sub, xgc_lpcstr rep )
	{
		xgc_long ilen = len;
		xgc_long inew = 0;
		xgc_long istr = 0;

		xgc_long irep = (xgc_long)strlen( rep );
		xgc_long isub = (xgc_long)strlen( sub );

		auto ptr = strstr( src, sub );
		while( ilen && ptr )
		{
			xgc_long copy = xgc_long( ptr - src );
			copy = XGC_MIN( ilen, copy );
			strncpy_s( buf + inew, ilen, src + istr, copy );
			ilen -= copy;
			inew += copy;
			istr += copy;

			copy = XGC_MIN( ilen, irep );
			strncpy_s( buf + inew, ilen, rep, copy );
			ilen -= copy;
			inew += copy;
			istr += isub;

			ptr = strstr( src + istr, sub );
		}

		return inew + sprintf_s( buf + inew, ilen, "%s", src + istr );
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

	std::mt19937& get_random_driver()
	{
		static std::mt19937 mt( (xgc_int32) time( xgc_nullptr ) );
		return mt;
	}

	xgc_bool string_match( const char *pattern, xgc_size patternLen, const char *string, xgc_size stringLen, int nocase )
	{
		while( patternLen )
		{
			switch( pattern[0] )
			{
			case '*':
				while( pattern[1] == '*' )
				{
					pattern++;
					patternLen--;
				}
				if( patternLen == 1 )
					return 1; /** match */
				while( stringLen )
				{
					if( string_match( pattern + 1, patternLen - 1, string, stringLen, nocase ) )
						return 1; /** match */
					string++;
					stringLen--;
				}
				return 0; /** no match */
				break;
			case '?':
				if( stringLen == 0 )
					return 0; /** no match */
				string++;
				stringLen--;
				break;
			case '[':
				{
					int inot, match;

					pattern++;
					patternLen--;
					inot = pattern[0] == '^';
					if( inot )
					{
						pattern++;
						patternLen--;
					}
					match = 0;
					while( 1 )
					{
						if( pattern[0] == '\\' )
						{
							pattern++;
							patternLen--;
							if( pattern[0] == string[0] )
								match = 1;
						}
						else if( pattern[0] == ']' )
						{
							break;
						}
						else if( patternLen == 0 )
						{
							pattern--;
							patternLen++;
							break;
						}
						else if( pattern[1] == '-' && patternLen >= 3 )
						{
							int start = pattern[0];
							int end = pattern[2];
							int c = string[0];
							if( start > end )
							{
								std::swap( start, end );
							}

							if( nocase )
							{
								start = tolower( start );
								end = tolower( end );
								c = tolower( c );
							}

							pattern += 2;
							patternLen -= 2;
							if( c >= start && c <= end )
								match = 1;
						}
						else
						{
							if( !nocase )
							{
								if( pattern[0] == string[0] )
									match = 1;
							}
							else
							{
								if( tolower( (int) pattern[0] ) == tolower( (int) string[0] ) )
									match = 1;
							}
						}
						pattern++;
						patternLen--;
					}
					if( inot )
						match = !match;
					if( !match )
						return 0; /** no match */
					string++;
					stringLen--;
					break;
				}
			case '\\':
				if( patternLen >= 2 )
				{
					pattern++;
					patternLen--;
				}
				/** fall through */
			default:
				if( !nocase )
				{
					if( pattern[0] != string[0] )
						return 0; /** no match */
				}
				else
				{
					if( tolower( (int) pattern[0] ) != tolower( (int) string[0] ) )
						return 0; /** no match */
				}
				string++;
				stringLen--;
				break;
			}
			pattern++;
			patternLen--;
			if( stringLen == 0 )
			{
				while( *pattern == '*' )
				{
					pattern++;
					patternLen--;
				}
				break;
			}
		}
		
		return patternLen == 0 && stringLen == 0;
	}

	xgc_bool string_match( const char *pattern, const char *string, int nocase )
	{
		return string_match( pattern, strlen( pattern ), string, strlen( string ), nocase );
	}

	xgc_size string_trim_left( xgc_lpstr str, xgc_lpcstr controls )
	{
		XGC_ASSERT_RETURN( str, -1 );

		auto n = strspn( str, controls );

		auto i = 0U;
		
		if( n > 0 )
		{
			for( ; str[n]; ++i, ++n )
				str[i] = str[n];

			str[i] = 0;
		}

		return i;
	}

	xgc_size string_trim_right( xgc_lpstr str, xgc_lpcstr controls )
	{
		XGC_ASSERT_RETURN( str, -1 );

		xgc_size n = strlen(str);

		while( n )
		{
			if( strchr( controls, str[n] ) )
				str[n] = 0;
			else
				break;

			--n;
		};

		return n;
	}

	xgc_size string_trim_all( xgc_lpstr str, xgc_lpcstr controls )
	{
		XGC_ASSERT_RETURN( str, -1 );

		xgc_size i = 0, n = 0;
		while( str[i] )
		{
			str[n] = str[i];

			if( xgc_nullptr == strchr(controls, str[i] ) )
				++n;

			++i;
		}

		return i;
	}
}



