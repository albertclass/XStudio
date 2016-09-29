#ifndef _ENCODEING_H_
#pragma once
namespace xgc
{
	///
	/// \brief 编码方式
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:35
	///
	enum COMMON_API encoding
	{
		/// Auto-detect input encoding using BOM or < / <? detection; use UTF8 if BOM is not found
		encoding_auto,		
		/// UTF8 encoding
		encoding_utf8,		
		/// Little-endian UTF16
		encoding_utf16_le,	
		/// Big-endian UTF16
		encoding_utf16_be,	
		/// UTF16 with native endianness
		encoding_utf16,		
		/// Little-endian UTF32
		encoding_utf32_le,	
		/// Big-endian UTF32
		encoding_utf32_be,	
		/// UTF32 with native endianness
		encoding_utf32,		
		/// Unicode
		encoding_unicode,	
		/// west eq
		encoding_latin1,	
	};

	///
	/// \brief 探测字符串编码类型
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:25
	///
	encoding guess_encoding( xgc_lpvoid *str, xgc_size len );

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

	COMMON_API xgc_int32 mbstoutf8( xgc_lpvoid src, xgc_lpstr dst, xgc_size dst_size );

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

	COMMON_API xgc_int32 utf8tombs( xgc_lpvoid src, xgc_lpstr dst, xgc_size dst_size );

}
#endif // !_ENCODEING_H_