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
#ifndef _DEBUG_CMD_H_
#define _DEBUG_CMD_H_

#pragma once
///
/// 发送调试信息到客户端，该函数需要宿主程序实现
/// [12/4/2014] create by albert.xu
///

typedef xgc_void (*fnSendMessage)( xgc_lpvoid lpContext, xgc_lpcstr lpMessage );

namespace DebugCommand
{
	struct CommandTable;

	///
	/// 程序内定义的指令入口表
	/// [12/30/2014] create by albert.xu
	///
	struct CommandEntry
	{
		/// @var 指令系统名
		xgc_lpcstr		lpEntryName;
		/// @var 指令描述表
		CommandTable*	lpEntryTable;
		/// @var 回显输出的回调
		fnSendMessage	lpEntryPrint;
	};

	///
	/// 获取指令入口表，需要宿主程序实现，用于确定INI文件中Debug节里的配置项。
	/// [12/30/2014] create by albert.xu
	///
	const CommandEntry* GetCommandEntry();

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
		xgc_bool( *pfnCmd )( xgc_size, xgc_lpstr const*, const CommandInfo* ); ///< 命令行解析回调
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
		/// @var 指令表处理规则 true - 框架自动处理指令表, false - 框架不处理指令表, 等待指令处理
		xgc_bool bAutoProcess;
	};

	///
	/// 指令操作描述
	/// [3/22/2015] create by albert.xu
	///
	struct CommandOperator
	{
		/// @var 操作的角色
		xgc_lpvoid	user;
		/// @var 操作的角色名
		xgc_lpcstr	name;
		/// @var 权限组标识
		xgc_uint32	group;
		/// @var 回显输出的回调
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
	///
	xgc_bool ProcessDebugCmd( xgc_lpcstr lpConfName, const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString );

	///
	/// 获取别名列表
	/// [11/11/2014] create by albert.xu
	///
	xgc_void GetAlias( xgc_vector< xgc_string >& Container, xgc_lpcstr lpConfName = xgc_nullptr );

	///
	/// 获取别名映射
	/// [11/11/2014] create by albert.xu
	///
	xgc_bool GetAlias( const xgc_string &strName, xgc_lpstr lpValue, xgc_size nSize, xgc_lpcstr lpConfName = xgc_nullptr );

	///
	/// 设置别名映射
	/// [11/11/2014] create by albert.xu
	///
	xgc_void SetAlias( const xgc_string &strName, xgc_lpcstr lpValue, xgc_lpcstr lpConfName = xgc_nullptr );

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
}

///
/// 初始化调试指令
/// [10/7/2014] create by albert.xu
///
xgc_bool InitDebugCmd( XGC::common::IniFile &ini );

///
/// 重载GM指令配置
/// [12/4/2014] create by albert.xu
///
xgc_bool ReloadDebugCmd( xgc_lpcstr lpTableName );

///
/// 清理调试指令表
/// [10/7/2014] create by albert.xu
///
xgc_void FiniDebugCmd();


#endif // _DEBUG_CMD_H_