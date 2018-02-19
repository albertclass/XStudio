#include "stdafx.h"
#include "command.h"

#define ECHO_MESSAGE_LENGTH 4096

namespace DebugCommand
{
	///
	/// ��ָ���
	/// [12/29/2014] create by albert.xu
	///
	struct Command
	{
		xgc_unordered_map< xgc_string, struct Command* > mChildrens;
	};

	/// ��ǰ����������Ϣ
	static CommandOperator const * gpCurrent = xgc_nullptr;

	///
	/// ������Ϣ���ͻ���
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
	/// ����ָ��������ͻ���
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
	/// \brief ��ȡ��ǰ������ɫ
	///
	/// \author albert.xu
	/// \date 2016/04/18 15:07
	///
	xgc_lpvoid GetOperatorUser()
	{
		return gpCurrent->user;
	}

	///
	/// ��ʼ�������б��Ա��������
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
	/// GMָ������
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool OnCmd_CommandExec( CommandTable* lpCmdTable, xgc_size argc, xgc_lpstr const *argv )
	{
		while( argc > 0 )
		{
			// �Թ�ָ�����
			if( argv[0][0] == '-' )
			{
				++argv;
				--argc;
				continue;
			}

			// ���������ָ������ƥ���㷨Ϊͷƥ�䡣
			// ����ֻҪ�к�������ַ���������ǰ��ȫ��ƥ������Ϊƥ�䡣
			CommandInfo* pCmdInfo = std::lower_bound( lpCmdTable->pFirst, lpCmdTable->pLast, argv[0],
													  []( const CommandInfo& lhs, xgc_lpcstr lpCmdString ){
				return _stricmp( lhs.lpCmdString, lpCmdString ) < 0;
			} );

			// ��֤�Ƿ��ҵ���lower_bound���ص��ڻ��ߴ���
			if( pCmdInfo == lpCmdTable->pLast )
				break;

			if( _strnicmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
			{
				// ��ָ���ж�������ģ��������
				if( pCmdInfo->pfnCmd && true == pCmdInfo->pfnCmd( argc, argv, pCmdInfo ) )
					return true;

				// ����ָ���ģ�����������ָ��
				if( pCmdInfo->lpSubCommands )
				{
					++argv;
					--argc;

					lpCmdTable = pCmdInfo->lpSubCommands;
					continue;
				}
				// û����ָ�����򷵻� true
				else
				{
					return true;
				}
			}
			break;
		}

		// ����ָ�������û�ҵ�ָ��ģ����������ҵ�ָ��
		if( lpCmdTable )
		{
			OnCmd_CommandHelp( lpCmdTable, 0, xgc_nullptr );
		}

		return false;
	}

	///
	/// ��ʾ����
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

			// ��֤�Ƿ��ҵ���lower_bound���ص��ڻ��ߴ���
			if( pCmdInfo == lpCmdTable->pLast )
			{
				PrintClient( "�����ָ�����!" );
				break;
			}

			++argv;
			--argc;

			lpCmdTable = pCmdInfo->lpSubCommands;

			if( _strnicmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
			{
				// û����ָ���ˣ�ֱ����ʾ�����İ���
				PrintUsage( pCmdInfo );
				break;
			}
		}

		if( lpCmdTable )
		{
			// ��ʾ������ָ���������
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
	/// GMָ������
	/// [8/8/2014] create by albert.xu
	/// @param lpConfName ���������������ļ���Debug���������һһ��Ӧ
	/// @param lpCurrent ������ɫ
	/// @param lpCmdString ָ�
	xgc_bool ProcessDebugCmd( const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( lpCmdString, false );

		xgc_lpcstr lpCmdEnd = lpCmdString;
		while( *lpCmdEnd && xgc_nullptr == strchr( " ,", *lpCmdEnd ) )
			++lpCmdEnd;

		gpCurrent = lpCurrent;
		//����ָ�����ȷ��,����Ҫ��֤Ȩ��
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

		USR_INFO( "��ɫ����ִ��GMָ�� Role = %s Execute = %s", lpRoleName, lpCmdString );

		if( OnCmd_CommandExec( lpCurrent->table, argc, argv ) )
		{
			PrintClient( "%s", lpCmdString );
			PrintClient( "ָ��ִ�гɹ���" );

			return true;
		}

		FUNCTION_END;
		return false;
	}
}