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
/// 初始化服务器
/// [11/26/2014] create by albert.xu
/// @param lpConfigPath 配置文件路径
/// @param InitConfiguration 配置初始化回调
/// @param lpParam 配置初始化参数
///
xgc_bool ServerInit( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( xgc::common::ini_reader &, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// 运行服务器
/// [11/29/2014] create by albert.xu
///
xgc_void ServerLoop( xgc_bool( *OnServerStep )( xgc_bool, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// 结束服务器
/// [11/26/2014] create by albert.xu
///
xgc_void ServerFini( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// 获取配置文件路径
///
xgc_void ServerConfigFile( xgc_lpstr szPath, xgc_size nSize );

///
/// 获取配置路径
///
xgc_void ServerConfigPath( xgc_lpstr szPath, xgc_size nSize );

///
/// 获取服务器名
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr ServerName();

#endif // _SERVER_BASE_H_
