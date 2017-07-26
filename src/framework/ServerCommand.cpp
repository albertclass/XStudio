#include "ServerDefines.h"
#include "ServerCommand.h"
#include "ServerBase.h"
#include <sys/stat.h>

#define ECHO_MESSAGE_LENGTH 4096

namespace DebugCommand
{
	///
	/// ��ָ���
	/// [12/29/2014] create by albert.xu
	///
	struct Command
	{
		/// ָ����Ϣ
		struct CommandInfo* lpCmdInfo;
		/// �������ָ���б�
		xgc_unordered_map< xgc_string, Command* > mChildren;
	};

	///
	/// Ȩ�ޱ�
	/// [12/29/2014] create by albert.xu
	///
	struct Group
	{
		xgc_uint32 mLevel;
		xgc_string mName;
		xgc_unordered_map< xgc_string, Command* > mCommands;
	};

	///
	/// ָ�������Ϣ
	/// [12/29/2014] create by albert.xu
	///
	struct CommandInstance
	{
		/// @var ������
		xgc_lpcstr mEntryName;
		/// @var �����ļ�·��
		xgc_char mConfigPath[XGC_MAX_PATH];
		/// @var �������ļ�·��
		xgc_char mScriptPath[XGC_MAX_PATH];
		/// @var Ȩ��ӳ���
		xgc_map< xgc_uint32, Group > mGroupMap;
		/// @var ����ӳ���
		xgc_map< xgc_string, xgc_string > mAliasMap;
		/// @var ���ָ���
		CommandTable *mEntryTable;
		/// @var ��Ϣ����ص�
		fnSendMessage mSendMessage;
	};

	///
	/// ������Ϣ
	/// [12/29/2014] create by zhangyupeng
	///
	struct CommandCache
	{
		xgc_string mCmd;
		xgc_uint32 mUpdateTime;
	};

	/// @var ��ǰ��ɫ
	static CommandOperator const * gpCurrent = xgc_nullptr;
	/// @var ��ǰָ���ʵ��
	static CommandInstance * gpInstance = xgc_nullptr;
}

/// @var ָ�����ӳ��
static xgc_unordered_map< xgc_string, DebugCommand::CommandInstance* > gCommandInstMap;

/// @var ָ����棬��������ȷ������
static xgc_unordered_map< xgc_string, DebugCommand::CommandCache > gCommandInstCache;

///
/// ����ָ���
/// [7/29/2015] create by albert.xu
///
static xgc_void FiniDebugCmd( DebugCommand::Command* pCommands );

///
/// ��ȡһ��ָ��Ȩ������
/// [10/7/2014] create by albert.xu
///
static struct DebugCommand::Command* ReadCommands( DebugCommand::CommandInfo* lpCmdInfo )
{
	struct DebugCommand::Command *pCommands = XGC_NEW DebugCommand::Command { lpCmdInfo };

	if( lpCmdInfo->lpSubCommands )
	{
		auto lpCmdTable = lpCmdInfo->lpSubCommands;
		struct DebugCommand::CommandInfo *pCmdInfo = lpCmdTable->pFirst;

		while( pCmdInfo != lpCmdTable->pLast )
		{
			// ������ָ���
			struct DebugCommand::Command * pSubCommands = ReadCommands( pCmdInfo );
			if( pSubCommands )
			{
				auto ib = pCommands->mChildren.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
				XGC_ASSERT( ib.second );
			}

			++pCmdInfo;
		}
	}

	return pCommands;
}

static struct DebugCommand::Command* ReadCommands( pugi::xml_node &node, DebugCommand::CommandInfo* lpCmdInfo );

/// �������ö�ȡָ���б�
static xgc_bool ReadCommands( pugi::xml_node &node, DebugCommand::CommandTable* lpCmdTable, xgc_unordered_map< xgc_string, DebugCommand::Command* > &Commands )
{
	int nCount = 0;

	for( auto cmd = node.child( "cmd" ); cmd; cmd = cmd.next_sibling( "cmd" ) )
	{
		auto command = cmd.attribute( "name" ).as_string( xgc_nullptr );
		XGC_ASSERT_CONTINUE( command );

		++nCount;

		// ��ָ֤���Ƿ����
		// ���������ָ������ƥ���㷨Ϊͷƥ�䡣
		// ����ֻҪ�к�������ַ���������ǰ��ȫ��ƥ������Ϊƥ�䡣
		DebugCommand::CommandInfo* pCmdInfo = std::lower_bound(
			lpCmdTable->pFirst,
			lpCmdTable->pLast,
			command,
			[]( const DebugCommand::CommandInfo& lhs, xgc_lpcstr lpCmdString ){
			return strcasecmp( lhs.lpCmdString, lpCmdString ) < 0;
		} );

		if( pCmdInfo == lpCmdTable->pLast )
			return false;
		if( strcasecmp( pCmdInfo->lpCmdString, command ) != 0 )
			return false;

		struct DebugCommand::Command* pCommand = ReadCommands( cmd, pCmdInfo );
		if( xgc_nullptr == pCommand )
			return false;

		auto ib = Commands.insert( std::make_pair( pCmdInfo->lpCmdString, pCommand ) );
		XGC_ASSERT( ib.second );
	}

	// ��������ָ����������������ָ��
	if( nCount == 0 && lpCmdTable )
	{
		struct DebugCommand::CommandInfo *pCmdInfo = lpCmdTable->pFirst;
		while( pCmdInfo != lpCmdTable->pLast )
		{
			// ������ָ���
			struct DebugCommand::Command * pSubCommands = ReadCommands( pCmdInfo );
			if( pSubCommands )
			{
				auto ib = Commands.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
				XGC_ASSERT( ib.second );
			}
			++pCmdInfo;
		}
	}

	return true;
}

///
/// ��ȡһ��ָ��Ȩ������
/// [10/7/2014] create by albert.xu
///
static struct DebugCommand::Command* ReadCommands( pugi::xml_node &node, DebugCommand::CommandInfo* lpCmdInfo )
{
	struct DebugCommand::Command *pCommands = XGC_NEW DebugCommand::Command { lpCmdInfo };

	if( lpCmdInfo->lpSubCommands )
	{
		ReadCommands( node, lpCmdInfo->lpSubCommands, pCommands->mChildren );
	}

	return pCommands;
}

///
/// ����ָ���ʵ���е�����
/// [12/30/2014] create by albert.xu
///
static xgc_void FiniDebugCmd( DebugCommand::Command* pCommands )
{
	XGC_ASSERT_RETURN( pCommands, XGC_NONE );
	for( auto &it : pCommands->mChildren )
	{
		FiniDebugCmd( it.second );
	}

	pCommands->mChildren.clear();
	SAFE_DELETE( pCommands );
}

///
/// ����ָ���ʵ���е�����
/// [12/30/2014] create by albert.xu
///
static xgc_void FiniDebugCmd( DebugCommand::CommandInstance* pInst )
{
	XGC_ASSERT_RETURN( pInst, XGC_NONE );
	for( auto &group : pInst->mGroupMap )
	{
		for( auto &it : group.second.mCommands )
			FiniDebugCmd( it.second );
	}

	pInst->mGroupMap.clear();
	pInst->mAliasMap.clear();
}

///
/// ����GMָ������
/// [12/4/2014] create by albert.xu
///
static xgc_bool ReloadDebugCmd( DebugCommand::CommandInstance* pInst )
{
	FiniDebugCmd( pInst );

	pugi::xml_document xml;
	pugi::xml_parse_result result = xml.load_file( pInst->mConfigPath );
	if( !result )
	{
		USR_ERROR( "Load %s failed. error = %s.", pInst->mConfigPath, result.description() );
		return false;
	}

	auto groups = xml.child( "Groups" );
	XGC_ASSERT_RETURN( groups, false );

	for( auto child = groups.first_child(); child; child = child.next_sibling() )
	{
		if( strcmp( child.name(), "Group" ) == 0 )
		{
			auto level = child.attribute( "level" ).as_int();

			auto name = child.attribute( "name" ).as_string( xgc_nullptr );
			XGC_ASSERT_CONTINUE( name );

			DebugCommand::Group group;
			group.mLevel = level;
			group.mName = name;
			if( false == ReadCommands( child, pInst->mEntryTable, group.mCommands ) )
				return false;

			auto ib = pInst->mGroupMap.insert( std::make_pair( level, group ) );
			XGC_ASSERT_MESSAGE( ib.second, "�����ظ���Ȩ�����ʶ %u - %s", level, name );
			if( false == ib.second )
				return false;
		}
		else if( strcmp( child.name(), "Alias" ) == 0 )
		{
			auto name = child.attribute( "name" ).as_string( xgc_nullptr );
			auto exec = child.attribute( "exec" ).as_string( xgc_nullptr );

			if( name && exec )
			{
				pInst->mAliasMap[name] = exec;
			}
		}
	}

	return true;
}

///
/// ��ʼ��ָ���
/// [10/7/2014] create by albert.xu
///
xgc_bool InitDebugCmd( ini_reader &ini, const DebugCommand::CommandEntry* pMainEntry )
{
	FUNCTION_BEGIN;

	// ��������ָ�������
	for( auto pEntry = pMainEntry; pEntry && pEntry->lpEntryName; ++pEntry )
	{
		xgc_lpcstr lpConfigPath = ini.get_item_value( "Debug", pEntry->lpEntryName, xgc_nullptr );
		xgc_lpcstr lpScriptPath = ini.get_item_value( "Debug", "ScriptPath", "" );

		if( !( lpConfigPath && lpConfigPath[0] ) )
		{
			continue;
		}

		// ����ָ���ʵ��
		DebugCommand::CommandInstance *pInstance = XGC_NEW DebugCommand::CommandInstance;

		pInstance->mEntryName = pEntry->lpEntryName;
		pInstance->mEntryTable = pEntry->lpEntryTable;
		pInstance->mSendMessage = pEntry->lpEntryPrint;

		// �˴�ָ�����Բ����������ļ���
		if( xgc_nullptr == get_absolute_path( pInstance->mConfigPath, sizeof( pInstance->mConfigPath ), "%s", lpConfigPath ) )
		{
			SYS_INFO( "%s%s", "����ָ���з�����Ч��·������", lpConfigPath );
			SAFE_DELETE( pInstance );
		}

		// �����������·��
		if( xgc_nullptr == get_absolute_path( pInstance->mScriptPath, sizeof( pInstance->mScriptPath ), "%s", lpScriptPath ) )
		{
			SYS_INFO( "%s%s", "����ָ���з�����Ч��·������", lpScriptPath );
			SAFE_DELETE( pInstance );
		}

		// ����ָ������ã�����Ȩ�ޣ�������
		if( false == ReloadDebugCmd( pInstance ) )
		{
			SYS_INFO( "%s%s", "�������ô���", lpConfigPath );
			SAFE_DELETE( pInstance );
			FiniDebugCmd();
			return false;
		}

		// ��ָ���ͱ�������ӳ��
		auto ib = gCommandInstMap.insert( std::make_pair( pEntry->lpEntryName, pInstance ) );
		XGC_ASSERT( ib.second );
	}

	return true;
	FUNCTION_END;
	return false;
}

///
/// ����GMָ������
/// [12/4/2014] create by albert.xu
///
xgc_bool ReloadDebugCmd( xgc_lpcstr lpTableName )
{
	if( xgc_nullptr == lpTableName )
	{
		for( auto &it : gCommandInstMap )
		{
			if( false == ReloadDebugCmd( it.second ) )
				return false;
		}
		return true;
	}

	auto it = gCommandInstMap.find( lpTableName );
	if( it == gCommandInstMap.end() )
		return false;

	return ReloadDebugCmd( it->second );
}

xgc_void FiniDebugCmd()
{
	for( auto it : gCommandInstMap )
	{
		FiniDebugCmd( it.second );
		SAFE_DELETE( it.second );
	}

	gCommandInstMap.clear();
}

namespace DebugCommand
{
	static xgc_uint16 OnCmd_Authorised( xgc_size argc, xgc_lpstr const *argv )
	{
		auto it1 = gpInstance->mGroupMap.find( gpCurrent->group );
		if( it1 != gpInstance->mGroupMap.end() )
		{
			auto sCommand = argv[0];
			auto iCommand = it1->second.mCommands.find( sCommand );

			if( iCommand == it1->second.mCommands.end() )
				return -1;

			--argc;
			++argv;

			auto pCommand = iCommand->second;

			while( argc && pCommand->mChildren.size() )
			{
				if( argv[0][0] != '-' )
				{
					auto it2 = pCommand->mChildren.find( argv[0] );
					if( it2 == pCommand->mChildren.end() )
						return -1;

					pCommand = it2->second;
				}
				++argv;
				--argc;
			}

			return 0;
		}

		return -1;
	}

	///
	/// ������Ϣ���ͻ���
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool PrintClient( xgc_lpcstr lpFmt, ... )
	{
		if( xgc_nullptr == gpCurrent )
			return false;

		if( xgc_nullptr == gpInstance )
			return false;

		fnSendMessage Print = gpInstance->mSendMessage;

		if( xgc_nullptr != gpCurrent->print )
			Print = gpCurrent->print;

		if( xgc_nullptr == Print )
			return false;

		xgc_char szMessage[4096];
		va_list args;
		va_start( args, lpFmt );
		int cnt = vsnprintf_s( szMessage, sizeof(szMessage) -1, lpFmt, args );
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
	/// ��ʼ�������б��Ա��������
	/// [8/8/2014] create by albert.xu
	///
	xgc_bool InitCmdTable( CommandTable* lpCmdTable )
	{
		std::sort( lpCmdTable->pFirst, lpCmdTable->pLast,
			[]( const CommandInfo& lhs, const CommandInfo& rhs ){
			return strcasecmp( lhs.lpCmdString, rhs.lpCmdString ) < 0;
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
				return strcasecmp( lhs.lpCmdString, lpCmdString ) < 0;
			} );

			// ��֤�Ƿ��ҵ���lower_bound���ص��ڻ��ߴ���
			if( pCmdInfo == lpCmdTable->pLast )
			{
				SYS_ERROR( "��֤�Ƿ��ҵ���lower_bound���ص��ڻ��ߴ��� %s", argv[0] );
				break;
			}

			if( strncasecmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
			{
				// ��ָ���ж�������ģ��������
				if( pCmdInfo->pfnCmd && false == pCmdInfo->pfnCmd( argc, argv, pCmdInfo ) )
				{
					SYS_ERROR( "û���ҵ�ָ���Ӧ��ִ�к��� ���� ִ��ָ��ʧ��: %s", argv[0] );
					return false;
				}

				// ����ָ���ģ�����������ָ��
				if( pCmdInfo->lpSubCommands && pCmdInfo->lpSubCommands->bAutoProcess )
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

		SYS_ERROR( "û���ҵ�ָ���Ӧ��ִ�к���: %s ��ӡhelp:%s", argv[0], lpCmdTable ? "true" : "false" );
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
				return strcasecmp( lhs.lpCmdString, lpCmdString ) < 0;
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

			if( strncasecmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
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
	static xgc_bool ProcessDebugCmd( const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( lpCmdString, false );

		xgc_lpcstr lpCmdEnd = lpCmdString;
		while( *lpCmdEnd && xgc_nullptr == strchr( " ,", *lpCmdEnd ) )
			++lpCmdEnd;

		xgc_char szCmdString[ECHO_MESSAGE_LENGTH] = { 0 };
		// �������
		auto it1 = gpInstance->mAliasMap.find( xgc_string( lpCmdString, lpCmdEnd ) );
		if( it1 != gpInstance->mAliasMap.end() )
		{
			// �ҵ�����ָ��
			sprintf_s( szCmdString, "%s%s", it1->second.c_str(), lpCmdEnd );
		}
		else
		{
			// δ�ҵ�����
			strncpy_s( szCmdString, lpCmdString, XGC_COUNTOF( szCmdString ) - 1 );
		}

		gpCurrent = lpCurrent;

		xgc_lpcstr lpRoleName = gpCurrent->name ? gpCurrent->name : "";

		//��֤�Ƿ�ͨ��
		xgc_bool bConfirmed = true;
		if( strcasecmp( szCmdString, "y" ) == 0 )
		{
			auto iter = gCommandInstCache.find( lpRoleName );
			if( iter == gCommandInstCache.end() )
			{
				PrintClient( "û�л����ָ�" );
				return true;
			}

			if( current_time() - iter->second.mUpdateTime > 15 )
			{
				PrintClient( "ָ��ȷ�ϳ�ʱ������������ָ��", szCmdString );
				gCommandInstCache.erase( iter );
				return true;
			}

			strncpy_s( szCmdString, iter->second.mCmd.c_str(), sizeof( szCmdString ) );

			bConfirmed = true;
		}

		xgc_size	argc = 0;
		xgc_lpstr	argv[64];

		xgc_char szCmdTmpString[ECHO_MESSAGE_LENGTH] = { 0 };
		strcpy_s( szCmdTmpString, szCmdString );
		xgc_lpstr str = szCmdTmpString;
		enum state { new_token, match_end, match_quote, match_quote2 };
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
					else if( !isblank( *str ) && ( isprint( *str ) /*|| _ismbslead( (xgc_byte*) str, (xgc_byte*) str )*/ ) )
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
		} while( argc < XGC_COUNTOF( argv ) && *++str );

		USR_INFO( "��ɫ����ִ��GMָ�� Role = %s GM = %d, Execute = %s",
			lpRoleName,
			gpCurrent->group,
			szCmdString );

		switch( OnCmd_Authorised( argc, argv ) )
		{
			case 0: // ��֤ͨ��
			if( OnCmd_CommandExec( gpInstance->mEntryTable, argc, argv ) )
			{
				PrintClient( "%s", szCmdString );
				PrintClient( "ָ��ִ�гɹ���" );
				gCommandInstCache.erase( lpRoleName );
				return true;
			}
			else
			{
				SYS_ERROR( "OnCmd_CommandExec return false, argc[%u]", argc );
				for( xgc_size i = 0; i < argc && i < 64; ++i )
				{
					SYS_ERROR( "                                argv[%s]", argv[i] );
				}
				return false;
			}
			break;
			default:
			{
				USR_INFO( "ָ��Ȩ��δͨ����֤�� Role = %s GM = %d",
					gpCurrent->name,
					gpCurrent->group );
				return false;
			}
			break;
		}
		return false;
		FUNCTION_END;
		return false;
	}

	///
	/// GMָ������
	/// [8/8/2014] create by albert.xu
	/// @param lpConfName ���������������ļ���Debug���������һһ��Ӧ
	/// @param lpCurrent ������ɫ
	/// @param lpCmdString ָ�
	xgc_bool ProcessDebugCmd( xgc_lpcstr lpConfName, const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( lpCmdString, false );

		auto it0 = gCommandInstMap.find( lpConfName );
		if( it0 == gCommandInstMap.end() )
			return false;

		gpInstance = it0->second;

		XGC_ASSERT_RETURN( gpInstance, false );
		return ProcessDebugCmd( lpCurrent, lpCmdString );
		FUNCTION_END;
		return false;
	}

	xgc_void GetAlias( xgc_vector< xgc_string >& Container, xgc_lpcstr lpConfName )
	{
		FUNCTION_BEGIN;
		if( lpConfName )
		{
			auto it0 = gCommandInstMap.find( lpConfName );
			if( it0 == gCommandInstMap.end() )
				return;

			gpInstance = it0->second;
		}

		if( gpInstance )
		{
			for( auto &it : gpInstance->mAliasMap )
				Container.push_back( it.first + "=" + it.second );
		}
		FUNCTION_END;
	}

	xgc_bool GetAlias( const xgc_string &strName, xgc_lpstr lpValue, xgc_size nSize, xgc_lpcstr lpConfName )
	{
		FUNCTION_BEGIN;
		if( lpConfName )
		{
			auto it0 = gCommandInstMap.find( lpConfName );
			if( it0 == gCommandInstMap.end() )
				return false;

			gpInstance = it0->second;
		}

		if( gpInstance )
		{
			auto it = gpInstance->mAliasMap.find( strName );
			if( it == gpInstance->mAliasMap.end() )
				return false;

			strncpy_s( lpValue, nSize, it->second.c_str(), nSize - 1 );
			return true;
		}
		FUNCTION_END;
		return false;
	}

	xgc_void SetAlias( const xgc_string &strName, xgc_lpcstr lpValue, xgc_lpcstr lpConfName )
	{
		FUNCTION_BEGIN;
		if( lpConfName )
		{
			auto it0 = gCommandInstMap.find( lpConfName );
			if( it0 == gCommandInstMap.end() )
				return;

			gpInstance = it0->second;
		}

		if( gpInstance )
		{
			if( lpValue == xgc_nullptr || lpValue[0] == 0 )
				gpInstance->mAliasMap.erase( strName );
			else
				gpInstance->mAliasMap[strName] = lpValue;
		}
		FUNCTION_END;
	}
}

//////////////////////////////////////////////////////////////////////////
// ��ܵ���ָ������ڿ�ܵ���
//////////////////////////////////////////////////////////////////////////
extern const char* __version__;
namespace DebugCommand
{
	///
	/// �鿴�������汾��
	/// [11/25/2014] create by wuhailin.jerry
	///
	xgc_bool OnCmd_ShowVersion( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo )
	{
		PrintClient( "%s", __version__ );
		return true;
	}

	///
	/// �鿴��������Ϣ
	/// [11/25/2014] create by wuhailin.jerry
	///
	xgc_bool OnCmd_ServerInfo( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo )
	{
		PrintClient( "server name : %s", ServerName() );
		PrintClient( "server code : %s", GetNetworkId() );

		xgc_char szPath[XGC_MAX_PATH] = { 0 };
		ServerConfigFile( szPath, sizeof(szPath) );
		PrintClient( "server path : %s", get_module_path() );
		PrintClient( "server conf : %s", szPath );

		xgc_char szDate[32] = { 0 };
		xgc_char szTime[32] = { 0 };
		PrintClient( "server date : %s", datetime::now( szDate, "%Y-%m-%d" ) );
		PrintClient( "server time : %s", datetime::now( szTime, "%H:%M:%S" ) );
		return true;
	}

	///
	/// ���ú�����ʱ���ĳ�ʱʱ�䣬��ʱ���ӡһ����־ 
	/// [11/25/2014] create by wuhailin.jerry
	///
	xgc_bool OnCmd_SetTimeout( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo )
	{
		if( argc < 2 )
		{
			PrintClient( "Usage : %s", pCmdInfo->lpUsage );
			return true;
		}

		SetDebugerTimeout( str2numeric< xgc_time32 >( argv[1], xgc_nullptr ) );
		return true;
	}

	xgc_bool OnCmd_RunBatch( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo )
	{
		FUNCTION_BEGIN;
		xgc_char szPath[XGC_MAX_PATH] = { 0 };
		if( argc > 1 )
			get_absolute_path( szPath, "%s\\%s", gpInstance->mScriptPath, argv[1] );
		else
			get_absolute_path( szPath, "%s", gpInstance->mScriptPath );

		struct _stat s;
		if( 0 != _stat( szPath, &s ) )
		{
			PrintClient( "��Ч��·�����ļ���! %s", szPath );
			return false;
		}

		PrintClient( "%s", szPath );

		if( s.st_mode & S_IFDIR )
		{
			list_directory( szPath, []( xgc_lpcstr root, xgc_lpcstr relative, xgc_lpcstr fname ){
				if( fname )
					PrintClient( "%s\n", fname );
				else
					PrintClient( "%s\n", relative );

				return true;
			});
		}
		else if( s.st_mode & S_IFREG )
		{
			FILE *fp = xgc_nullptr;
			fopen_s( &fp, szPath, "r" );
			if( xgc_nullptr == fp )
			{
				PrintClient( "����ִ��GMָ��,���ļ�:[%s]ʧ��,dwErr:[%d]", szPath, errno );
				return false;
			}

			xgc_char chBuffer[1024] = { 0 };
			xgc_size nLen = 0;
			while( xgc_nullptr != fgets( chBuffer, sizeof( chBuffer ), fp ) )
			{
				if( chBuffer[0] == 0 )
					continue;

				ProcessDebugCmd( gpCurrent, chBuffer );
			}

			fclose( fp );
		}
		return true;
		FUNCTION_END;
		return false;
	}

	///
	/// []�еĲ���Ϊ��ѡ����
	/// <>�еĲ���Ϊ�û��������
	/// �� role info [-name <param>]������'role'Ϊ���'info'Ϊ���������
	/// '-name <param>'Ϊ��ѡ��������'aaaaa'Ϊ�û���Ҫ����ġ�
	/// �������Ĺ�ϵ��'|'�ָ�� role teleport <scenename> [<posx> <posy>|<teleport>]
	/// ������˳���ϵͨ��Ƕ�ױ�ʶ���� scene info <scenename> [-l <linenumber>] 
	///		��ʾ����ʹ��-l�����ǷǷ��ģ�������scenenameһͬʹ�á�
	///
	static CommandInfo Commands[] =
	{
		{
			"version",
			OnCmd_ShowVersion,
			"�鿴�������汾��",
			"version",
			xgc_nullptr
		},

		{
			"info",
			OnCmd_ServerInfo,
			"�鿴��������Ϣ",
			"info",
			xgc_nullptr
		},

		{
			"timeout",
			OnCmd_SetTimeout,
			"���÷�����������ʱʱ��",
			"timeout <millseconds>",
			xgc_nullptr
		},

		{
			"run",
			OnCmd_RunBatch,
			"��������GMָ��",
			"run <filename>",
			xgc_nullptr
		},

	};

	CommandTable SystemCmdTable = { Commands, Commands + XGC_COUNTOF( Commands ), true };

	static xgc_bool gCmdTableInit = InitCmdTable( &SystemCmdTable );
}