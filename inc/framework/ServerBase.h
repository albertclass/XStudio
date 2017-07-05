#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_
#include "ServerService.h"
#include "ServerCommand.h"
#include "ServerDatabase.h"
#include "ServerEventLog.h"
#include "ServerParams.h"

///
/// ��ʼ��������
/// [11/26/2014] create by albert.xu
/// @param lpConfigPath �����ļ�·��
/// @param InitConfiguration ���ó�ʼ���ص�
/// @param lpParam ���ó�ʼ������
///
xgc_bool InitServer( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( xgc::common::ini_reader &, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// ���з�����
/// [11/29/2014] create by albert.xu
///
xgc_void RunServer( xgc_bool( *OnServerStep )( xgc_bool, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// ����������
/// [11/26/2014] create by albert.xu
///
xgc_void FiniServer( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// ��ȡ��������
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerName();

///
/// ����ˢ����ϵͳ�¼�
/// @param lpSystem ϵͳ����
/// @param lpContext ����������
/// [1/21/2015] create by albert.xu
///
xgc_void UpdateServerRefresh( xgc_lpcstr lpSystem, xgc_lpvoid lpContext );

///
/// ���һ��ΨһID
/// [6/3/2015] create by jianglei.kinly
///
xgc_uint64 GetSequenceID();

#endif // _SERVER_BASE_H_
