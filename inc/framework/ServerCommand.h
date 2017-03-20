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
#ifndef _DEBUG_CMD_H_
#define _DEBUG_CMD_H_

#pragma once
///
/// ���͵�����Ϣ���ͻ��ˣ��ú�����Ҫ��������ʵ��
/// [12/4/2014] create by albert.xu
///

typedef xgc_void (*fnSendMessage)( xgc_lpvoid lpContext, xgc_lpcstr lpMessage );

namespace DebugCommand
{
	struct CommandTable;

	///
	/// �����ڶ����ָ����ڱ�
	/// [12/30/2014] create by albert.xu
	///
	struct CommandEntry
	{
		/// @var ָ��ϵͳ��
		xgc_lpcstr		lpEntryName;
		/// @var ָ��������
		CommandTable*	lpEntryTable;
		/// @var ��������Ļص�
		fnSendMessage	lpEntryPrint;
	};

	///
	/// ��ȡָ����ڱ���Ҫ��������ʵ�֣�����ȷ��INI�ļ���Debug����������
	/// [12/30/2014] create by albert.xu
	///
	const CommandEntry* GetCommandEntry();

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
		xgc_bool( *pfnCmd )( xgc_size, xgc_lpstr const*, const CommandInfo* ); ///< �����н����ص�
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
		/// @var ָ�������� true - ����Զ�����ָ���, false - ��ܲ�����ָ���, �ȴ�ָ���
		xgc_bool bAutoProcess;
	};

	///
	/// ָ���������
	/// [3/22/2015] create by albert.xu
	///
	struct CommandOperator
	{
		/// @var �����Ľ�ɫ
		xgc_lpvoid	user;
		/// @var �����Ľ�ɫ��
		xgc_lpcstr	name;
		/// @var Ȩ�����ʶ
		xgc_uint32	group;
		/// @var ��������Ļص�
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
	///
	xgc_bool ProcessDebugCmd( xgc_lpcstr lpConfName, const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString );

	///
	/// ��ȡ�����б�
	/// [11/11/2014] create by albert.xu
	///
	xgc_void GetAlias( xgc_vector< xgc_string >& Container, xgc_lpcstr lpConfName = xgc_nullptr );

	///
	/// ��ȡ����ӳ��
	/// [11/11/2014] create by albert.xu
	///
	xgc_bool GetAlias( const xgc_string &strName, xgc_lpstr lpValue, xgc_size nSize, xgc_lpcstr lpConfName = xgc_nullptr );

	///
	/// ���ñ���ӳ��
	/// [11/11/2014] create by albert.xu
	///
	xgc_void SetAlias( const xgc_string &strName, xgc_lpcstr lpValue, xgc_lpcstr lpConfName = xgc_nullptr );

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
}

///
/// ��ʼ������ָ��
/// [10/7/2014] create by albert.xu
///
xgc_bool InitDebugCmd( XGC::common::IniFile &ini );

///
/// ����GMָ������
/// [12/4/2014] create by albert.xu
///
xgc_bool ReloadDebugCmd( xgc_lpcstr lpTableName );

///
/// �������ָ���
/// [10/7/2014] create by albert.xu
///
xgc_void FiniDebugCmd();


#endif // _DEBUG_CMD_H_