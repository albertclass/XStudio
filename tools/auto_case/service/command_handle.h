///////////////////////////////////////////////////////////////
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file command_handle.h
/// \brief 文件简介
/// \author xufeng04
/// \date 三月 2016
///
/// 指令处理文件头
///
///////////////////////////////////////////////////////////////
#pragma once

#ifndef _COMMAND_HANDLE_H_
#define _COMMAND_HANDLE_H_

namespace DebugCommand
{
	struct CommandInfo;

	///
	/// \brief 处理指令
	///
	/// \author albert.xu
	/// \date 2016/03/14 17:32
	///
	void Process( xgc_lpcstr username, case_executer* proc, xgc_lpcstr cmd );

	///
	/// \brief 更新脚本
	///
	/// \author albert.xu
	/// \date 2016/03/14 17:37
	///
	xgc_bool OnCmd_Tips( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo );
}
#endif // _COMMAND_HANDLE_H_