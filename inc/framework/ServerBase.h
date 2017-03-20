#ifndef _SERVER_BASE_H_
#define _SERVER_BASE_H_
#include "ServerDefines.h"
#include "ServerService.h"
#include "ServerCommand.h"
#include "ServerDatabase.h"
#include "ServerEventLog.h"
#include "ServerParams.h"
#include "MessageHandler.h"

///
/// 初始化服务器
/// [11/26/2014] create by albert.xu
/// @param lpConfigPath 配置文件路径
/// @param InitConfiguration 配置初始化回调
/// @param lpParam 配置初始化参数
///
xgc_bool InitServer( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( IniFile &, xgc_lpvoid ), xgc_lpvoid lpParam );

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
/// 转换服务器ID为数值
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeType( xgc_uint32 nPipeID );

///
/// 转换服务器ID为数值
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeIndex( xgc_uint32 nPipeID );

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
// 网络相关的函数
//////////////////////////////////////////////////////////////////////////
// 管道相关的网络函数
///
/// 获取PipeSession
/// [12/1/2014] create by albert.xu
///
CPipeSession* GetPipeSession( ServerType eType, xgc_byte byIndex = 1 );

///
/// 获取PipeSession
/// [12/9/2014] create by albert.xu
///
CPipeSession* GetPipeSession( xgc_uint32 nPipeID );

void GetPipeSession(ServerType eType, vector<CPipeSession*>&);

///
/// 根据过滤条件获取服务器连接对象
/// [2/20/2014 albert.xu]
///
CPipeSession* GetPipeSession( const std::function< xgc_bool( xgc_uint32 ) > &fnFilter );

///
/// 获取PipeSession的PipeID
/// [12/9/2014] create by albert.xu
///
xgc_uint32 GetPipeID( CPipeSession* pSession );

///
/// 获取PipeSession的PipeID（字符串形式）
/// [12/11/2014] create by albert.xu
///
xgc_lpcstr GetPipeID( CPipeSession* pSession, xgc_lpstr lpOutput, xgc_size nSize = 0 );

//////////////////////////////////////////////////////////////////////////
// GameSession 相关的网络函数
//////////////////////////////////////////////////////////////////////////
///
/// 设置用户数据
/// [12/1/2014] create by albert.xu
///
xgc_void SetUserdata( CGameSession* pSession, xgc_lpvoid pUserdata );

///
/// 获取用户数据
/// [12/1/2014] create by albert.xu
///
xgc_lpvoid GetUserdata( CGameSession* pSession );

///
/// 断开连接
/// [12/1/2014] create by albert.xu
///
xgc_void Disconnect( CGameSession* pSession );

///
/// 获取远端地址
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetRemoteAddr( CGameSession* pSession );

///
/// 获取远端端口
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetRemotePort( CGameSession* pSession );

///
/// 获取远端地址
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetLocalAddr( CGameSession* pSession );

///
/// 获取远端端口
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetLocalPort( CGameSession* pSession );

///
/// 获取GameSession的GateID
/// [1/16/2015] create by jianglei.kinly
///
xgc_uint32 GetGateID( CGameSession* pSession );

///
/// 设置GameSession的GateID
/// [1/16/2015] create by albert.xu
///
xgc_void SetGateID( CGameSession* pSession, xgc_uint32 nGateID );

///
/// 获取GameSession的TransID
/// [1/16/2015] create by jianglei.kinly
///
xgc_uint32 GetTransID( CGameSession* pSession );

///
/// 设置GameSession的TransID
/// [1/16/2015] create by jianglei.kinly
///
xgc_void SetTransID( CGameSession* pSession, xgc_uint32 nTransID );

//////////////////////////////////////////////////////////////////////////
// GateSession 相关的网络函数
//////////////////////////////////////////////////////////////////////////
///
/// 设置用户数据
/// [12/1/2014] create by albert.xu
///
xgc_void SetUserdata( CGateSession* pSession, xgc_lpvoid pUserdata );

///
/// 获取用户数据
/// [12/1/2014] create by albert.xu
///
xgc_lpvoid GetUserdata( CGateSession* pSession );

///
/// 断开连接
/// [12/1/2014] create by albert.xu
///
xgc_void Disconnect( CGateSession* pSession );

///
/// 获取远端地址
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetRemoteAddr( CGateSession* pSession );

///
/// 获取远端端口
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetRemotePort( CGateSession* pSession );

///
/// 获取远端地址
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetLocalAddr( CGateSession* pSession );

///
/// 获取远端端口
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetLocalPort( CGateSession* pSession );

///
/// 设置Gate关联关系
/// [12/1/2014] create by albert.xu
///
xgc_void RegistGateSession( xgc_uint32 nIndex, CGateSession* pSession );

///
/// 获取GateSession
/// [12/1/2014] create by albert.xu
///
CGateSession* GetGateSession( xgc_uint32 nIndex );

///
/// 获取Gate的索引编号
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetGateIndex( CGateSession* pSession );

//////////////////////////////////////////////////////////////////////////
// 监控相关操作
//////////////////////////////////////////////////////////////////////////
xgc_void WriteToMonitor( xgc_lpcstr format, ... );

///
/// 获取是否开启了Monitor功能
/// [12/30/2014] create by jianglei.kinly
///
xgc_bool GetMonitorIsOpen();

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