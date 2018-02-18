#include "stdafx.h"
#include "manager.h"
#include "command_handle.h"
#include "command.h"

#include "proto_def.h"
#include "message_handle.h"

using xgc::message::g_session;

namespace DebugCommand
{
	static void SendMessage( xgc_lpvoid lpContext, xgc_lpcstr lpMessage )
	{

	}

	static CommandInfo Commands[] =
	{
		{
			"tips",
			OnCmd_Tips,
			"更新提示信息",
			"tips text",
			xgc_nullptr
		}
	};

	static CommandTable Table = { Commands, Commands + XGC_COUNTOF( Commands ) };

	// 初始化命令表
	static xgc_bool TableInit = InitCmdTable( &Table );

	void Process( xgc_lpcstr username, case_executer * proc, xgc_lpcstr cmd )
	{
		CommandOperator cmd_operator;
		cmd_operator.user = (xgc_lpvoid) proc;
		cmd_operator.name = username;
		cmd_operator.table = &Table;
		cmd_operator.print = SendMessage;

		ProcessDebugCmd( &cmd_operator, cmd );
	}

	xgc_bool OnCmd_Tips( xgc_size argc, xgc_lpstr const * argv, const CommandInfo * pCmdInfo )
	{
		if( argc > 1 )
		{
			ClientStatusNtf stNtf;
			case_executer* pexecuter = (case_executer*) GetOperatorUser();
			stNtf.pid = pexecuter->get_pid();
			stNtf.tid = pexecuter->get_tid();

			stNtf.status = argv[1];

			Send( g_session, stNtf );
		}
		return true;
	}
}