#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_
#include "ServerService.h"
#include "ServerCommand.h"
#include "ServerParams.h"
#include "ServerRefresh.h"
#include "ServerEvent.h"
#include "ServerSequence.h"

///
/// \brief 初始化服务器
/// \date 11/26/2014
/// \author albert.xu
/// \param lpConfigPath 配置文件路径
/// \param InitConfiguration 配置初始化回调
/// \param lpParam 配置初始化参数
///
xgc_bool InitServer( xgc_lpcstr lpConfigPath, const std::function< bool( ini_reader & ) > & fnInitConf );

///
/// 运行服务器
/// [11/29/2014] create by albert.xu
///
xgc_void LoopServer( const std::function< bool( bool ) > &GameLogic );

///
/// 结束服务器
/// [11/26/2014] create by albert.xu
///
xgc_void FiniServer( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam );

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
