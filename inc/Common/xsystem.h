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
	COMMON_API xgc_lpcstr get_normal_path( xgc_lpstr szOut, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

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
	COMMON_API xgc_bool get_process_memory_usage( xgc_handle h, xgc_uint64 *pnMem, xgc_uint64 *pnVMem );

	///
	/// \brief ��ȡϵͳ�ڴ�ʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:22
	///
	COMMON_API xgc_bool get_system_memory_usage( xgc_uint64 *pnTotalMem, xgc_uint64 *pnUsageMem, xgc_uint32 *pnLoadMem );

	///
	/// \brief ��ȡ��ǰCPUʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 14:55
	///
	COMMON_API xgc_int32 get_process_cpu_usage( xgc_handle h );

	///
	/// \brief ��ȡ��ǰϵͳ��CPUʹ�����
	///
	/// \author albert.xu
	/// \date 2016/08/08 15:09
	///
	COMMON_API xgc_int32 get_system_cpu_usage();

	///
	/// \brief ���ý���Ȩ��
	///
	/// \author albert.xu
	/// \date 2016/08/10 15:52
	///
	COMMON_API xgc_bool privilege( xgc_lpcstr prililege_name, xgc_bool enable = true );
}
#endif // _XSYSTEM_H_