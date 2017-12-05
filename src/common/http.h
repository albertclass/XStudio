///
/// CopyRight ? 2016 X Studio
/// \file http.h
/// \date ���� 2016
///
/// \author albert.xu windxu@126.com
/// \brief ��HTTP��ص�
/// 

#pragma once
#ifndef _HTTP_H_
#define _HTTP_H_

namespace xgc
{
	namespace http
	{
		///
		/// ����URL����
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc::string EnCodeEscapeURL( xgc_lpcstr URL );

		///
		/// ����URL����
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc::string DeCodeEscapeURL( xgc_lpcstr URL );

		///
		/// HTTP�����������������
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc_bool SetWininetMaxConnection( xgc_uint32 nMaxConnection );

		///
		/// ͬ��HTTP����
		/// [7/23/2015] create by albert.xu
		///
		COMMON_API xgc_bool SyncHttpRequest( xgc_lpcstr url, xgc::string &result );
	}
}
#endif // _HTTP_H_