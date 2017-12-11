#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_
#include "ServerService.h"
#include "ServerCommand.h"
#include "ServerParams.h"
#include "ServerRefresh.h"
#include "ServerEvent.h"
#include "ServerSequence.h"

///
/// \brief ��ʼ��������
/// \date 11/26/2014
/// \author albert.xu
/// \param lpConfigPath �����ļ�·��
/// \param InitConfiguration ���ó�ʼ���ص�
/// \param lpParam ���ó�ʼ������
///
xgc_bool InitServer( xgc_lpcstr lpConfigPath, const std::function< bool( ini_reader & ) > & fnInitConf );

///
/// ���з�����
/// [11/29/2014] create by albert.xu
///
xgc_void LoopServer( const std::function< bool( bool ) > &GameLogic );

///
/// ����������
/// [11/26/2014] create by albert.xu
///
xgc_void FiniServer( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam );

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
