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
	/// @var ��
	xgc::string key;
	/// @var ֵ
	xgc::string val;
	/// @var ���λ���룬˵���ñ�����;
	xgc_uint32 mask;
	/// @var ������ʱ��
	datetime update;
};

/// @var �־û�
#define GLOBAL_PARAM_MASK_HOLDON	0x10000000
/// @var ֵ���ʱ����֪ͨ
#define GLOBAL_PARAM_MASK_NOTIFY	0x20000000

/// @var �ͻ���ʹ�õ�,���ڽ��������ʱ֪ͨ���ͻ���
#define GLOBAL_PARAM_MASK_CLIENT	0x00001000

/// @var �ű�ʹ�õ�
#define GLOBAL_PARAM_MASK_SCRIPT	0x00002000

/// @var ���ʱ֪ͨDB
#define GLOBAL_PARAM_MASK_SYNCDB	0x00000001
/// @var ���ʱ֪ͨMS
#define GLOBAL_PARAM_MASK_SYNCMS	0x00000002
/// @var ���ʱ֪ͨGS
#define GLOBAL_PARAM_MASK_SYNCGS	0x00000004
/// @var ���ʱ֪ͨGG
#define GLOBAL_PARAM_MASK_SYNCGG	0x00000008

/// ֪ͨ�ص�
typedef xgc_void( *PF_NotifyCallback )( const stGlobalParam& stParameter, bool bNew );

///
/// ��ʼ��ȫ�ֱ�����
/// [12/19/2014] create by albert.xu
///
xgc_bool InitGlobalParams( ini_reader& ini );

///
/// ����ͬ���ص�
/// [12/19/2014] create by albert.xu
///
xgc_void SetGlobalParameterNotifier( PF_NotifyCallback pfnNotifier );

///
/// GetGlobalParameter����Ĭ�ϵ�Fillter
/// [1/29/2015] create by albert.xu
///
xgc_bool GetGlobalParameter_DefaultFillter( const stGlobalParam& st, xgc_lpcstr lpKey, xgc_uint32 nMask );

///
/// ��ȡ����������ȫ�ֱ���
/// [1/16/2015] create by albert.xu
///
xgc::vector< stGlobalParam > GetGlobalParameter( const std::function< xgc_bool( const stGlobalParam& ) > &pfnFillter );


///
/// ��ȡ�ַ����͵�ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_lpcstr GetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpDefault );

///
/// ��ȡ������ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_bool GetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bDefault );

///
/// ��ȡ��ֵ��ȫ�ֱ���
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
/// �����ַ����͵�ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpValue, xgc_uint32 nMask );

///
/// ���ÿ�����ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_bool SetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bValue, xgc_uint32 nMask );

///
/// ������ֵ��ȫ�ֱ���
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
