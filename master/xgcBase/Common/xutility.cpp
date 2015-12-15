#include "xutility.h"

namespace XGC
{
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

	//////////////////////////////////////////////////////////////////////////
	/// ��CharתΪ16�����ַ���
	//////////////////////////////////////////////////////////////////////////
	xgc_void Char2Hex( xgc_byte ch, xgc_lpstr szHex )
	{
		static xgc_byte saucHex[] = "0123456789ABCDEF";
		szHex[0] = saucHex[ch >> 4];
		szHex[1] = saucHex[ch & 0xF];
		szHex[2] = 0;
	}

	//--------------------------------------------------------------------------------------------------------//
	// D2Hex
	// pData		:	������ַ
	// nDataSize	:	���ݳ��ȣ����ַ��ƣ�
	// pOut			:	���������ַ
	// nOutSize		:	���������ȣ����ַ��ƣ�
	// nFlags		:	��־ ��δ���壩
	// return		:	����ת���˶����ַ�
	//--------------------------------------------------------------------------------------------------------//
	xgc_size Binary2Hex( xgc_byte *data, xgc_size size, xgc_lpstr out, xgc_size osize, xgc_size flags )
	{
		if( data == NULL ) return -1;
		if( out == NULL ) return -1;
		if( (xgc_void*) data == (xgc_void*) out ) return -1;

		xgc_int32 width = ( ( flags & 0x00000003 ) + 1 ) * 8;
		xgc_lpstr ascii = (xgc_char*) malloc( ( width + 1 )*sizeof( xgc_char ) );
		XGC_ASSERT_RETURN( ascii, 0 );

		xgc_lpstr ptr = ascii;
		xgc_lpstr p = out;
		xgc_lpstr end = p + osize;

		xgc_int32 n = sprintf_s( p, (size_t) ( end - p ), "\n" );
		if( n < 0 )
			return 0;
		p += n;
		size_t i = 0;
		for( ; i < size && i < osize - n; ++i )
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

	//////////////////////////////////////////////////////////////////////////
	/// ��16�����ַ�תΪChar
	//////////////////////////////////////////////////////////////////////////
	xgc_bool Hex2Char( xgc_lpcstr szHex, xgc_byte& rch )
	{
		if( *szHex >= '0' && *szHex <= '9' )
			rch = *szHex - '0';
		else if( *szHex >= 'A' && *szHex <= 'F' )
			rch = *szHex - 55; //-'A' + 10
		else
			return false;
		szHex++;
		if( *szHex >= '0' && *szHex <= '9' )
			( rch <<= 4 ) += *szHex - '0';
		else if( *szHex >= 'A' && *szHex <= 'F' )
			( rch <<= 4 ) += *szHex - 55; //-'A' + 10;
		else
			return false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	/// Method       : Hex2Binary
	/// Description  : ��16�����ַ���תΪchar����
	/// FullName     : Hex2Binary
	/// Access       : public
	/// Returns      : bool
	/// Qualifier    :
	/// Parameter    : char const* pszHexStr ��ת����16�����ַ���
	/// Parameter    : unsigned char* pucBinStr ��ת�����
	/// Parameter    : int iBinSize ϣ��ת����λ��
	//////////////////////////////////////////////////////////////////////////
	xgc_bool Hex2Binary( xgc_lpcstr pszHexStr, xgc_byteptr pucBinStr, xgc_size iBinSize/* = 0*/ )
	{
		xgc_size nHexLen = strlen( pszHexStr );
		if( nHexLen % 2 != 0 )
		{
			return false;
		}

		if( iBinSize == 0 )
		{
			iBinSize = nHexLen / 2;
		}

		if( iBinSize > nHexLen / 2 )
		{
			return false;
		}

		xgc_byte ch;
		for( xgc_size i = 0; i < iBinSize; i++, pszHexStr += 2, pucBinStr++ )
		{
			if( false == Hex2Char( pszHexStr, ch ) )
				return false;
			*pucBinStr = ch;
		}
		return true;
	}

}



