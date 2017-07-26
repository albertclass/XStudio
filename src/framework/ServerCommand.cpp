#include "ServerDefines.h"
#include "ServerCommand.h"
#include "ServerBase.h"
#include <sys/stat.h>

#define ECHO_MESSAGE_LENGTH 4096

namespace DebugCommand
{
	///
	/// 子指令表
	/// [12/29/2014] create by albert.xu
	///
	struct Command
	{
		/// 指令信息
		struct CommandInfo* lpCmdInfo;
		/// 允许的子指令列表
		xgc_unordered_map< xgc_string, Command* > mChildren;
	};

	///
	/// 权限表
	/// [12/29/2014] create by albert.xu
	///
	struct Group
	{
		xgc_uint32 mLevel;
		xgc_string mName;
		xgc_unordered_map< xgc_string, Command* > mCommands;
	};

	///
	/// 指令入口信息
	/// [12/29/2014] create by albert.xu
	///
	struct CommandInstance
	{
		/// @var 配置名
		xgc_lpcstr mEntryName;
		/// @var 配置文件路径
		xgc_char mConfigPath[XGC_MAX_PATH];
		/// @var 批处理文件路径
		xgc_char mScriptPath[XGC_MAX_PATH];
		/// @var 权限映射表
		xgc_map< xgc_uint32, Group > mGroupMap;
		/// @var 别名映射表
		xgc_map< xgc_string, xgc_string > mAliasMap;
		/// @var 入口指令表
		CommandTable *mEntryTable;
		/// @var 信息输出回调
		fnSendMessage mSendMessage;
	};

	///
	/// 缓存信息
	/// [12/29/2014] create by zhangyupeng
	///
	struct CommandCache
	{
		xgc_string mCmd;
		xgc_uint32 mUpdateTime;
	};

	/// @var 当前角色
	static CommandOperator const * gpCurrent = xgc_nullptr;
	/// @var 当前指令表实例
	static CommandInstance * gpInstance = xgc_nullptr;
}

/// @var 指令表名映射
static xgc_unordered_map< xgc_string, DebugCommand::CommandInstance* > gCommandInstMap;

/// @var 指令表缓存，用来二次确认命令
static xgc_unordered_map< xgc_string, DebugCommand::CommandCache > gCommandInstCache;

///
/// 清理指令表
/// [7/29/2015] create by albert.xu
///
static xgc_void FiniDebugCmd( DebugCommand::Command* pCommands );

///
/// 读取一条指令权限配置
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
			// 存在子指令表
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

/// 根据配置读取指令列表
static xgc_bool ReadCommands( pugi::xml_node &node, DebugCommand::CommandTable* lpCmdTable, xgc_unordered_map< xgc_string, DebugCommand::Command* > &Commands )
{
	int nCount = 0;

	for( auto cmd = node.child( "cmd" ); cmd; cmd = cmd.next_sibling( "cmd" ) )
	{
		auto command = cmd.attribute( "name" ).as_string( xgc_nullptr );
		XGC_ASSERT_CONTINUE( command );

		++nCount;

		// 验证指令是否存在
		// 查找输入的指令，这里的匹配算法为头匹配。
		// 即，只要有和输入的字符串结束符前的全部匹配则认为匹配。
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

	// 不设置子指令，则代表允许所有子指令
	if( nCount == 0 && lpCmdTable )
	{
		struct DebugCommand::CommandInfo *pCmdInfo = lpCmdTable->pFirst;
		while( pCmdInfo != lpCmdTable->pLast )
		{
			// 存在子指令表
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
/// 读取一条指令权限配置
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
/// 清理指令表实例中的数据
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
/// 清理指令表实例中的数据
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
/// 重载GM指令配置
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
			XGC_ASSERT_MESSAGE( ib.second, "发现重复的权限组标识 %u - %s", level, name );
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
/// 初始化指令表
/// [10/7/2014] create by albert.xu
///
xgc_bool InitDebugCmd( ini_reader &ini, const DebugCommand::CommandEntry* pMainEntry )
{
	FUNCTION_BEGIN;

	// 遍历所有指令表配置
	for( auto pEntry = pMainEntry; pEntry && pEntry->lpEntryName; ++pEntry )
	{
		xgc_lpcstr lpConfigPath = ini.get_item_value( "Debug", pEntry->lpEntryName, xgc_nullptr );
		xgc_lpcstr lpScriptPath = ini.get_item_value( "Debug", "ScriptPath", "" );

		if( !( lpConfigPath && lpConfigPath[0] ) )
		{
			continue;
		}

		// 创建指令表实例
		DebugCommand::CommandInstance *pInstance = XGC_NEW DebugCommand::CommandInstance;

		pInstance->mEntryName = pEntry->lpEntryName;
		pInstance->mEntryTable = pEntry->lpEntryTable;
		pInstance->mSendMessage = pEntry->lpEntryPrint;

		// 此处指令表可以不放在配置文件中
		if( xgc_nullptr == get_absolute_path( pInstance->mConfigPath, sizeof( pInstance->mConfigPath ), "%s", lpConfigPath ) )
		{
			SYS_INFO( "%s%s", "调试指令中发现无效的路径配置", lpConfigPath );
			SAFE_DELETE( pInstance );
		}

		// 批处理的运行路径
		if( xgc_nullptr == get_absolute_path( pInstance->mScriptPath, sizeof( pInstance->mScriptPath ), "%s", lpScriptPath ) )
		{
			SYS_INFO( "%s%s", "调试指令中发现无效的路径配置", lpScriptPath );
			SAFE_DELETE( pInstance );
		}

		// 载入指令表配置，包括权限，别名等
		if( false == ReloadDebugCmd( pInstance ) )
		{
			SYS_INFO( "%s%s", "载入配置错误。", lpConfigPath );
			SAFE_DELETE( pInstance );
			FiniDebugCmd();
			return false;
		}

		// 将指令表和表名进行映射
		auto ib = gCommandInstMap.insert( std::make_pair( pEntry->lpEntryName, pInstance ) );
		XGC_ASSERT( ib.second );
	}

	return true;
	FUNCTION_END;
	return false;
}

///
/// 重载GM指令配置
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
	/// 发送消息到客户端
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
	/// 初始化命令行表，对表进行排序。
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
				return strcasecmp( lhs.lpCmdString, lpCmdString ) < 0;
			} );

			// 验证是否找到，lower_bound返回等于或者大于
			if( pCmdInfo == lpCmdTable->pLast )
			{
				SYS_ERROR( "验证是否找到，lower_bound返回等于或者大于 %s", argv[0] );
				break;
			}

			if( strncasecmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
			{
				// 该指令有额外参数的，则处理参数
				if( pCmdInfo->pfnCmd && false == pCmdInfo->pfnCmd( argc, argv, pCmdInfo ) )
				{
					SYS_ERROR( "没有找到指令对应的执行函数 或者 执行指令失败: %s", argv[0] );
					return false;
				}

				// 有子指令表的，继续处理子指令
				if( pCmdInfo->lpSubCommands && pCmdInfo->lpSubCommands->bAutoProcess )
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

		SYS_ERROR( "没有找到指令对应的执行函数: %s 打印help:%s", argv[0], lpCmdTable ? "true" : "false" );
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
				return strcasecmp( lhs.lpCmdString, lpCmdString ) < 0;
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

			if( strncasecmp( argv[0], pCmdInfo->lpCmdString, strlen( argv[0] ) ) == 0 )
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
	static xgc_bool ProcessDebugCmd( const CommandOperator *lpCurrent, xgc_lpcstr lpCmdString )
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( lpCmdString, false );

		xgc_lpcstr lpCmdEnd = lpCmdString;
		while( *lpCmdEnd && xgc_nullptr == strchr( " ,", *lpCmdEnd ) )
			++lpCmdEnd;

		xgc_char szCmdString[ECHO_MESSAGE_LENGTH] = { 0 };
		// 处理别名
		auto it1 = gpInstance->mAliasMap.find( xgc_string( lpCmdString, lpCmdEnd ) );
		if( it1 != gpInstance->mAliasMap.end() )
		{
			// 找到别名指令
			sprintf_s( szCmdString, "%s%s", it1->second.c_str(), lpCmdEnd );
		}
		else
		{
			// 未找到别名
			strncpy_s( szCmdString, lpCmdString, XGC_COUNTOF( szCmdString ) - 1 );
		}

		gpCurrent = lpCurrent;

		xgc_lpcstr lpRoleName = gpCurrent->name ? gpCurrent->name : "";

		//验证是否通过
		xgc_bool bConfirmed = true;
		if( strcasecmp( szCmdString, "y" ) == 0 )
		{
			auto iter = gCommandInstCache.find( lpRoleName );
			if( iter == gCommandInstCache.end() )
			{
				PrintClient( "没有缓存的指令。" );
				return true;
			}

			if( current_time() - iter->second.mUpdateTime > 15 )
			{
				PrintClient( "指令确认超时，请重新输入指令", szCmdString );
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

		USR_INFO( "角色请求执行GM指令 Role = %s GM = %d, Execute = %s",
			lpRoleName,
			gpCurrent->group,
			szCmdString );

		switch( OnCmd_Authorised( argc, argv ) )
		{
			case 0: // 验证通过
			if( OnCmd_CommandExec( gpInstance->mEntryTable, argc, argv ) )
			{
				PrintClient( "%s", szCmdString );
				PrintClient( "指令执行成功！" );
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
				USR_INFO( "指令权限未通过验证。 Role = %s GM = %d",
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
	/// GM指令处理入口
	/// [8/8/2014] create by albert.xu
	/// @param lpConfName 配置名，与配置文件中Debug节里的配置一一对应
	/// @param lpCurrent 操作角色
	/// @param lpCmdString 指令串
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
// 框架调试指令表，用于框架调试
//////////////////////////////////////////////////////////////////////////
extern const char* __version__;
namespace DebugCommand
{
	///
	/// 查看服务器版本号
	/// [11/25/2014] create by wuhailin.jerry
	///
	xgc_bool OnCmd_ShowVersion( xgc_size argc, xgc_lpstr const *argv, const CommandInfo* pCmdInfo )
	{
		PrintClient( "%s", __version__ );
		return true;
	}

	///
	/// 查看服务器信息
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
	/// 设置函数超时检测的超时时间，超时会打印一条日志 
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
			PrintClient( "无效的路径或文件名! %s", szPath );
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
				PrintClient( "批量执行GM指令,打开文件:[%s]失败,dwErr:[%d]", szPath, errno );
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
	/// []中的参数为可选参数
	/// <>中的参数为用户输入参数
	/// 如 role info [-name <param>]，其中'role'为命令，'info'为必填参数，
	/// '-name <param>'为可选参数其中'aaaaa'为用户需要输入的。
	/// 参数间或的关系用'|'分割。如 role teleport <scenename> [<posx> <posy>|<teleport>]
	/// 参数间顺序关系通过嵌套标识。如 scene info <scenename> [-l <linenumber>] 
	///		表示单独使用-l参数是非法的，必须与scenename一同使用。
	///
	static CommandInfo Commands[] =
	{
		{
			"version",
			OnCmd_ShowVersion,
			"查看服务器版本号",
			"version",
			xgc_nullptr
		},

		{
			"info",
			OnCmd_ServerInfo,
			"查看服务器信息",
			"info",
			xgc_nullptr
		},

		{
			"timeout",
			OnCmd_SetTimeout,
			"设置服务器函数超时时间",
			"timeout <millseconds>",
			xgc_nullptr
		},

		{
			"run",
			OnCmd_RunBatch,
			"批量处理GM指令",
			"run <filename>",
			xgc_nullptr
		},

	};

	CommandTable SystemCmdTable = { Commands, Commands + XGC_COUNTOF( Commands ), true };

	static xgc_bool gCmdTableInit = InitCmdTable( &SystemCmdTable );
}