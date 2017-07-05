#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_
#include "ServerService.h"
#include "ServerCommand.h"
#include "ServerDatabase.h"
#include "ServerEventLog.h"
#include "ServerParams.h"

///
/// 初始化服务器
/// [11/26/2014] create by albert.xu
/// @param lpConfigPath 配置文件路径
/// @param InitConfiguration 配置初始化回调
/// @param lpParam 配置初始化参数
///
xgc_bool InitServer( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( xgc::common::ini_reader &, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// 运行服务器
/// [11/29/2014] create by albert.xu
///
xgc_void RunServer( xgc_bool( *OnServerStep )( xgc_bool, xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// 结束服务器
/// [11/26/2014] create by albert.xu
///
xgc_void FiniServer( xgc_void( *FiniConfiguration )( xgc_lpvoid ), xgc_lpvoid lpParam );

///
/// 获取服务器名
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerName();

///
/// 处理刷新子系统事件
/// @param lpSystem 系统名称
/// @param lpContext 调用上下文
/// [1/21/2015] create by albert.xu
///
xgc_void UpdateServerRefresh( xgc_lpcstr lpSystem, xgc_lpvoid lpContext );

///
/// 获得一个唯一ID
/// [6/3/2015] create by jianglei.kinly
///
xgc_uint64 GetSequenceID();

#endif // _SERVER_BASE_H_
