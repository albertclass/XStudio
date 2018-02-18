#include "stdafx.h"
#include "command.h"

#define ECHO_MESSAGE_LENGTH 4096

namespace DebugCommand
{
	///
	/// 子指令表
	/// [12/29/2014] create by albert.xu
	///
	struct Command
	{
		xgc_unordered_map< xgc_string, struct Command* > mChildrens;
	};

	/// 当前操作对象信息
	static CommandOperator const * gpCurrent = xgc_nullptr;

	///
	/// 发送消息到客户端
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintClient( xgc_lpcstr lpFmt, ... )
	{
		if( xgc_nullptr == gpCurrent )
			return false;

		fnSendMessage Print = gpCurrent->print;

		if( xgc_nullptr == Print )
			return false;

		xgc_char szMessage[4096];
		va_list args;
		va_start( args, lpFmt );
		int cnt = _vsnprintf_s( szMessage, sizeof( szMessage ) - 1, lpFmt, args );
		if( cnt > 0 )
			Print( gpCurrent->user, szMessage );
		va_end( args );

		return true;
	}

	///
	/// 发送指令帮助到客户端
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintUsage( const CommandInfo* pCmdInfo )
	{
		bool bRet = true;
		bRet = bRet && PrintClient( "%s", pCmdInfo->lpIntroduction );
		bRet = bRet && PrintClient( "usage : %s", pCmdInfo->lpUsage );
		return bRet;
	}

	///
	/// \brief 获取当前操作角色
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:07
	///
	xgc_lpvoid GetOperatorUser()
	{
		return gpCurrent->user;
	}

	///
	/// 初始化命令行表，对表进行排序。
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool InitCmdTable( CommandTable* lpCmdTable )
	{
		std::sort( lpCmdTable->pFirst, lpCmdTable->pLast,
				   []( const CommandInfo& lhs, const CommandInfo& rhs ){
			return _stricmp( lhs.lpCmdString, rhs.lpCmdString ) < 0;
		} );

		return true;
	}

	///
	/// GM指令处理入口
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandExec( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv )
	{
		while( argc > 0 )
		{
			// 略过指令参数
			if( argv[0][0] == '-' )
			{
				++argv;
				--argc;
				continue;
			}

			// 查找输入的指令，这里的匹配算法为头匹配。
			// 即，只要有和输入的字符串结束符前的全部匹配则认为匹配。
			CommandInfo* pCmdInfo = std::lower_bound( lpCmdTable->pFirst, lpCmdTable->pLast, argv[0],
													  []( const CommandInfo& lhs, xgc_lpcstr lpCmdString ){
				return _stricmp( lhs.lpCmdString, lpCmdString ) < 0;
			} );

			// 验证是否找到，lower_bound返回等于或者大于
			if( pCmdInfo == lpCmdTable->pLast )
				break;

			if( _strnicmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
			{
				// 该指令有额外参数的，则处理参数
				if( pCmdInfo->pfnCmd && true == pCmdInfo->pfnCmd( argc, argv, pCmdInfo ) )
					return true;

				// 有子指令表的，继续处理子指令
				if( pCmdInfo->lpSubCommands )
				{
					++argv;
					--argc;

					lpCmdTable = pCmdInfo->lpSubCommands;
					continue;
				}
				// 没有子指令表的则返回 true
				else
				{
					return true;
				}
			}
			break;
		}

		// 有子指令表，但是没找到指令的，给出最后查找的指令
		if( lpCmdTable )
		{
			OnCmd_CommandHelp( lpCmdTable, 0, xgc_nullptr );
		}

		return false;
	}

	///
	/// 显示帮助
	/// [10/9/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandHelp( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv )
	{
		CommandInfo* pCmdInfo = xgc_nullptr;
		while( argc > 1 )
		{
			pCmdInfo = std::lower_bound( lpCmdTable->pFirst, lpCmdTable->pLast, argv[1],
										 []( const CommandInfo& lhs, xgc_lpcstr lpCmdString ){
				return _stricmp( lhs.lpCmdString, lpCmdString ) < 0;
			} );

			// 验证是否找到，lower_bound返回等于或者大于
			if( pCmdInfo == lpCmdTable->pLast )
			{
				PrintClient( "请求的指令不存在!" );
				break;
			}

			++argv;
			--argc;

			lpCmdTable = pCmdInfo->lpSubCommands;

			if( _strnicmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
			{
				// 没有子指令了，直接显示该语句的帮助
				PrintUsage( pCmdInfo );
				break;
			}
		}

		if( lpCmdTable )
		{
			// 显示所有子指令及参数帮助
			for( CommandInfo* pCmdInfo = lpCmdTable->pFirst; pCmdInfo != lpCmdTable->pLast; ++pCmdInfo )
			{
				PrintClient( "------------------" );
				PrintClient( "%s - %s", pCmdInfo->lpCmdString, pCmdInfo->lpIntroduction );
				PrintClient( "Usage : %s", pCmdInfo->lpUsage );
			}
		}
		return true;
	}

	///
	/// GM指令处理入口
	/// [8/8/2014] create by albert.xu
	/// @param lpConfName 配置名，与配置文件中Debug节里的配置一一对应
	/// @param lpCurrent 操作角色
	/// @param lpCmdString 指令串
	xgc_bool ProcessDebugCmd( const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( lpCmdString, false );

		xgc_lpcstr lpCmdEnd = lpCmdString;
		while( *lpCmdEnd && xgc_nullptr == strchr( " ,", *lpCmdEnd ) )
			++lpCmdEnd;

		gpCurrent = lpCurrent;
		//缓存指令二次确认,不需要验证权限
		xgc_lpcstr lpRoleName = gpCurrent->name ? gpCurrent->name : "";

		xgc_size	argc = 0;
		xgc_lpstr	argv[64];

		xgc_char szTmpString[ECHO_MESSAGE_LENGTH] = { 0 };
		strcpy_s( szTmpString, lpCmdString );
		xgc_lpstr str = szTmpString;
		enum state
		{
			new_token, match_end, match_quote, match_quote2
		};
		state s = new_token;
		do
		{
			switch( s )
			{
			case new_token:
				{
					if( *str == '\'' )
					{
						s = match_quote;
						argv[argc++] = ++str;
					}
					else if( *str == '\"' )
					{
						s = match_quote2;
						argv[argc++] = ++str;
					}
					else if( !isblank( *str ) && (isprint( *str ) || _ismbslead( (xgc_byte*) str, (xgc_byte*) str )) )
					{
						s = match_end;
						argv[argc++] = str;
					}
				}
				break;
			case match_end:
				if( isblank( *str ) )
				{
					*str = 0;
					s = new_token;
				}
				break;
			case match_quote:
				if( *str == '\'' )
				{
					*str = 0;
					s = match_end;
				}
				break;
			case match_quote2:
				if( *str == '\"' )
				{
					*str = 0;
					s = match_end;
				}
				break;
			}
		} while( argc < xgc_countof( argv ) && *++str );

		USR_INFO( "角色请求执行GM指令 Role = %s Execute = %s", lpRoleName, lpCmdString );

		if( OnCmd_CommandExec( lpCurrent->table, argc, argv ) )
		{
			PrintClient( "%s", lpCmdString );
			PrintClient( "指令执行成功！" );

			return true;
		}

		FUNCTION_END;
		return false;
	}
}