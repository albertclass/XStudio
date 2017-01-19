#pragma once
#ifndef _XSYSTEM_H_
#define _XSYSTEM_H_

#include "defines.h"
#include "exports.h"

namespace xgc
{
	///
	/// [12/16/2013 albert.xu]
	/// ��ȡִ�г�����ļ���
	///
	COMMON_API xgc_lpcstr get_module_name( xgc_bool rebuild = false );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡִ�г����Ŀ¼��Ŀ¼��
	///
	COMMON_API xgc_lpcstr get_module_path( xgc_bool rebuild = false );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡָ���ļ���Ŀ¼��Ŀ¼��
	/// @param pszRelativePath �������ļ�Ϊ��Ŀ¼�����·��
	/// @param SplitWith Ŀ¼��ķָ��
	/// @return ���ļ������򷵻��ļ��ľ���·�������򷵻ؿ�ָ��
	///
	COMMON_API xgc_lpcstr get_normal_path( xgc_lpstr absolute, xgc_size size, xgc_lpcstr relative, ... );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡָ���ļ���Ŀ¼��Ŀ¼��
	/// @param pszRelativePath �������ļ�Ϊ��Ŀ¼�����·��
	/// @param SplitWith Ŀ¼��ķָ��
	/// @return ���ļ������򷵻��ļ��ľ���·�������򷵻ؿ�ָ��
	///
	COMMON_API xgc_lpcstr get_normal_path_args( xgc_lpstr absolute, xgc_size size, xgc_lpcstr relative, va_list args );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡָ���ļ���Ŀ¼��Ŀ¼��
	/// @param pszRelativePath �������ļ�Ϊ��Ŀ¼�����·��
	/// @param SplitWith Ŀ¼��ķָ��
	/// @return ���ļ������򷵻��ļ��ľ���·�������򷵻ؿ�ָ��
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
	/// ����Ŀ¼
	///
	COMMON_API xgc_ulong makepath( xgc_lpcstr path, xgc_bool recursion = true );

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡ����ID
	///
	COMMON_API xgc_ulong get_process_id();

	///
	/// [12/16/2013 albert.xu]
	/// ��ȡ�߳�ID
	///
	COMMON_API xgc_ulong get_thread_id();

	///
	/// \brief ��ȡ�����ڴ�ʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_bool get_process_memory_usage( xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

	///
	/// \brief ��ȡϵͳ�ڴ�ʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:22
	///
	COMMON_API xgc_bool get_system_memory_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem );

	///
	/// \brief ��ȡϵͳ�����ڴ�ʹ�����
	///
	/// \author albert.xu
	/// \date 2017/01/11 10:57
	///
	COMMON_API xgc_bool get_system_virtual_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint64 *pnPrivateMem );

	///
	/// \brief ��ȡ��ǰCPUʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_real64 get_process_cpu_usage();

	///
	/// \brief ��ȡ��ǰϵͳ��CPUʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:09
	///
	COMMON_API xgc_real64 get_system_cpu_usage();

	///
	/// \brief ���ý���Ȩ��
	///
	/// \author albert.xu
	/// \date 2016/08/10 15:52
	///
	COMMON_API xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable = true );
}
#endif // _XSYSTEM_H_