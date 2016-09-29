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
	/// 获取执行程序的文件名
	///
	COMMON_API xgc_lpcstr GetModuleName( HMODULE hModule = xgc_nullptr );

	///
	/// [12/16/2013 albert.xu]
	/// 获取执行程序的目录，目录以
	///
	COMMON_API xgc_lpcstr GetModulePath( HMODULE hModule = xgc_nullptr );

	///
	/// [12/16/2013 albert.xu]
	/// 获取指定文件的目录，目录以
	/// @param pszRelativePath 以运行文件为根目录的相对路径
	/// @param SplitWith 目录间的分割符
	/// @return 若文件存在则返回文件的绝对路径，否则返回空指针
	///
	COMMON_API xgc_lpcstr GetNormalPath( xgc_lpstr szOut, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

	///
	/// \brief 获取进程内存使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_bool GetProcessMemoryUsage( xgc_handle h, xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

	///
	/// \brief 获取系统内存使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:22
	///
	COMMON_API xgc_bool GetSystemMemoryUsage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint32 *pnLoadMem );

	///
	/// \brief 获取当前CPU使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_int32 GetProcessCpuUsage( xgc_handle h );

	///
	/// \brief 获取当前系统的CPU使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:09
	///
	COMMON_API xgc_int32 GetSystemCpuUsage();

	///
	/// 获取随机数发生引擎
	/// [12/23/2014] create by albert.xu
	///
	COMMON_API std::mt19937& GetRandomDriver();

	///
	/// 获取随机数
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
	/// \brief 设置进程权限
	///
	/// \author albert.xu
	/// \date 2016/08/10 15:52
	///
	COMMON_API xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable = true );
}
#endif // _XSYSTEM_H_