#include "defines.h"
#include "exports.h"
#include "encoding.h"

#pragma warning( disable : 4996 )
namespace xgc
{
	///
	/// \brief 解析BOM
	///
	/// \author albert.xu
	/// \date 2015/12/31 16:58
	///

	static encoding parse_bom( xgc_lpvoid *ptr, xgc_size len )
	{
		xgc_byte bom[4] = { 0, 0, 0, 0 };
		memcpy( bom, *ptr, XGC_MIN( 4, len ) );

		// look for BOM in first few bytes
		if( bom[0] == 0 && bom[1] == 0 && bom[2] == 0xfe && bom[3] == 0xff )
		{
			*ptr = (xgc_lpstr)*ptr + 4;
			return encoding_utf32_be;
		}

		if( bom[0] == 0xff && bom[1] == 0xfe && bom[2] == 0 && bom[3] == 0 )
		{
			*ptr = (xgc_lpstr)*ptr + 4;
			return encoding_utf32_le;
		}

		if( bom[0] == 0xfe && bom[1] == 0xff )
		{
			*ptr = (xgc_lpstr)*ptr + 2;
			return encoding_utf16_be;
		}

		if( bom[0] == 0xff && bom[1] == 0xfe )
		{
			*ptr = (xgc_lpstr)*ptr + 2;
			return encoding_utf16_le;
		}

		if( bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf )
		{
			*ptr = (xgc_lpstr)*ptr + 3;
			return encoding_utf8;
		}

		// no known BOM detected, assume utf8
		return encoding_auto;
	}

	///
	/// \brief 验证是否UTF-8编码
	///
	/// \author albert.xu
	/// \date 2016/01/05 10:20
	///
	static xgc_bool guess_encoding_utf8( xgc_lpvoid ptr, xgc_size len )
	{
		xgc_byte* str = (xgc_byte*) ptr;
		xgc_int32 chk = 0;
		while( str[0] )
		{
			if( str[0] < 0xC0 )
			{
				// 1 byte
				chk = 0;
			}
			else if( str[0] < 0xE0 )
			{
				// 2 bytes
				chk = 1;
			}
			else if( str[0] < 0xF0 )
			{
				// 3 bytes
				chk = 2;
			}
			else if( str[0] < 0xF8 )
			{
				// 4 bytes
				chk = 3;
			}
			else if( str[0] < 0xFC )
			{
				// 5 bytes
				chk = 4;
			}
			else
			{
				// 6 bytes
				chk = 5;
			}

			// 防止超界
			if( str + chk + 1 > (xgc_byte*) ptr + len )
				return false;

			str += 1;
			for( auto i = 0; i < chk; ++i )
			{
				// 0x80 = 1000 0000b
				if( str[i] < 0x80 )
					return false;

				// 0xBF = 1011 1111b
				if( str[i] > 0xBF )
					return false;
			}

			str += chk;
		}

		return true;
	}

	///
	/// \brief 探测字符串编码类型
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:25
	///

	encoding guess_encoding( xgc_lpvoid *str, xgc_size len )
	{
		encoding enc = encoding_auto;

		enc = parse_bom( str, len );

		if( enc != encoding_auto )
			return enc;

		// 编码自动探测
		if( guess_encoding_utf8( *str, len ) )
			return encoding_utf8;

		return encoding_latin1;
	}

	xgc_size utf8_to_ucs( xgc_lpcvoid utf8, xgc_ulong *ucs )
	{
		const xgc_byte* str = (const xgc_byte*)utf8;
		
		if( (str[0] & 0x80) == 0 )
		{
			// 1 byte
			if( ucs ) *ucs = str[0];
			return 1;
		}
		else if( (str[0] & 0xDF) == str[0] )
		{
			// 2 bytes
			if( ucs ) *ucs = ( (str[0] & 0x1f) << 6 ) | ( str[1] & 0x3f );
			return 2;
		}
		else if( (str[0] & 0xEF) == str[0] )
		{
			// 3 bytes
			if( ucs ) *ucs = ( (str[0] & 0x0f) << 12 ) | ( ( str[1] & 0x3f ) << 6 ) | ( str[2] & 0x3f );
			return 3;
		}
		else if( (str[0] & 0xF7) == str[0] )
		{
			// 4 bytes
			if( ucs ) *ucs = ( (str[0] & 0x07) << 18 ) | ( (str[1] & 0x3f) << 12 ) | ( ( str[2] & 0x3f ) << 6 ) | ( str[3] & 0x3f );
			return 4;
		}
		else if( (str[0] & 0xFB) == str[0] )
		{
			// 5 bytes
			if( ucs ) *ucs = ( (str[0] & 0x03) << 24 ) | ( (str[1] & 0x3f) << 18 ) | ( (str[2] & 0x3f) << 12 ) | ( ( str[3] & 0x3f ) << 6 ) | ( str[4] & 0x3f );
			return 5;
		}
		else if( (str[0] & 0xFD) == str[0] )
		{
			// 6 bytes
			if( ucs ) *ucs = ( (str[0] & 0x01) << 30 ) | ( (str[1] & 0x3f) << 24 ) | ( (str[2] & 0x3f) << 18 ) | ( ( str[3] & 0x3f ) << 12 ) | ( ( str[3] & 0x3f ) << 6 ) | ( str[5] & 0x3f );
			return 6;
		}

		return 0;
	}

	xgc_size ucs_to_utf8( xgc_ulong ucs, xgc_lpvoid utf8, xgc_size size )
	{
		xgc_byte *dst = (xgc_byte*)utf8;

		if ( ucs <= 0x0000007F && size >= 1 )  
		{  
			// * U-00000000 - U-0000007F:  0xxxxxxx  
			if( dst )
			{
				dst[0] = (ucs & 0x7F);  
			}
			return 1;  
		}
		else if ( ucs >= 0x00000080 && ucs <= 0x000007FF && size >= 2 )  
		{  
			// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
			if( dst )
			{
				dst[1] = (ucs & 0x3F) | 0x80;
				dst[0] = ((ucs >> 6) & 0x1F) | 0xC0;
			}
			return 2;  
		}  
		else if ( ucs >= 0x00000800 && ucs <= 0x0000FFFF && size >= 3 )  
		{  
			// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
			if( dst )
			{
				dst[2] = (ucs & 0x3F) | 0x80;
				dst[1] = ((ucs >> 6) & 0x3F) | 0x80;
				dst[0] = ((ucs >> 12) & 0x0F) | 0xE0;
			}
			return 3;  
		}  
		else if ( ucs >= 0x00010000 && ucs <= 0x001FFFFF && size >= 4 )  
		{  
			// * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
			if( dst )
			{
				dst[3] = (ucs & 0x3F) | 0x80;
				dst[2] = ((ucs >> 6) & 0x3F) | 0x80;
				dst[1] = ((ucs >> 12) & 0x3F) | 0x80;
				dst[0] = ((ucs >> 18) & 0x07) | 0xF0;
			}
			return 4;  
		}  
		else if ( ucs >= 0x00200000 && ucs <= 0x03FFFFFF && size >= 5 )  
		{  
			// * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
			if( dst )
			{
				dst[4] = (ucs & 0x3F) | 0x80;
				dst[3] = ((ucs >> 6) & 0x3F) | 0x80;
				dst[2] = ((ucs >> 12) & 0x3F) | 0x80;
				dst[1] = ((ucs >> 18) & 0x3F) | 0x80;
				dst[0] = ((ucs >> 24) & 0x03) | 0xF8;
			}
			return 5;  
		}  
		else if ( ucs >= 0x04000000 && ucs <= 0x7FFFFFFF && size >= 6 )  
		{  
			// * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
			if( dst )
			{
				dst[5] = (ucs & 0x3F) | 0x80;
				dst[4] = ((ucs >> 6) & 0x3F) | 0x80;
				dst[3] = ((ucs >> 12) & 0x3F) | 0x80;
				dst[2] = ((ucs >> 18) & 0x3F) | 0x80;
				dst[1] = ((ucs >> 24) & 0x3F) | 0x80;
				dst[0] = ((ucs >> 30) & 0x01) | 0xFC;
			}
			return 6;
		}

		return 0;  
	}

	///
	/// \brief 编码转换为utf8
	///
	/// \param dst 转码后的存储位置
	/// \param dst_size 转码缓冲区长度
	/// \param src 原始串起始位置
	/// \param encoding 原始串编码方式
	///
	/// \return 转码是否成功 -1 - 失败，否则返回目标缓冲的填充长度
	/// \author albert.xu
	/// \date 2015/12/31 12:26
	///

	xgc_size mbs_to_utf8( xgc_lpcvoid mbs, xgc_lpvoid utf8, xgc_size size )
	{
		xgc_wchar ucs;
		xgc_lpstr  poi = (xgc_lpstr )utf8;
		xgc_lpcstr ptr = (xgc_lpcstr)mbs;

		xgc_size cch = 0;
		
		#if defined(_LINUX)
		mbstate_t mbstat;
		memset(&mbstat, 0, sizeof(mbstat));
		#endif

		while( *ptr )
		{
			#if defined(_LINUX)
			int i = mbrtowc( &ucs, ptr, MB_CUR_MAX, &mbstat );
			#else
			int i = mbtowc( &ucs, ptr, MB_CUR_MAX );
			#endif
			
			if( i < 0 )
				return -1;

			ptr += i;

			if( utf8 )
			{
				cch = ucs_to_utf8( ucs, utf8 ? poi : xgc_nullptr, size ? (xgc_lpstr)utf8 + size - poi : 0 );
				if( cch == 0 )
					return -1;
			}
			else
			{
				cch = ucs_to_utf8( ucs, xgc_nullptr, 0 );
				if( cch == 0 )
					return -1;
			}

			poi += cch;
		}

		return poi - (xgc_lpstr)utf8;
	}

	///
	/// \brief 编码转换mbsc
	///
	/// \param dst 转码后的存储位置
	/// \param dst_size 转码缓冲区长度
	/// \param src 原始串起始位置
	/// \param encoding 原始串编码方式
	///
	/// \return 转码是否成功 -1 - 失败，否则返回目标缓冲的填充长度
	/// \author albert.xu
	/// \date 2015/12/31 12:26
	///

	xgc_size utf8_to_mbs( xgc_lpcvoid utf8, xgc_lpvoid mbs, xgc_size size )
	{
		xgc_ulong ucs;
		xgc_lpstr poi = (xgc_lpstr)mbs;
		xgc_byte* str = (xgc_byte*)utf8;

		xgc_size cch = 0;
		xgc_size add = 0;

		xgc_char mbc[32];

		#if defined(_LINUX)
		mbstate_t mbstat;
		mbsinit( &mbstat );
		wcrtomb( xgc_nullptr, L'\0', &mbstat );
		#endif

		while( *str )
		{
			add = utf8_to_ucs( str, &ucs );
			if( add == 0 )
				return -1;

			#if defined(_LINUX)
			cch = wcrtomb( mbc, (xgc_wchar)ucs, &mbstat );
			if( cch == -1 )
				return -1;
			#elif defined(_WINDOWS)
			int ich = 0;
			wctomb_s( &ich, mbs ? mbc : xgc_nullptr, mbs ? MB_CUR_MAX : 0, (xgc_wchar)ucs );
			if( ich == -1 )
				return -1;

			cch = ich;
			#else
			// wctomb not thread safety
			cch = wctomb( mbs ? poi : (xgc_lpstr)mbs, (xgc_wchar)ucs );
			if( cch == -1 )
				return -1;
			#endif

			if( mbs )
				for( int i = 0; i < cch; ++i )
					poi[i] = mbc[i];

			poi += cch;
			str += add;
		}

		return poi - (xgc_lpstr)mbs;
	}

}