#pragma once
#ifndef _XSYSTEM_H_
#define _XSYSTEM_H_

#include "defines.h"
#include "exports.h"

namespace xgc
{
	///
	/// [12/16/2013 albert.xu]
	/// 获取执行程序的文件名
	///
	COMMON_API xgc_lpcstr get_module_name( xgc_bool rebuild = false );

	///
	/// [12/16/2013 albert.xu]
	/// 获取执行程序的目录，目录以
	///
	COMMON_API xgc_lpcstr get_module_path( xgc_bool rebuild = false );

	///
	/// [12/16/2013 albert.xu]
	/// 获取指定文件的目录，目录以
	/// @param pszRelativePath 以运行文件为根目录的相对路径
	/// @param SplitWith 目录间的分割符
	/// @return 若文件存在则返回文件的绝对路径，否则返回空指针
	///
	COMMON_API xgc_lpcstr get_normal_path( xgc_lpstr absolute, xgc_size size, xgc_lpcstr relative, ... );

	///
	/// [12/16/2013 albert.xu]
	/// 获取指定文件的目录，目录以
	/// @param pszRelativePath 以运行文件为根目录的相对路径
	/// @param SplitWith 目录间的分割符
	/// @return 若文件存在则返回文件的绝对路径，否则返回空指针
	///
	COMMON_API xgc_lpcstr get_normal_path_args( xgc_lpstr absolute, xgc_size size, xgc_lpcstr relative, va_list args );

	///
	/// [12/16/2013 albert.xu]
	/// 获取指定文件的目录，目录以
	/// @param pszRelativePath 以运行文件为根目录的相对路径
	/// @param SplitWith 目录间的分割符
	/// @return 若文件存在则返回文件的绝对路径，否则返回空指针
	///
	template< size_t size >
	xgc_lpcstr get_normal_path( xgc_char( &absolute )[size], xgc_lpcstr relative, ... )
	{
		va_list ap;
		va_start( ap, relative );
		auto ret = get_normal_path_args( absolute, size, relative, ap );
		va_end( ap );

		return ret;
	}
	
	///
	/// [12/16/2013 albert.xu]
	/// 获取进程ID
	///
	COMMON_API xgc_ulong get_process_id();

	///
	/// [12/16/2013 albert.xu]
	/// 获取线程ID
	///
	COMMON_API xgc_ulong get_thread_id();

	///
	/// \brief 获取进程内存使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_bool get_process_memory_usage( xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

	///
	/// \brief 获取系统内存使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:22
	///
	COMMON_API xgc_bool get_system_memory_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem );

	///
	/// \brief 获取系统虚拟内存使用情况
	///
	/// \author albert.xu
	/// \date 2017/01/11 10:57
	///
	COMMON_API xgc_bool get_system_virtual_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem );

	///
	/// \brief 获取当前CPU使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_real64 get_process_cpu_usage();

	///
	/// \brief 获取当前系统的CPU使用情况
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:09
	///
	COMMON_API xgc_real64 get_system_cpu_usage();

	///
	/// \brief 设置进程权限
	///
	/// \author albert.xu
	/// \date 2016/08/10 15:52
	///
	COMMON_API xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable = true );
}
#endif // _XSYSTEM_H_