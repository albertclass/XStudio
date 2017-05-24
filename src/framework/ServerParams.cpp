/******************************************************************************
*	Copyright (c) 2013,	SNDA, all rights reserved.
*	FileName:	GlobalParam.cpp
*	Abstract:	the class manages global params. This class only read value from 
		database or receive value from MServer. DO NOT change any value of global
		param.
*	History:
*		Date		Version		Author		Description
*		2013-09-06	V0.1		Huxiaobo	Initial version
******************************************************************************/
#include "ServerDefines.h"
#include "ServerParams.h"
#include "ServerDatabase.h"

using namespace xgc::sql;

/// @var 通知回调
static xgc_void( *gNotifyCallback )( const stGlobalParam& Parameter ) = xgc_nullptr;

/// @var 服务器全局参数
static xgc_unordered_map< xgc_string, stGlobalParam > gGlobalParams;

///
/// 设置同步回调
/// [12/19/2014] create by albert.xu
///
xgc_void SetGlobalParameterNotifier( xgc_void( *pfnNotify )( const stGlobalParam& Parameter ) )
{
	gNotifyCallback = pfnNotify;
}

///
/// 从配置文件加载
/// [12/19/2014] create by albert.xu
///
static xgc_bool LoadGlobalConfig( ini_reader &ini )
{
	xgc_lpcstr lpSecionName = "ServerParams";
	xgc_uint32 nNewParamCount = 0;

	if( ini.is_exist_section( lpSecionName ) )
	{
		// 存在配置则读取，不存在则略过
		for( xgc_size nIdx = 0; nIdx < ini.get_item_count( lpSecionName ); ++nIdx )
		{
			xgc_lpcstr lpKey = ini.get_item_name( lpSecionName, nIdx );
			xgc_lpcstr lpVal = ini.get_item_value( lpSecionName, nIdx, xgc_nullptr );

			XGC_ASSERT_CONTINUE( lpVal );
			// 此名字保留，用于重定向
			if( strcmp( "ConfigurationPath", lpKey ) == 0 )
			{
				xgc_char szIniFile[XGC_MAX_PATH] = { 0 };
				get_absolute_path( szIniFile, XGC_MAX_PATH, lpVal );

				// 加载重定向的配置文件
				ini_reader params;
				if( false == params.load( szIniFile ) )
				{
					SYS_ERROR( "无法打开服务器参数配置文件 - %s", szIniFile );
					return false;
				}

				// 递归调用，逐级读取。
				XGC_ASSERT_RETURN( LoadGlobalConfig( params ), false );
			}
			else
			{
				// 判断是否已读取该服务器端配置
				if( xgc_nullptr == GetGlobalParameter( lpKey, xgc_nullptr ) )
				{
					xgc_char szVal[256] = { 0 };
					strcpy_s( szVal, lpVal );
					xgc_uint32 nMask = 0;
					xgc_lpstr lpNext = xgc_nullptr;
					xgc_lpstr lpVal = strtok_s( szVal, " ,", &lpNext );
					if( xgc_nullptr == lpVal )
						continue;

					xgc_lpstr lpMask = strtok_s( lpNext, " ,", &lpNext );
					while( lpMask )
					{
						if( strcmp( lpMask, "CLIENT" ) == 0 )
							nMask |= GLOBAL_PARAM_MASK_CLIENT;
						else if( strcmp( lpMask, "SYNCDB" ) == 0 )
							nMask |= GLOBAL_PARAM_MASK_SYNCDB;
						else if( strcmp( lpMask, "SYNCMS" ) == 0 )
							nMask |= GLOBAL_PARAM_MASK_SYNCMS;
						else if( strcmp( lpMask, "SYNCGS" ) == 0 )
							nMask |= GLOBAL_PARAM_MASK_SYNCGS;
						else if( strcmp( lpMask, "SYNCGG" ) == 0 )
							nMask |= GLOBAL_PARAM_MASK_SYNCGG;
						else if( strcmp( lpMask, "SCRIPT" ) == 0 )
							nMask |= GLOBAL_PARAM_MASK_SCRIPT;
						else
							XGC_DEBUG_MESSAGE( "未知的属性配置" );

						lpMask = strtok_s( xgc_nullptr, " ,", &lpNext );
					}

					// XGC_ASSERT_MESSAGE( nMask, "全局变量未配置属性，这将导致该属性仅在本地服务器生效。" );
					SetGlobalParameter( lpKey, lpVal, nMask | GLOBAL_PARAM_MASK_SYSTEM );
				}
			}
		}
	}

	return true;
}

///
/// 初始化全局变量表
/// [12/19/2014] create by albert.xu
///
xgc_bool InitGlobalParams( ini_reader& ini )
{
	if( false == SyncGlobalParams() )
		return false;

	return LoadGlobalConfig( ini );
}

///
/// 同步全局变量表
/// [8/27/2014] create by albert.xu
///
xgc_bool SyncGlobalParams()
{
	sql_recordset rs;
	sql_result res = SyncDBExecuteRc( "select `key`, `value`, `mask`, `updatetime` from `global_memory`;", rs );
	if( sql_failed == res )
	{
		SYS_ERROR( "%u - %s", SyncDBErrorCode(), SyncDBErrorInfo() );
		release( rs );
		return false;
	}

	if( sql_success == res )
	{
		while( movenext( rs ) )
		{
			xgc_lpcstr key = field_string( rs, 0 );
			xgc_lpcstr val = field_string( rs, 1 );

			xgc_uint32 mask = field_unsigned( rs, 2, 0 );

			datetime update = datetime::convert( field_string( rs, 3 ) );

			auto it = gGlobalParams.find( key );
			if( it == gGlobalParams.end() )
			{
				// 没有则添加
				stGlobalParam stParam;
				strcpy_s( stParam.key, key );
				strcpy_s( stParam.val, val );
				stParam.mask = mask;
				stParam.update = datetime::now();

				gGlobalParams.insert( std::make_pair( key, stParam ) );
			}
			else if( it->second.update > update )
			{
				// 写回数据库
				xgc_char sql[256];
				sprintf_s( sql, "update global_memory set `value`='%s'", val );
				if( false == SyncDBExecute( sql ) )
				{
					SYS_ERROR( "%u - %s", SyncDBErrorCode(), SyncDBErrorInfo() );
					release( rs );
					return false;
				}
			}
			else
			{
				// 更新本地
				strcpy_s( it->second.val, val );
				it->second.mask = mask;
				it->second.update = update;
			}
		}
	}
	release( rs );

	return true;
}

///
/// 默认的Fillter
/// [1/29/2015] create by albert.xu
///
xgc_bool GetGlobalParameter_DefaultFillter( const stGlobalParam& st, xgc_lpcstr lpKey, xgc_uint32 nMask )
{
	if( lpKey )
	{
		if( xgc_nullptr == strstr( st.key, lpKey ) )
			return false;
	}

	if( nMask )
	{
		if( 0 == ( st.mask & nMask ) )
			return false;
	}

	return true;
}

///
/// 获取符合条件的全局变量
/// [1/16/2015] create by albert.xu
///
xgc_vector< stGlobalParam > GetGlobalParameter( std::function< xgc_bool( const stGlobalParam& ) > fnFillter )
{
	xgc_vector< stGlobalParam > Container;
	FUNCTION_BEGIN;
	for each ( auto const &it in gGlobalParams )
	{
		if( fnFillter( it.second ) )
			Container.push_back( it.second );
	}

	FUNCTION_END;
	return Container;
}

///
/// 获取开关型全局变量
/// [8/27/2014] create by albert.xu
///
xgc_bool GetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bDefault, xgc_bool bFromDB )
{
	xgc_lpcstr lpValue = GetGlobalParameter( lpKey, xgc_nullptr, bFromDB );
	if( lpValue == xgc_nullptr )
		return bDefault;

	if( strcasecmp( "true", lpValue ) == 0 )
		return true;

	if( atoi( lpValue ) != 0 )
		return true;

	return false;
}

///
/// 获取字符串型的全局变量
/// [8/27/2014] create by albert.xu
///
xgc_lpcstr GetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpDefault, xgc_bool bFromDB )
{
	if( bFromDB )
	{
		sql_recordset rs = 0;

		xgc_char sql[128];
		sprintf_s( sql, "select `key`, `value`, `mask`, `updatetime` from `global_memory` where `key`='%s'", lpKey );
		sql_result res = SyncDBExecuteRc( sql, rs );
		if( sql_failed == res )
		{
			SYS_ERROR( "%u - %s", SyncDBErrorCode(), SyncDBErrorInfo() );
		}
		else if( sql_success == res && movenext( rs ) )
		{
			xgc_lpcstr key = field_string( rs, 0 );
			xgc_lpcstr val = field_string( rs, 1 );

			xgc_uint32 mask = field_unsigned( rs, 2, 0 );
			datetime update = datetime::convert( field_string( rs, 3 ) );

			auto it = gGlobalParams.find( key );
			if( it == gGlobalParams.end() )
			{
				// 没有则添加
				stGlobalParam stParam;
				strcpy_s( stParam.key, key );
				strcpy_s( stParam.val, val );
				stParam.mask = mask;
				stParam.update = datetime::now();

				gGlobalParams.insert( std::make_pair( key, stParam ) );
			}
			else
			{
				strcpy_s( it->second.val, val );
				it->second.mask		= mask;
				it->second.update	= update;
			}
		}

		release( rs );
	}

	auto it = gGlobalParams.find( lpKey );
	if( it != gGlobalParams.end() )
		lpDefault = it->second.val;

	return lpDefault;
}

//--------------------------------------------------//

///
/// 设置开关型全局变量
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bValue, xgc_uint32 nMask )
{
	return SetGlobalParameter( lpKey, bValue ? "true" : "false", nMask );
}

///
/// 设置字符串型的全局变量，带变量属性掩码
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpValue, xgc_uint32 nMask )
{
	// 保存到DB
	if( nMask & GLOBAL_PARAM_MASK_SAVEDB )
	{
		xgc_char sql[1024] = { 0 };
		if( lpValue )
		{
			if( nMask & GLOBAL_PARAM_MASK_CREATE )
			{
				// 允许新建
				sprintf_s( sql,
					"insert global_memory( `key`, `value`, `mask` ) values( '%s', '%s', %u ) "
					"on duplicate key update value='%s';",
					lpKey,
					lpValue,
					nMask & 0xffff,
					lpValue );
			}
			else
			{
				// 仅作更新
				sprintf_s( sql, "update global_memory set value='%s' where `key`='%s';", lpValue, lpKey );
			}
		}
		else
		{
			sprintf_s( sql, "delete from global_memory where `key` = '%s';", lpKey );
		}

		if( false == SyncDBExecute( sql ) )
		{
			SYS_ERROR( "%u - %s", SyncDBErrorCode(), SyncDBErrorInfo() );
			return false;
		}
	}

	// 清除全局变量
	if( xgc_nullptr == lpValue )
	{
		gGlobalParams.erase( lpKey );
		return true;
	}

	auto it = gGlobalParams.find( lpKey );
	if( it == gGlobalParams.end() )
	{
		if( 0 == XGC_CHK_FLAGS( nMask, GLOBAL_PARAM_MASK_CREATE ) )
			return false;

		if( xgc_nullptr == lpValue )
			return false;
		
		// 没有则添加
		stGlobalParam stParam;
		strcpy_s( stParam.key, lpKey );
		strcpy_s( stParam.val, lpValue );
		stParam.mask = nMask & 0xffff;
		stParam.update = datetime::now();

		auto ib = gGlobalParams.insert( std::make_pair( lpKey, stParam ) );
		XGC_ASSERT_RETURN( ib.second, false );
		it = ib.first;
	}

	strcpy_s( it->second.val, lpValue );
	it->second.update = datetime::now();

	if( XGC_CHK_FLAGS( nMask, GLOBAL_PARAM_MASK_NOTIFY ) && gNotifyCallback )
		gNotifyCallback( it->second );

	return true;
}


///
/// 依赖总开关获取子开关 
/// [3/9/2015] create by wuhailin.jerry
///
xgc_bool GetGlobalParameterWithFather( xgc_lpcstr lpFatherkey, xgc_lpcstr lpSunKey, xgc_bool bDefault, xgc_bool bFromDB )
{
	// 父开关关闭
	if (!GetGlobalParameter( lpFatherkey, false ))
	{
		return false;
	}

	return GetGlobalParameter( lpSunKey, bDefault );
}