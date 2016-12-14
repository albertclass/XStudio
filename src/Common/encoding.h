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
	COMMON_API encoding guess_encoding( xgc_lpvoid *str, xgc_size len );

	///
	/// \brief utf8����ת��Ϊunicode
	///
	/// \param utf8 Ҫת�ı���
	/// \param ucs  �洢Ϊunicode����
	///
	/// \return ת����utf8�����ֽ���
	/// \author albert.xu
	/// \date 2016/10/20 18:23
	///
	COMMON_API xgc_size utf8_to_ucs( xgc_lpcvoid utf8, xgc_ulong *ucs );

	///
	/// \brief unicode����ת��Ϊutf8
	///
	/// \param ucs  ׼��ת����unicode����
	/// \param utf8 utf8���ջ�������ַ
	/// \param size utf8���ջ���������
	///
	/// \return ת����utf8�����ֽ���
	/// \author albert.xu
	/// \date 2016/10/20 18:23
	///
	COMMON_API xgc_size ucs_to_utf8( xgc_ulong ucs, xgc_lpvoid utf8, xgc_size size );

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

	COMMON_API xgc_size mbs_to_utf8( xgc_lpcvoid mbsc, xgc_lpvoid utf8, xgc_size size );

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

	COMMON_API xgc_size utf8_to_mbs( xgc_lpcvoid utf8, xgc_lpvoid mbsc, xgc_size size );

}
#endif // !_ENCODEING_H_