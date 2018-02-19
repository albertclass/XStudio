/*******************************************************************/
//! \file DebugCmd.h
//! \brief GMָ���ܴ���
//! 2014/08/08 by Albert.xu
//! ��Ҫע�������
//! �ÿ��Ҫ���ܷ�ָ�����ָ�ÿ�����ܴ����һ��CPP�ļ�
//! ���磺
//!		�������ܵĲ��ֵ���һ���ļ�DebugCmdScene��
//!		��ɫ���ܵĲ��ֵ���һ���ļ�DebugCmdRole
//! ��������ΪDebugCmdXXX
//! XXXΪ��Ĺ���ģ����
/*******************************************************************/
#pragma once

#ifndef _COMMAND_H_
#define _COMMAND_H_
///
/// ���͵�����Ϣ���ͻ��ˣ��ú�����Ҫ��������ʵ��
/// [12/4/2014] create by albert.xu
///
 
typedef xgc_void (*fnSendMessage)( xgc_lpvoid lpContext, xgc_lpcstr lpMessage );
 
namespace DebugCommand
{
	struct CommandTable;

	///
	/// GMָ��
	/// getopt �÷�
	/// �����ַ���ʾѡ��
	/// �����ַ������һ��ð�ţ���ʾ��ѡ�������һ������������������ѡ�����ÿո����
	/// �����ַ����������ð�ţ���ʾ��ѡ�����Ը�һ��������Ҳ���Բ������������������ѡ��󣬲����Կո������
	/// [8/8/2014] create by albert.xu
	///
	struct CommandInfo
	{
		xgc_lpcstr	lpCmdString;	///< ����
		xgc_bool( *pfnCmd )(xgc_size, xgc_lpstr const*, const CommandInfo*); ///< �����н����ص�
		xgc_lpcstr	lpIntroduction;	///< ָ����
		xgc_lpcstr	lpUsage;		///< ָ�����

		struct CommandTable* lpSubCommands;	///< ��ָ���
	};

	///
	/// ָ���������
	/// [3/22/2015] create by albert.xu
	///
	struct CommandTable
	{
		/// @var ָ����׵�ַ
		CommandInfo *pFirst;
		/// @var ָ���β��ַ
		CommandInfo *pLast;
	};

	///
	/// ָ���������
	/// [3/22/2015] create by albert.xu
	///
	struct CommandOperator
	{
		/// �����Ľ�ɫ
		xgc_lpvoid	user;
		/// �����Ľ�ɫ��
		xgc_lpcstr	name;
		/// ָ���
		CommandTable* table;
		/// ��������Ļص�
		fnSendMessage print;
	};

	///
	/// ��ʼ���������б�
	/// [10/7/2014] create by albert.xu
	///
	xgc_bool InitCmdTable( CommandTable* lpCmdTable );

	///
	/// GM��ָ������
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandExec( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv );

	///
	/// ��ʾָ���İ�����Ϣ
	/// [8/12/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandHelp( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv );

	///
	/// GMָ������
	/// [8/8/2014] create by albert.xu
	/// @param lpConfName ���������������ļ���Debug���������һһ��Ӧ
	/// @param lpCurrent ������ɫ
	/// @param lpCmdString ָ�
	xgc_bool ProcessDebugCmd( const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString );
		
	///
	/// ������Ϣ���ͻ���
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintClient( xgc_lpcstr lpFmt, ... );

	///
	/// ����ָ��������ͻ���
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintUsage( const CommandInfo* pCmdInfo );

	///
	/// \brief ��ȡ��ǰ������ɫ
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:07
	///
	xgc_lpvoid GetOperatorUser();
} 

#endif // _COMMAND_H_