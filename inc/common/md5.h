///
/// CopyRight ? 2016 X Studio
/// \file md5.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief MD5 摘要算法
/// 



#ifndef _MD5_H_
#define _MD5_H_
#include "defines.h"
#include "exports.h"

namespace xgc
{
	namespace Encryption
	{
		//! MD5摘要
		//! @param data 输入数据
		//! @param data_len 输入数据长度
		//! @param digest MD5摘要
		//!
		//! data 和 digest 可以指向同一缓冲区
		COMMON_API xgc_void Md5( const char *data, int data_len, char md5[16] );

		///
		/// \brief 获取文件Md5摘要
		///
		/// \author albert.xu
		/// \date 2016/08/04 11:37
		///
		COMMON_API xgc_bool Md5_file( const char *filename, char md5[16] );

		///
		/// \brief 获取MD5码
		///
		/// \author albert.xu
		/// \date 2016/08/04 11:38
		///
		COMMON_API xgc_void Md5_text( char md5[16], char text[33], bool caps = true );
	} // namepsace utils
}

#endif // _MD5_H_
