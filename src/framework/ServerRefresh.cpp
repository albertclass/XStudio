#include "ServerDefines.h"
#include "ServerBase.h"
#include "ServerRefresh.h"
#include "ServerDatabase.h"

using namespace xgc::sql;

/// @var ˢ��ϵͳʹ�õ�ר�ж�ʱ��
static timer refresh_clock;

///
/// ˢ����Ϣ
/// [1/20/2015] create by albert.xu
///
struct RefreshInfo
{
	/// @var ǰһ�ε���ʱ��
	datetime prev_invoke_time;
	/// @var ��һ�ε���ʱ��
	datetime next_invoke_time;
	/// @var ϵͳ����
	xgc_char sysname[64];
	/// @var ˢ������
	xgc_char type[128];
	/// @var ��ʱ����
	xgc_char clock_invoke[128];
	/// @var ������
	xgc_char check_invoke[128];
	/// @var ��ʱ�����
	timer_h  handle;
};

/// @var ˢ����Ϣ
static xgc_unordered_map< xgc_string, xgc_vector< RefreshInfo* > > gRefreshInfos;

/// @var ˢ��ϵͳ����
static xgc_char szConfName[64] = { 0 };
static xgc_char szConfPath[XGC_MAX_PATH] = { 0 };

xgc_bool InitServerRefresh( ini_reader &ini )
{
	FUNCTION_BEGIN;
	if( ini.is_exist_section( "RefreshConf" ) )
	{
		xgc_lpcstr lpConfName = ini.get_item_value( "RefreshConf", "ConfigName", xgc_nullptr );
		XGC_ASSERT_RETURN( lpConfName, false, "δ����RefreshConf.ConfigName" );
		xgc_lpcstr lpConfPath = ini.get_item_value( "RefreshConf", "ConfigPath", xgc_nullptr );
		XGC_ASSERT_RETURN( lpConfName, false, "δ����RefreshConf.ConfigPath" );

		strcpy_s( szConfName, lpConfName );

		get_absolute_path( szConfPath, sizeof( szConfPath ), "%s", lpConfPath );

		if( !LoadServerRefresh() )
		{
			USR_ERROR( "refresh init failed." );
			return false;
		}
	}

	FUNCTION_END;
	return true;
}

///
/// ��������ģʽ
/// ����ģʽ����'@'��ʼ��Ϊ����������������һ�����ַ������֡��»��ߵ��ַ���ֹ.
/// ʹ�� @@ ת��Ϊһ�� @
/// ������ֻ���� ��ĸ�����֡��»�����ɣ������ַ����������֡�
/// [1/21/2015] create by albert.xu
///
static xgc_bool AnalyseInvokePattern( pugi::xml_node &xml, xgc_lpcstr lpPattern, xgc_lpstr lpBuffer, xgc_size nSize )
{
	XGC_ASSERT_RETURN( lpPattern && lpBuffer && nSize, false );
	xgc_size nWrite = 0;

	xgc_lpcstr pArg = xgc_nullptr;
	for( xgc_size n = 0; lpPattern[n] && nSize > nWrite; ++n )
	{
		if( xgc_nullptr == pArg && lpPattern[n] == '@' )
		{
			if( lpPattern[++n] != '@' )
				pArg = lpPattern + n;

			// ����ĸ��������ĸ���»��߿�ͷ
			if( !isalpha( *pArg ) && *pArg != '_' )
				return false;
		}

		if( pArg )
		{
			// �ҵ���һ�������ַ�Ҳ��������Ҳ����'_'���ַ�����Ϊ�������ѽ���
			if( !isalnum( lpPattern[n] ) && lpPattern[n] != '_' )
			{
				xgc_char name[64] = { 0 };
				strncpy_s( name, pArg, lpPattern + n - pArg );

				auto attr = xml.attribute( name ).as_string();
				XGC_ASSERT_RETURN( attr, false, "����[%s]������", name );

				int cpy = sprintf_s( lpBuffer + nWrite, nSize - nWrite, "%s", attr );
				XGC_ASSERT_RETURN( cpy > 0, false, "����[%s]���岻��, nWrite = %u", name, nWrite );

				nWrite += cpy;

				*( lpBuffer + nWrite ) = lpPattern[n];
				++nWrite;

				pArg = xgc_nullptr;
			}
		}
		else
		{
			*( lpBuffer + nWrite ) = lpPattern[n];
			++nWrite;
		}
	}

	if( nWrite >= nSize )
		return false;

	*( lpBuffer + nWrite ) = 0;
	return true;
}

xgc_bool LoadServerRefresh()
{
	FUNCTION_BEGIN;
	FiniServerRefresh();

	pugi::xml_document xml;
	auto result = xml.load( szConfPath );
	if( !result )
	{
		USR_ERROR( "��ȡ�ļ�%s, ���� - %s", szConfPath, result.description() );
		return false;
	}

	xgc_bool bRet = false;

	auto refresh_node = xml.child( "refresh" );
	if( !refresh_node )
		return false;

	auto server_node = refresh_node.child( "server" );
	while( server_node )
	{
		xgc_string strConfName = server_node.attribute("name").as_string();
			
		if( strConfName != szConfName )
			continue;

		auto system_node = server_node.child( "system" );
		while( system_node )
		{
			auto sysname = system_node.attribute( "name" ).as_string( xgc_nullptr );
			XGC_ASSERT_BREAK( sysname, "system node attribute 'name' not found." );

			// ��ȡ����ģʽ��ģʽ����@��ʼ��Ϊ���Բ�����������������һֱ����һ����ƥ���ַ���ȡֵΪ��������
			auto clock_invoke_pattern = system_node.attribute( "clock_invoke" ).as_string();
			XGC_ASSERT_BREAK( clock_invoke_pattern, "system node attribute 'clock_invoke' not found." );

			auto check_invoke_pattern = system_node.attribute( "check_invoke" ).as_string();
			XGC_ASSERT_BREAK( clock_invoke_pattern, "system node attribute 'check_invoke' not found." );

			auto item_node = system_node.child( "item" );
			while( item_node )
			{
				// ���ˢ����ṹ
				RefreshInfo * pInfo = XGC_NEW RefreshInfo;
				memset( pInfo, 0, memsize( pInfo ) );
				strcpy_s( pInfo->sysname, sysname );

				auto start = item_node.attribute( "start" ).as_string( xgc_nullptr );
				if( xgc_nullptr == start )
				{
					XGC_ASSERT_MESSAGE( bRet, "read attrib start error" );
					SAFE_DELETE( pInfo );
					break;
				}

				auto type = item_node.attribute( "type" ).as_string( xgc_nullptr );
				if( xgc_nullptr == type )
				{
					XGC_ASSERT_MESSAGE( bRet, "read attrib type error" );
					SAFE_DELETE( pInfo );
					break;
				}

				strcpy_s( pInfo->type, type );

				pInfo->prev_invoke_time = datetime::convert( start );
				pInfo->prev_invoke_time = refresh_clock.adjust_lower( pInfo->prev_invoke_time, pInfo->type );
				pInfo->next_invoke_time	= refresh_clock.adjust_upper( pInfo->prev_invoke_time, pInfo->type );

				xgc_char szPrevInvokeTime[64] = { 0 };
				xgc_char szNextInvokeTime[64] = { 0 };

				pInfo->prev_invoke_time.to_string( szPrevInvokeTime, sizeof( szPrevInvokeTime ) );
				pInfo->next_invoke_time.to_string( szNextInvokeTime, sizeof( szNextInvokeTime ) );

				// ��������ģʽ���ؽ����ò���
				bRet = AnalyseInvokePattern( xml, clock_invoke_pattern, pInfo->clock_invoke, sizeof( pInfo->clock_invoke ) );
				if( false == bRet )
				{
					XGC_ASSERT_MESSAGE( bRet, "clock invoke pattern analyse error %s", clock_invoke_pattern );
					SAFE_DELETE( pInfo );
					break;
				}

				bRet = AnalyseInvokePattern( xml, check_invoke_pattern, pInfo->check_invoke, sizeof( pInfo->check_invoke ) );
				if( false == bRet )
				{
					XGC_ASSERT_MESSAGE( bRet, "clock invoke pattern analyse error %s", check_invoke_pattern );
					SAFE_DELETE( pInfo );
					break;
				}

				DBG_INFO( "ˢ������Ϊ %s ��һ�ε���ʱ��Ϊ[%s]����һ�ε���ʱ��Ϊ[%s]", pInfo->type, szPrevInvokeTime, szNextInvokeTime );

				auto func = [pInfo]( timer_h, intptr_t& )->xgc_void
				{
					OnServerRefresh( (xgc_lpcstr) pInfo->clock_invoke, pInfo->prev_invoke_time, xgc_nullptr );

					pInfo->prev_invoke_time = pInfo->next_invoke_time;
					pInfo->next_invoke_time = refresh_clock.adjust_upper( pInfo->prev_invoke_time, pInfo->type );
				};

				pInfo->handle = refresh_clock.insert( func, pInfo->prev_invoke_time, DURATION_FOREVER, pInfo->type );

				if( INVALID_TIMER_HANDLE == pInfo->handle )
				{
					XGC_ASSERT_MESSAGE( bRet, "insert clock error %llu, %s", pInfo->prev_invoke_time.to_ctime(), pInfo->type );
					SAFE_DELETE( pInfo );
					break;
				}

				gRefreshInfos[sysname].push_back( pInfo );
				item_node = item_node.next_sibling( "item" );
			}
			// end of refresh element
			system_node = system_node.next_sibling( "system" );
		}

		// end of server element
		server_node = server_node.next_sibling( "server" );
	}

	return bRet;
	FUNCTION_END;
	return false;
}

xgc_void StepServerRefresh()
{
	FUNCTION_BEGIN;
	refresh_clock.step( datetime::now().to_ctime() );
	FUNCTION_END;
}

xgc_void FiniServerRefresh()
{
	FUNCTION_BEGIN;
	for( auto &itr : gRefreshInfos )
	{
		for( auto ptr : itr.second )
		{
			if( ptr->handle != INVALID_TIMER_HANDLE )
			{
				refresh_clock.remove( ptr->handle );
				ptr->handle = INVALID_TIMER_HANDLE;
				delete ptr;
			}
		}
	}

	gRefreshInfos.clear();
	FUNCTION_END;
}

xgc_void UpdateServerRefresh( xgc_lpcstr lpSystem, xgc_lpvoid lpContext )
{
	FUNCTION_BEGIN;
	for( auto &it : gRefreshInfos )
	{
		if( strcasecmp( it.first.c_str(), lpSystem ) != 0 )
			continue;
		
		for( auto ptr : it.second )
		{
			OnServerRefresh( ptr->check_invoke, ptr->prev_invoke_time, lpContext );
		}
	}
	FUNCTION_END;
}
