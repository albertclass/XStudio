#pragma once
#ifndef _XSYSTEM_H_
#define _XSYSTEM_H_

#include <random>

#include "defines.h"
#include "exports.h"

#include <json/json.h>

namespace XGC
{
	///
	/// [12/16/2013 albert.xu]
	/// ��ȡִ�г�����ļ���
	///
	COMMON_API xgc_lpcstr GetModuleName( HMODULE hModule = xgc_nullptr );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡִ�г����Ŀ¼��Ŀ¼��
	///
	COMMON_API xgc_lpcstr GetModulePath( HMODULE hModule = xgc_nullptr );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡָ���ļ���Ŀ¼��Ŀ¼��
	/// @param pszRelativePath �������ļ�Ϊ��Ŀ¼�����·��
	/// @param SplitWith Ŀ¼��ķָ��
	/// @return ���ļ������򷵻��ļ��ľ���·�������򷵻ؿ�ָ��
	///
	COMMON_API xgc_lpcstr GetNormalPath( xgc_lpstr szOut, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

	//-----------------------------------//
	// [1/21/2014 albert.xu]
	// ��ȡ��ǰ�ڴ����
	//-----------------------------------//
	COMMON_API xgc_void GetMemoryStatus( xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

	///
	/// ��ȡ�������������
	/// [12/23/2014] create by albert.xu
	///
	COMMON_API std::mt19937& GetRandomDriver();

	///
	/// ��ȡ�����
	/// [9/5/2014] create by albert.xu
	///
	template < class T1, class T2, typename std::enable_if< !std::is_floating_point< T1 >::value && !std::is_floating_point<T2>::value, xgc_bool >::type = true >
	auto RandomRange( T1 Min, T2 Max )->decltype( Min + Max )
	{
		if( Min == Max )
			return Min;

		using NewT = decltype( Min + Max );
		NewT tMin = static_cast<NewT>( Min );
		NewT tMax = static_cast<NewT>( Max );

		if( tMin > tMax )
		{
			std::swap( tMin, tMax );
		}

		tMax -= 1;

		return std::uniform_int_distribution<NewT>( tMin, tMax )( GetRandomDriver() );
	}

	template< class T1, class T2, typename std::enable_if< std::is_floating_point< T1 >::value && std::is_floating_point<T2>::value, xgc_bool >::type = true>
	auto RandomRange( T1 Min, T2 Max )->decltype( Min + Max )
	{
		if( Min == Max )
		{
			return Min;
		}

		using NewT = decltype( Min + Max );
		NewT tMin = static_cast<NewT>( Min );
		NewT tMax = static_cast<NewT>( Max );

		if( tMin > tMax )
		{
			std::swap( tMin, tMax );
		}

		return std::uniform_real_distribution<NewT>( tMin, tMax )( GetRandomDriver() );
	}

	///
	/// ����URL����
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_string EnCodeEscapeURL( xgc_lpcstr URL );

	///
	/// ����URL����
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_string DeCodeEscapeURL( xgc_lpcstr URL );

	///
	/// HTTP�����������������
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_bool SetWininetMaxConnection( xgc_uint32 nMaxConnection );

	///
	/// ͬ��HTTP����
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_bool SyncHttpRequest( xgc_lpcstr url, xgc_string &result );
}
#endif // _XSYSTEM_H_