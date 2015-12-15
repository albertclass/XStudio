#include "stdafx.h"
#include "ServerCommand.h"
#include "protoevent.h"
#include "ServerBase.h"
#include <sys/stat.h>

#define ECHO_MESSAGE_LENGTH 4096
extern xgc_lpcstr GetConfPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

namespace DebugCommand
{
	///
	/// ��ָ���
	/// [12/29/2014] create by albert.xu
	///
	struct Command
	{
		xgc_bool mConfirm;
		xgc_map < xgc_string, struct Command*, string_case_less > mChildrens;
	};

	///
	/// Ȩ�ޱ�
	/// [12/29/2014] create by albert.xu
	///
	struct Group
	{
		xgc_uint32 mLevel;
		xgc_string mName;
		struct Command *mCommands;
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
		xgc_char mConfigPath[_MAX_PATH];
		/// @var �������ļ�·��
		xgc_char mScriptPath[_MAX_PATH];
		/// @var Ȩ��ӳ���
		xgc_map< xgc_uint32, Group > mGroupMap;
		/// @var ����ӳ���
		xgc_map< xgc_string, xgc_string, string_case_less >	mAliasMap;
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
static struct DebugCommand::Command* ReadAllCommands( DebugCommand::CommandTable* lpCmdTable, xgc_unordered_map< xgc_string, xgc_bool >& mapConfirm )
{
	struct DebugCommand::Command *pCommands = XGC_NEW DebugCommand::Command;
	struct DebugCommand::CommandInfo *pCmdInfo = lpCmdTable->pFirst;
	pCommands->mConfirm = false;
	while( pCmdInfo != lpCmdTable->pLast )
	{
		if( pCmdInfo->lpSubCommands )
		{
			// ������ָ���
			struct DebugCommand::Command * pSubCommands = ReadAllCommands( pCmdInfo->lpSubCommands, mapConfirm );
			auto ib = pCommands->mChildrens.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
			XGC_ASSERT_MESSAGE( ib.second, "�����ظ���ָ���� %s", pCmdInfo->lpCmdString );
			if( false == ib.second )
			{
				FiniDebugCmd( pSubCommands );
			}
		}
		else
		{
			// ��������ָ���
			struct DebugCommand::Command *pSubCommands = XGC_NEW DebugCommand::Command;
			//ֻ����ָ��Ż����ȷ���ж�
			auto iter = mapConfirm.find( pCmdInfo->lpCmdString );
			pSubCommands->mConfirm = ( iter != mapConfirm.end() );
			auto ib = pCommands->mChildrens.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
			XGC_ASSERT_MESSAGE( ib.second, "�����ظ���ָ���� %s", pCmdInfo->lpCmdString );
			if( false == ib.second )
			{
				FiniDebugCmd( pSubCommands );
			}
		}
		++pCmdInfo;
	}

	return pCommands;
}

///
/// ��ȡһ��ָ��Ȩ������
/// [10/7/2014] create by albert.xu
///
static struct DebugCommand::Command* ReadCommands( CMarkupSTL &xml, DebugCommand::CommandTable* lpCmdTable, xgc_uint32 nLayer, xgc_unordered_map< xgc_string, xgc_bool >& mapConfirm )
{
	if( false == xml.IntoElem() )
		return xgc_nullptr;

	struct DebugCommand::Command *pCommands = XGC_NEW DebugCommand::Command;
	pCommands->mConfirm = false;
	if( xml.FindElem( "cmd" ) )
	{
		do
		{
			xgc_string strCmd = xml.GetAttrib( "name" );
			// ��ָ֤���Ƿ����
			// ���������ָ������ƥ���㷨Ϊͷƥ�䡣
			// ����ֻҪ�к�������ַ���������ǰ��ȫ��ƥ������Ϊƥ�䡣
			DebugCommand::CommandInfo* pCmdInfo = std::lower_bound( lpCmdTable->pFirst, lpCmdTable->pLast, strCmd.c_str(),
				[]( const DebugCommand::CommandInfo& lhs, xgc_lpcstr lpCmdString ){
				return _stricmp( lhs.lpCmdString, lpCmdString ) < 0;
			} );

			if( pCmdInfo != lpCmdTable->pLast && _stricmp( pCmdInfo->lpCmdString, strCmd.c_str() ) == 0 )
			{
				if( pCmdInfo->lpSubCommands )
				{
					// ������ָ���
					struct DebugCommand::Command * pSubCommands = ReadCommands( xml, pCmdInfo->lpSubCommands, nLayer + 1, mapConfirm );
					auto ib = pCommands->mChildrens.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
					XGC_ASSERT_MESSAGE( ib.second, "�����ظ���ָ���� %s", pCmdInfo->lpCmdString );
					if( false == ib.second )
					{
						FiniDebugCmd( pSubCommands );
					}
				}
				else
				{
					// ��������ָ���
					struct DebugCommand::Command *pSubCommands = XGC_NEW DebugCommand::Command;
					//ֻ����ָ��Ż����ȷ���ж�
					auto iter = mapConfirm.find( pCmdInfo->lpCmdString );
					pSubCommands->mConfirm = ( iter != mapConfirm.end() );
					auto ib = pCommands->mChildrens.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
					XGC_ASSERT_MESSAGE( ib.second, "�����ظ���ָ���� %s", pCmdInfo->lpCmdString );
					if( false == ib.second )
					{
						FiniDebugCmd( pSubCommands );
					}
				}
			}
		} while( xml.FindElem( "cmd" ) );
	}
	else if( nLayer && lpCmdTable )
	{
		struct DebugCommand::CommandInfo *pCmdInfo = lpCmdTable->pFirst;
		while( pCmdInfo != lpCmdTable->pLast )
		{
			if( pCmdInfo->lpSubCommands )
			{
				// ������ָ���
				struct DebugCommand::Command * pSubCommands = ReadAllCommands( pCmdInfo->lpSubCommands, mapConfirm );
				auto ib = pCommands->mChildrens.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
				XGC_ASSERT_MESSAGE( ib.second, "�����ظ���ָ���� %s", pCmdInfo->lpCmdString );
				if( false == ib.second )
				{
					FiniDebugCmd( pSubCommands );
				}
			}
			else
			{
				// ��������ָ���
				struct DebugCommand::Command *pSubCommands = XGC_NEW DebugCommand::Command;
				//ֻ����ָ��Ż����ȷ���ж�
				auto iter = mapConfirm.find( pCmdInfo->lpCmdString );
				pSubCommands->mConfirm = ( iter != mapConfirm.end() );
				auto ib = pCommands->mChildrens.insert( std::make_pair( pCmdInfo->lpCmdString, pSubCommands ) );
				XGC_ASSERT_MESSAGE( ib.second, "�����ظ���ָ���� %s", pCmdInfo->lpCmdString );
				if( false == ib.second )
				{
					FiniDebugCmd( pSubCommands );
				}

			}
			++pCmdInfo;
		}
	}
	xml.OutOfElem();
	return pCommands;
}

///
/// ����ָ���ʵ���е�����
/// [12/30/2014] create by albert.xu
///
static xgc_void FiniDebugCmd( DebugCommand::Command* pCommands )
{
	XGC_ASSERT_RETURN( pCommands, XGC_NONE );
	for each( auto &it in pCommands->mChildrens )
	{
		FiniDebugCmd( it.second );
	}

	pCommands->mChildrens.clear();
	SAFE_DELETE( pCommands );
}

///
/// ����ָ���ʵ���е�����
/// [12/30/2014] create by albert.xu
///
static xgc_void FiniDebugCmd( DebugCommand::CommandInstance* pInst )
{
	XGC_ASSERT_RETURN( pInst, XGC_NONE );
	for each( auto it in pInst->mGroupMap )
	{
		FiniDebugCmd( it.second.mCommands );
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

	CMarkupSTL xml;
	if( !xml.Load( pInst->mConfigPath ) )
	{
		USR_ERROR( "Load %s failed. error = %s.", pInst->mConfigPath, xml.GetError().c_str() );
		return false;
	}

	xgc_bool bRet = xml.FindElem( "Groups" );
	XGC_ASSERT_RETURN( bRet, false );

	xml.IntoElem();

	//��ȡ����ȷ��GMָ���
	xgc_unordered_map< xgc_string, xgc_bool > mapConfirm;
	while( xml.FindElem( "NeedConfirm" ) )
	{
		xgc_string strName = xml.GetAttrib( "name" );
		mapConfirm.insert( make_pair( strName, true ) );
	}

	while( xml.FindNode( CMarkupSTL::MNT_ELEMENT ) )
	{
		if( xml.GetTagName() == "Group" )
		{
			xgc_uint32 nLevel = 0;
			bRet = GetXMLAttribInteger( xml, "level", nLevel );
			XGC_ASSERT_CONTINUE( bRet );

			xgc_char szName[64] = { 0 };
			bRet = GetXMLAttribString( xml, "name", szName, sizeof( szName ) );
			XGC_ASSERT_CONTINUE( bRet );

			DebugCommand::Group group;
			group.mLevel = nLevel;
			group.mName = szName;
			group.mCommands = ReadCommands( xml, pInst->mEntryTable, 0, mapConfirm );

			auto ib = pInst->mGroupMap.insert( std::make_pair( nLevel, group ) );
			XGC_ASSERT_MESSAGE( ib.second, "�����ظ���Ȩ�����ʶ %u - %s", nLevel, szName );
			if( false == ib.second )
			{
				FiniDebugCmd( group.mCommands );
			}
		}
		else if( xml.GetTagName() == "Alias" )
		{
			xgc_string strName = xml.GetAttrib( "name" );
			xgc_string strCmd = xml.GetAttrib( "cmd" );

			if( !strName.empty() && !strCmd.empty() )
				pInst->mAliasMap[strName] = strCmd;
		}
	}

	xml.OutOfElem();
	return true;
}

///
/// ��ʼ��ָ���
/// [10/7/2014] create by albert.xu
///
xgc_bool InitDebugCmd( IniFile &ini )
{
	FUNCTION_BEGIN;

	// ��������ָ�������
	for( auto pEntry = DebugCommand::GetCommandEntry(); pEntry && pEntry->lpEntryName; ++pEntry )
	{
		xgc_lpcstr lpConfigPath = ini.GetItemValue( "Debug", pEntry->lpEntryName, xgc_nullptr );
		xgc_lpcstr lpScriptPath = ini.GetItemValue( "Debug", "ScriptPath", "" );

		if( !( lpConfigPath && lpConfigPath[0] ) )
		{
			continue;
		}

		// ����ָ���ʵ��
		DebugCommand::CommandInstance *pInstance = XGC_NEW DebugCommand::CommandInstance;

		pInstance->mEntryName   = pEntry->lpEntryName;
		pInstance->mEntryTable  = pEntry->lpEntryTable;
		pInstance->mSendMessage = pEntry->lpEntryPrint;

		// �˴�ָ�����Բ����������ļ���
		if( xgc_nullptr == GetNormalPath( pInstance->mConfigPath, sizeof( pInstance->mConfigPath ), "%s", lpConfigPath ) )
		{
			SYS_INFO( "%s%s", "����ָ���з�����Ч��·������", lpConfigPath );
			SAFE_DELETE( pInstance );
		}

		// �����������·��
		if( xgc_nullptr == GetNormalPath( pInstance->mScriptPath, sizeof( pInstance->mScriptPath ), "%s", lpScriptPath ) )
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
		for each( auto &it in gCommandInstMap )
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
	for each( auto it in gCommandInstMap )
	{
		FiniDebugCmd( it.second );
		SAFE_DELETE( it.second );
	}

	gCommandInstMap.clear();
}

namespace DebugCommand
{
	static xgc_uint16 OnCmd_Authorised( xgc_size argc, xgc_lpstr const *argv, xgc_bool bConfirmed )
	{
		auto it = gpInstance->mGroupMap.find( gpCurrent->group );
		if( it != gpInstance->mGroupMap.end() )
		{
			xgc_uint16 nAuthorised = 0;

			Command *pCommand = it->second.mCommands;
			if( pCommand->mChildrens.size() == 0 )
			{
				nAuthorised = -1;
				return nAuthorised;
			}
			while( argc && pCommand->mChildrens.size() )
			{
				if( argv[0][0] != '-' )
				{
					auto itc = pCommand->mChildrens.find( argv[0] );
					if( itc == pCommand->mChildrens.end() )
					{
						nAuthorised = -1;
						break;
					}

					//��ָ����Ƿ���Ҫ����ȷ��
					if( bConfirmed == false && itc->second->mConfirm )
					{
						nAuthorised = 1;
					}
					pCommand = itc->second;
				}
				++argv;
				--argc;
			}

			return nAuthorised;
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
		int cnt = _vsnprintf_s( szMessage, sizeof(szMessage) -1, lpFmt, args );
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
				if( pCmdInfo->pfnCmd && false == pCmdInfo->pfnCmd( argc, argv, pCmdInfo ) )
					return false;

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
			strncpy_s( szCmdString, lpCmdString, xgc_countof( szCmdString ) - 1 );
		}

		gpCurrent = lpCurrent;
		//����ָ�����ȷ��,����Ҫ��֤Ȩ��
		xgc_lpcstr lpRoleName = gpCurrent->name ? gpCurrent->name : "";

		//��֤�Ƿ�ͨ��
		xgc_bool bConfirmed = true;
		if( stricmp( szCmdString, "y" ) == 0 )
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
					else if( !isblank( *str ) && ( isprint( *str ) || _ismbslead( (xgc_byte*) str, (xgc_byte*) str ) ) )
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

		USR_INFO( "��ɫ����ִ��GMָ�� Role = %s GM = %d, Execute = %s",
			lpRoleName,
			gpCurrent->group,
			szCmdString );

		switch( OnCmd_Authorised( argc, argv, bConfirmed ) )
		{
			case 0: // ��֤ͨ��
			if( OnCmd_CommandExec( gpInstance->mEntryTable, argc, argv ) )
			{
				PrintClient( "%s", szCmdString );
				PrintClient( "ָ��ִ�гɹ���" );
                //ADD_EVTLOG
                UseGmCmdEvent evt = { 0 };
				strcpy_s( evt.szRoleName, lpCurrent->name );
				strcpy_s( evt.szGmCmd, lpCmdString );
                evt.byGmLevel = lpCurrent->group;
                evt.byresult = 0;
                LogEvent(MsgUseGmCmdEvent, evt);
				gCommandInstCache.erase( lpRoleName );
				return true;
			}
			break;
			case 1: // ��Ҫ����ȷ��
			{
				//GDָ���ɫ����Ϊ�գ�����Ҫ����ȷ��
				CommandCache info;
				info.mCmd = szCmdString;
				info.mUpdateTime = (xgc_uint32) time( xgc_nullptr );
				gCommandInstCache[lpRoleName] = info;

				PrintClient( "����ָ�%s", szCmdString );
				PrintClient( "ȷ��ִ�������� @y " );
                //ADD_EVTLOG
                UseGmCmdEvent evt = { 0 };
				strcpy_s( evt.szRoleName, lpCurrent->name );
				strcpy_s( evt.szGmCmd, lpCmdString );
                evt.byGmLevel = lpCurrent->group;
                evt.byresult = 1;
                LogEvent(MsgUseGmCmdEvent, evt);
				return true;
			}
			break;
			default:
			{
				USR_INFO( "ָ��Ȩ��δͨ����֤�� Role = %s GM = %d",
					gpCurrent->name,
					gpCurrent->group );
                //ADD_EVTLOG
                UseGmCmdEvent evt = { 0 };
                strcpy_s(evt.szRoleName, lpCurrent->name);
                
				strcpy_s( evt.szGmCmd, lpCmdString );
                evt.byGmLevel = lpCurrent->group;
                evt.byresult = -1;
                LogEvent(MsgUseGmCmdEvent, evt);
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
			for each( auto &it in gpInstance->mAliasMap )
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
		PrintClient( "server name : %s", GetServerName() );
		PrintClient( "server code : %s", GetServerCode() );

		xgc_char szPath[_MAX_PATH] = { 0 };
		PrintClient( "server path : %s", GetModulePath() );
		PrintClient( "server conf : %s", GetConfPath( szPath, sizeof( szPath ), "" ) );

		xgc_char szDate[32] = { 0 };
		xgc_char szTime[32] = { 0 };
		PrintClient( "server date : %s", _strdate( szDate ) );
		PrintClient( "server time : %s", _strtime( szTime ) );
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
		xgc_char szPath[_MAX_PATH] = { 0 };
		if( argc > 1 )
			sprintf_s( szPath, "%s\\%s", gpInstance->mScriptPath, argv[1] );
		else
			sprintf_s( szPath, "%s", gpInstance->mScriptPath );

		_fullpath( szPath, szPath, sizeof( szPath ) );

		struct _stat stat;
		if( 0 != _stat( szPath, &stat ) )
		{
			PrintClient( "��Ч��·�����ļ���! %s", szPath );
			return false;
		}

		PrintClient( "%s", szPath );

		if( stat.st_mode & _S_IFDIR )
		{

			strcat_s( szPath, "\\*" );
			_finddata_t fdata;
			memset( &fdata, 0, sizeof( fdata ) );

			intptr_t fd = _findfirst( szPath, &fdata );
			if( fd == -1 )
			{
				PrintClient( "��Ч��Ŀ¼!" );
				return false;
			}

			do
			{
				PrintClient( "%s\n", fdata.name );
			} while( _findnext( fd, &fdata ) == 0 );

			_findclose( fd );
		}
		else if( stat.st_mode & _S_IFREG )
		{
			FILE *fp = xgc_nullptr;
			fopen_s( &fp, szPath, "r" );
			if( xgc_nullptr == fp )
			{
				PrintClient( "����ִ��GMָ��,���ļ�:[%s]ʧ��,dwErr:[%d]", szPath, GetLastError() );
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