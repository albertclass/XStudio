///
/// CopyRight ? 2016 X Studio
/// \file md5.h
/// \date ���� 2016
///
/// \author albert.xu windxu@126.com
/// \brief MD5 ժҪ�㷨
/// 
#ifndef _MD5_H_
#define _MD5_H_
#include "defines.h"
#include "exports.h"

namespace xgc
{
	namespace encrypt
	{
		//! MD5ժҪ
		//! @param data ��������
		//! @param data_len �������ݳ���
		//! @param digest MD5ժҪ
		//!
		//! data �� digest ����ָ��ͬһ������
		COMMON_API xgc_void md5_data( const char *data, int data_len, char md5[16] );

		///
		/// \brief ��ȡ�ļ�md5ժҪ
		///
		/// \author albert.xu
		/// \date 2016/08/04 11:37
		///
		COMMON_API xgc_bool md5_file( const char *filename, char md5[16] );

		///
		/// \brief ��ȡMD5��
		///
		/// \author albert.xu
		/// \date 2016/08/04 11:38
		///
		COMMON_API xgc_void md5_text( char md5[16], char text[33], bool caps = true );
	} // namepsace utils
}

#endif // _MD5_H_
