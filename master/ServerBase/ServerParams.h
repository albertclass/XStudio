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
	xgc_char key[64];
	/// @var ֵ
	xgc_char val[64];
	/// @var ���λ���룬˵���ñ�����;
	xgc_uint32 mask;
	/// @var ������ʱ��
	datetime update;
};

/// @var �½������Ѵ��������
#define GLOBAL_PARAM_MASK_CREATE	0x10000000
/// @var ���浽���ݿ�
#define GLOBAL_PARAM_MASK_SAVEDB	0x20000000
/// @var ����֪ͨ
#define GLOBAL_PARAM_MASK_NOTIFY	0x40000000
/// @var ϵͳʹ�õ�
#define GLOBAL_PARAM_MASK_SYSTEM	( GLOBAL_PARAM_MASK_CREATE | GLOBAL_PARAM_MASK_SAVEDB )

/// @var �ͻ���ʹ�õ�,���ڽ��������ʱ֪ͨ���ͻ���
#define GLOBAL_PARAM_MASK_CLIENT	0x00001000

/// @var �ű�ʹ�õ�
#define GLOBAL_PARAM_MASK_SCRIPT	0x00008000

/// @var ���ʱ֪ͨDB
#define GLOBAL_PARAM_MASK_SYNCDB	0x40000001
/// @var ���ʱ֪ͨMS
#define GLOBAL_PARAM_MASK_SYNCMS	0x40000002
/// @var ���ʱ֪ͨGS
#define GLOBAL_PARAM_MASK_SYNCGS	0x40000004
/// @var ���ʱ֪ͨGG
#define GLOBAL_PARAM_MASK_SYNCGG	0x40000008

///
/// ��ʼ��ȫ�ֱ�����
/// [12/19/2014] create by albert.xu
///
xgc_bool InitGlobalParams( IniFile& ini );

///
/// ͬ��ȫ�ֱ�����
/// [8/27/2014] create by albert.xu
///
xgc_bool SyncGlobalParams();

///
/// ����ͬ���ص�
/// [12/19/2014] create by albert.xu
///
xgc_void SetGlobalParameterNotifier( xgc_void( *pfnNotify )( const stGlobalParam &Parameter ) );

///
/// Ĭ�ϵ�Fillter
/// [1/29/2015] create by albert.xu
///
xgc_bool GetGlobalParameter_DefaultFillter( const stGlobalParam& st, xgc_lpcstr lpKey, xgc_uint32 nMask );

///
/// ��ȡ����������ȫ�ֱ���
/// [1/16/2015] create by albert.xu
///
xgc_vector< stGlobalParam > GetGlobalParameter( std::function< xgc_bool( const stGlobalParam& ) > fnFillter );

///
/// ��ȡ������ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_bool GetGlobalParameter( xgc_lpcstr lpKey, xgc_bool bDefault, xgc_bool bFromDB = false );

///
/// ��ȡ��ֵ��ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
template< class T, typename std::enable_if< is_numeric< T >::value, xgc_bool >::type = true >
T GetGlobalParameter( xgc_lpcstr lpKey, T _Default, xgc_bool bFromDB = false )
{
	xgc_lpcstr lpValue = GetGlobalParameter( lpKey, xgc_nullptr, bFromDB );
	if( lpValue == xgc_nullptr )
		return _Default;

	return str2numeric< T >( lpValue );
}

///
/// ��ȡ�ַ����͵�ȫ�ֱ���
/// [8/27/2014] create by albert.xu
///
xgc_lpcstr GetGlobalParameter( xgc_lpcstr lpKey, xgc_lpcstr lpDefault, xgc_bool bFromDB = false );

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
	xgc_char szValue[64];
	numeric2str< T >( _Value, szValue, sizeof( szValue ) );

	return SetGlobalParameter( lpKey, szValue, nMask );
}

///
/// �����ܿ��ػ�ȡ�ӿ��� 
/// [3/9/2015] create by wuhailin.jerry
///
xgc_bool GetGlobalParameterWithFather(xgc_lpcstr lpFatherkey, xgc_lpcstr lpSunKey, xgc_bool bDefault, xgc_bool bFromDB = false);

#endif // _SERVER_PARAMS_H_
