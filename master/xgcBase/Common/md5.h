//! @file md5.h
//! @brief MD5ժҪ�㷨


#ifndef _MD5_H_
#define _MD5_H_
#include "defines.h"
#include "exports.h"

namespace XGC
{
	namespace Encryption
	{
		//! MD5ժҪ
		//! @param data ��������
		//! @param data_len �������ݳ���
		//! @param digest MD5ժҪ
		//!
		//! data �� digest ����ָ��ͬһ������
		COMMON_API xgc_void Md5( const char *data, int data_len, char* digest, bool output_hex = false, bool bCap = true );

		COMMON_API xgc_string md5ToString( char md5[16] );
	} // namepsace utils
}

#endif // _MD5_H_
