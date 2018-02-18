/*******************************************************************/
//! \file DebugCmd.h
//! \brief GM指令框架代码
//! 2014/08/08 by Albert.xu
//! 需要注意的事情
//! 该框架要求功能分指令和子指令，每个功能大类分一个CPP文件
//! 例如：
//!		场景功能的部分单独一个文件DebugCmdScene，
//!		角色功能的部分单独一个文件DebugCmdRole
//! 命名规则为DebugCmdXXX
//! XXX为你的功能模块名
/*******************************************************************/
#pragma once

#ifndef _COMMAND_H_
#define _COMMAND_H_
///
/// 发送调试信息到客户端，该函数需要宿主程序实现
/// [12/4/2014] create by albert.xu
///
 
typedef xgc_void (*fnSendMessage)( xgc_lpvoid lpContext, xgc_lpcstr lpMessage );
 
namespace DebugCommand
{
	struct CommandTable;

	///
	/// GM指令
	/// getopt 用法
	/// 单个字符表示选项
	/// 单个字符后紧跟一个冒号，表示该选项后必须跟一个参数，参数紧跟在选项后或用空格隔开
	/// 单个字符后紧跟两个冒号，表示该选项后可以跟一个参数，也可以不跟，参数必须紧跟在选项后，不能以空格隔开。
	/// [8/8/2014] create by albert.xu
	///
	struct CommandInfo
	{
		xgc_lpcstr	lpCmdString;	///< 命令
		xgc_bool( *pfnCmd )(xgc_size, xgc_lpstr const*, const CommandInfo*); ///< 命令行解析回调
		xgc_lpcstr	lpIntroduction;	///< 指令简介
		xgc_lpcstr	lpUsage;		///< 指令帮助

		struct CommandTable* lpSubCommands;	///< 子指令表
	};

	///
	/// 指令表描述符
	/// [3/22/2015] create by albert.xu
	///
	struct CommandTable
	{
		/// @var 指令表首地址
		CommandInfo *pFirst;
		/// @var 指令表尾地址
		CommandInfo *pLast;
	};

	///
	/// 指令操作描述
	/// [3/22/2015] create by albert.xu
	///
	struct CommandOperator
	{
		/// 操作的角色
		xgc_lpvoid	user;
		/// 操作的角色名
		xgc_lpcstr	name;
		/// 指令表
		CommandTable* table;
		/// 回显输出的回调
		fnSendMessage print;
	};

	///
	/// 初始化命令行列表
	/// [10/7/2014] create by albert.xu
	///
	xgc_bool InitCmdTable( CommandTable* lpCmdTable );

	///
	/// GM子指令处理入口
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandExec( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv );

	///
	/// 显示指令表的帮助信息
	/// [8/12/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandHelp( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv );

	///
	/// GM指令处理入口
	/// [8/8/2014] create by albert.xu
	/// @param lpConfName 配置名，与配置文件中Debug节里的配置一一对应
	/// @param lpCurrent 操作角色
	/// @param lpCmdString 指令串
	xgc_bool ProcessDebugCmd( const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString );
		
	///
	/// 发送消息到客户端
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintClient( xgc_lpcstr lpFmt, ... );

	///
	/// 发送指令帮助到客户端
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintUsage( const CommandInfo* pCmdInfo );

	///
	/// \brief 获取当前操作角色
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:07
	///
	xgc_lpvoid GetOperatorUser();
} 

#endif // _COMMAND_H_