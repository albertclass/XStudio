///
/// CopyRight ? 2016 X Studio
/// \file http.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief 与HTTP相关的
/// 

#pragma once
#ifndef _HTTP_H_
#define _HTTP_H_

namespace xgc
{
	namespace http
	{
		///
		/// 编码URL连接
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc::string EnCodeEscapeURL( xgc_lpcstr URL );

		///
		/// 解码URL连接
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc::string DeCodeEscapeURL( xgc_lpcstr URL );

		///
		/// HTTP请求设置最大连接数
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc_bool SetWininetMaxConnection( xgc_uint32 nMaxConnection );

		///
		/// 同步HTTP请求
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc_bool SyncHttpRequest( xgc_lpcstr url, xgc::string &result );
	}
}
#endif // _HTTP_H_