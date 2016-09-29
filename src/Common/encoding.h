#ifndef _ENCODEING_H_
#pragma once
namespace xgc
{
	///
	/// \brief ���뷽ʽ
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
	/// \brief ̽���ַ�����������
	///
	/// \author albert.xu
	/// \date 2015/12/31 12:25
	///
	encoding guess_encoding( xgc_lpvoid *str, xgc_size len );

	///
	/// \brief ����ת��Ϊutf8
	///
	/// \param dst ת���Ĵ洢λ��
	/// \param dst_size ת�뻺��������
	/// \param src ԭʼ����ʼλ��
	/// \param encoding ԭʼ�����뷽ʽ
	///
	/// \return ת���Ƿ�ɹ� -1 - ʧ�ܣ����򷵻�Ŀ�껺�����䳤��
	/// \author albert.xu
	/// \date 2015/12/31 12:26
	///

	COMMON_API xgc_int32 mbstoutf8( xgc_lpvoid src, xgc_lpstr dst, xgc_size dst_size );

	///
	/// \brief ����ת��mbsc
	///
	/// \param dst ת���Ĵ洢λ��
	/// \param dst_size ת�뻺��������
	/// \param src ԭʼ����ʼλ��
	/// \param encoding ԭʼ�����뷽ʽ
	///
	/// \return ת���Ƿ�ɹ� -1 - ʧ�ܣ����򷵻�Ŀ�껺�����䳤��
	/// \author albert.xu
	/// \date 2015/12/31 12:26
	///

	COMMON_API xgc_int32 utf8tombs( xgc_lpvoid src, xgc_lpstr dst, xgc_size dst_size );

}
#endif // !_ENCODEING_H_