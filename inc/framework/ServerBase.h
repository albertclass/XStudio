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
/// 报告服务状态
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceStatus( xgc_uint32 nState, xgc_uint32 nExitCode, xgc_uint32 nWaitHint );

///
/// 报告服务器事件
/// @param nEventType EVENTLOG_
/// @param nErrorCode ServerService.h defined.
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceEvent( xgc_uint16 nEventType, xgc_uint32 nErrorCode, xgc_lpcstr lpInfomation );

///
/// 服务器是否已停止
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerStoped();

///
/// 服务器是否已暂停
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerPaused();

///
/// 是否通过服务启动
/// [12/5/2014] create by albert.xu
///
xgc_bool IsServerService();

///
/// 获取服务器名
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerName();

///
/// 获取配置路径
/// [11/29/2014] create by albert.xu
///
xgc_lpcstr GetConfPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

///
///  获取配置路径
/// [1/4/2015] create by wuhailin.jerry
///
xgc_string GetConfPath( xgc_lpcstr lpRelativePath, ... );

///
/// 获取配置路径
/// [8/6/2015] create by jianglei.kinly
///
xgc_string LuaGetConfPath( xgc_lpcstr lpRelativePath );

///
/// 获取日志路径
/// [12/3/2014] create by albert.xu
///
xgc_lpcstr GetLogPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

//////////////////////////////////////////////////////////////////////////
// 刷新系统调用函数
//////////////////////////////////////////////////////////////////////////

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