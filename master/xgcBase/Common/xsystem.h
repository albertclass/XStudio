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

	//-----------------------------------//
	// [1/21/2014 albert.xu]
	// 获取当前内存情况
	//-----------------------------------//
	COMMON_API xgc_void GetMemoryStatus( xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

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
	/// 编码URL连接
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_string EnCodeEscapeURL( xgc_lpcstr URL );

	///
	/// 解码URL连接
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_string DeCodeEscapeURL( xgc_lpcstr URL );

	///
	/// HTTP请求设置最大连接数
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_bool SetWininetMaxConnection( xgc_uint32 nMaxConnection );

	///
	/// 同步HTTP请求
	/// [7/23/2015] create by albert.xu
	///
	COMMON_API xgc_bool SyncHttpRequest( xgc_lpcstr url, xgc_string &result );
}
#endif // _XSYSTEM_H_