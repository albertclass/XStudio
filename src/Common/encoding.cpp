#include "defines.h"
#include "exports.h"
#include "encoding.h"

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

	xgc_int32 mbstoutf8( xgc_lpvoid src, xgc_lpstr dst, xgc_size dst_size )
	{
		xgc_wchar ucs;
		xgc_lpstr poi = dst;
		xgc_lpstr mbs = (xgc_lpstr)src;

		int cch = 0;
		while( *mbs )
		{
			int i = mbtowc( &ucs, mbs, MB_CUR_MAX );
			if( i < 0 )
				return -1;

			mbs += i;

			if( dst )
			{
				cch = ::WideCharToMultiByte( CP_UTF8, 0, &ucs, 1, poi, (int) (dst + dst_size - poi), NULL, NULL );
				if( cch == 0 )
					return -1;
			}
			else
			{
				cch = ::WideCharToMultiByte( CP_UTF8, 0, &ucs, 1, xgc_nullptr, 0, NULL, NULL );
				if( cch == 0 )
					return -1;
			}

			poi += cch;
		}

		return (xgc_int32)(poi - dst);
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

	xgc_int32 utf8tombs( xgc_lpvoid src, xgc_lpstr dst, xgc_size dst_size )
	{
		xgc_wchar ucs;
		xgc_lpstr poi = dst;
		xgc_byte* str = (xgc_byte*)src;

		int cch = 0;
		int chk = 0;

		while( str[0] )
		{
			if( str[0] < 0xC0 )
			{
				// 1 byte
				if( dst )
					*poi = *str;

				poi += 1;
				str += 1;
			}
			else if( str[0] < 0xE0 )
			{
				// 2 bytes
				ucs = 0;
				ucs = ucs | ( (str[0] & 0x1f) << 6 ) | ( str[1] & 0x3f );

				if( dst )
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, poi, (int) (dst + dst_size - poi), NULL, NULL );
					if( cch == 0 )
						return -1;
				}
				else
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, xgc_nullptr, 0, NULL, NULL );
					if( cch == 0 )
						return -1;
				}

				poi += cch;
				str += 2;
			}
			else if( str[0] < 0xF0 )
			{
				// 3 bytes
				ucs = 0;
				ucs = ucs | ( (str[0] & 0x0f) << 12 ) | ( ( str[1] & 0x3f ) << 6 ) | ( str[2] & 0x3f );

				if( dst )
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, poi, (int) (dst + dst_size - poi), NULL, NULL );
					if( cch == 0 )
						return -1;
				}
				else
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, xgc_nullptr, 0, NULL, NULL );
					if( cch == 0 )
						return -1;
				}

				poi += cch;
				str += 3;
			}
			else if( str[0] < 0xF8 )
			{
				// 4 bytes
				ucs = 0;
				ucs = ucs | ( (str[0] & 0x07) << 18 ) | ( (str[0] & 0x3f) << 12 ) | ( ( str[1] & 0x3f ) << 6 ) | ( str[2] & 0x3f );

				if( dst )
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, poi, (int) (dst + dst_size - poi), NULL, NULL );
					if( cch == 0 )
						return -1;
				}
				else
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, xgc_nullptr, 0, NULL, NULL );
					if( cch == 0 )
						return -1;
				}

				poi += cch;
				str += 4;
			}
			else if( str[0] < 0xFC )
			{
				// 4 bytes
				ucs = 0;
				ucs = ucs | ( (str[0] & 0x03) << 24 ) | ( (str[0] & 0x3f) << 18 ) | ( (str[0] & 0x3f) << 12 ) | ( ( str[1] & 0x3f ) << 6 ) | ( str[2] & 0x3f );

				if( dst )
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, poi, (int) (dst + dst_size - poi), NULL, NULL );
					if( cch == 0 )
						return -1;
				}
				else
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, xgc_nullptr, 0, NULL, NULL );
					if( cch == 0 )
						return -1;
				}

				poi += cch;
				str += 4;
			}
			else
			{
				// 4 bytes
				ucs = 0;
				ucs = ucs | ( (str[0] & 0x01) << 18 ) | ( (str[0] & 0x3f) << 18 ) | ( (str[0] & 0x3f) << 12 ) | ( ( str[1] & 0x3f ) << 6 ) | ( str[2] & 0x3f );

				if( dst )
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, poi, (int) (dst + dst_size - poi), NULL, NULL );
					if( cch == 0 )
						return -1;
				}
				else
				{
					cch = ::WideCharToMultiByte( CP_ACP, 0, &ucs, 1, xgc_nullptr, 0, NULL, NULL );
					if( cch == 0 )
						return -1;
				}

				poi += cch;
				str += 4;
			}
		}

		return (xgc_int32)(poi - dst);
	}

}