//! @file md5.h
//! @brief MD5摘要算法


#ifndef _MD5_H_
#define _MD5_H_
#include "defines.h"
#include "exports.h"

namespace XGC
{
	namespace Encryption
	{
		//! MD5摘要
		//! @param data 输入数据
		//! @param data_len 输入数据长度
		//! @param digest MD5摘要
		//!
		//! data 和 digest 可以指向同一缓冲区
		COMMON_API xgc_void Md5( const char *data, int data_len, char* digest, bool output_hex = false, bool bCap = true );

		COMMON_API xgc_string md5ToString( char md5[16] );
	} // namepsace utils
}

#endif // _MD5_H_
