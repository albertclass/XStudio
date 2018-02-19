///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file command_handle.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// ָ����ļ�ͷ
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _COMMAND_HANDLE_H_
#define _COMMAND_HANDLE_H_

namespace DebugCommand
{
	struct CommandInfo;

	///
	/// \brief ����ָ��
	///
	/// \author albert.xu
	/// \date 2016/03/14 17:32
	///
	void Process( xgc_lpcstr username, case_executer* proc, xgc_lpcstr cmd );

	///
	/// \brief ���½ű�
	///
	/// \author albert.xu
	/// \date 2016/03/14 17:37
	///
	xgc_bool OnCmd_Tips( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo );
}
#endif // _COMMAND_HANDLE_H_