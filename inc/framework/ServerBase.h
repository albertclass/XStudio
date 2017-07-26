#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_
#include "ServerService.h"
#include "ServerCommand.h"
#include "ServerDatabase.h"
#include "ServerParams.h"
#include "ServerDatabase.h"
#include "ServerLogger.h"
#include "ServerRefresh.h"
#include "ServerAsyncEvt.h"
#include "ServerSequence.h"

///
/// ��ʼ��������
/// [11/26/2014] create by albert.xu
/// @param lpConfigPath �����ļ�·��
/// @param InitConfiguration ���ó�ʼ���ص�
/// @param lpParam ���ó�ʼ������
///
xgc_bool ServerInit( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( xgc::common::ini_reader &, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// ���з�����
/// [11/29/2014] create by albert.xu
///
xgc_void ServerLoop( xgc_bool( *OnServerStep )( xgc_bool, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// ����������
/// [11/26/2014] create by albert.xu
///
xgc_void ServerFini( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// ��ȡ�����ļ�·��
///
xgc_void ServerConfigFile( xgc_lpstr szPath, xgc_size nSize );

///
/// ��ȡ����·��
///
xgc_void ServerConfigPath( xgc_lpstr szPath, xgc_size nSize );

///
/// ��ȡ��������
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr ServerName();

#endif // _SERVER_BASE_H_
