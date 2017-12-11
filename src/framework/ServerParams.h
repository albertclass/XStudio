/******************************************************************************
*	Copyright (c) 2013,	SNDA, all rights reserved.
*	FileName:	GlobalParam.h
*	Abstract:	the class manages global params. This class only read value from 
		database or receive value from MServer. DO NOT change any value of global
		param.
******************************************************************************/

#ifndef _SERVER_PARAMS_H_
#define _SERVER_PARAMS_H_

struct stGlobalParam
{
	/// @var 键
	xgc::string key;
	/// @var 值
	xgc::string val;
	/// @var 标记位掩码，说明该变量用途
	xgc_uint32 mask;
	/// @var 最后更新时间
	datetime update;
};

/// @var 持久化
#define GLOBAL_PARAM_MASK_HOLDON	0x10000000
/// @var 值变更时发送通知
#define GLOBAL_PARAM_MASK_NOTIFY	0x20000000

/// @var 客户端使用的,需在进入服务器时通知给客户端
#define GLOBAL_PARAM_MASK_CLIENT	0x00001000

/// @var 脚本使用的
#define GLOBAL_PARAM_MASK_SCRIPT	0x00002000

/// @var 变更时通知DB
#define GLOBAL_PARAM_MASK_SYNCDB	0x00000001
/// @var 变更时通知MS
#define GLOBAL_PARAM_MASK_SYNCMS	0x00000002
/// @var 变更时通知GS
#define GLOBAL_PARAM_MASK_SYNCGS	0x00000004
/// @var 变更时通知GG
#define GLOBAL_PARAM_MASK_SYNCGG	0x00000008

/// 通知回调
typedef xgc_void( *PF_NotifyCallback )( const stGlobalParam& stParameter, bool bNew );

///
/// 初始化全局变量表
/// [12/19/2014] create by albert.xu
///
xgc_bool InitGlobalParams( ini_reader& ini );

///
/// 设置同步回调
/// [12/19/2014] create by albert.xu
///
xgc_void SetGlobalParameterNotifier( PF_NotifyCallback pfnNotifier );

///
/// GetGlobalParameter函数默认的Fillter
/// [1/29/2015] create by albert.xu
///
xgc_bool GetGlobalParameter_DefaultFillter( const stGlobalParam& st, xgc_lpcstr lpKey, xgc_uint32 nMask );

///
/// 获取符合条件的全局变量
/// [1/16/2015] create by albert.xu
///
xgc::vector< stGlobalParam > GetGlobalParameter( const std::function< xgc_bool( const stGlobalParam& ) > &pfnFillter );


///
/// 获取字符串型的全局变量
/// [8/27/2014] create by albert.xu
///
xgc_lpcstr GetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpDefault );

///
/// 获取开关型全局变量
/// [8/27/2014] create by albert.xu
///
xgc_bool GetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bDefault );

///
/// 获取数值型全局变量
/// [8/27/2014] create by albert.xu
///
template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
T GetGlobalParameter( xgc_lpcstr lpKey, T _Default )
{
	xgc_lpcstr lpValue = GetGlobalParameter( lpKey, xgc_nullptr );
	if( lpValue == xgc_nullptr )
		return _Default;

	return str2numeric< T >( lpValue );
}

///
/// 设置字符串型的全局变量
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpValue, xgc_uint32 nMask );

///
/// 设置开关型全局变量
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bValue, xgc_uint32 nMask );

///
/// 设置数值型全局变量
/// [8/27/2014] create by albert.xu
///
template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, T _Value, xgc_uint32 nMask )
{
	xgc_char szValue[128];
	numeric2str< T >( _Value, szValue, sizeof( szValue ) );

	return SetGlobalParameter( lpKey, szValue, nMask );
}

#endif // _SERVER_PARAMS_H_
