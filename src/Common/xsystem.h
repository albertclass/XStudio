#pragma once
#ifndef _XSYSTEM_H_
#define _XSYSTEM_H_

#include "defines.h"
#include "exports.h"

#include <random>

namespace xgc
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

	///
	/// \brief ��ȡ�����ڴ�ʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_bool GetProcessMemoryUsage( xgc_handle h, xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

	///
	/// \brief ��ȡϵͳ�ڴ�ʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:22
	///
	COMMON_API xgc_bool GetSystemMemoryUsage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint32 *pnLoadMem );

	///
	/// \brief ��ȡ��ǰCPUʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_int32 GetProcessCpuUsage( xgc_handle h );

	///
	/// \brief ��ȡ��ǰϵͳ��CPUʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:09
	///
	COMMON_API xgc_int32 GetSystemCpuUsage();

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

		if( tMin > tMax )
			std::swap( tMin, tMax );

		return std::uniform_int_distribution<NewT>( Min, Max )( GetRandomDriver() );
	}

	template< class T1, class T2, typename std::enable_if< std::is_floating_point< T1 >::value && std::is_floating_point<T2>::value, xgc_bool >::type = true>
	auto RandomRange( T1 Min, T2 Max )->decltype( Min + Max )
	{
		if( Min == Max ) 
			return Min;

		using NewT = decltype( Min + Max );

		if( tMin > tMax ) 
			std::swap( tMin, tMax );

		return std::uniform_real_distribution<NewT>( Min, std::nextafter( Max, std::numeric_limits<NewT>::max() ) )(GetRandomDriver());
	}

	///
	/// \brief ���ý���Ȩ��
	///
	/// \author albert.xu
	/// \date 2016/08/10 15:52
	///
	COMMON_API xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable = true );
}
#endif // _XSYSTEM_H_