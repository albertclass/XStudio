#ifndef _SERVER_BASE_SERVER_MONITOR_H_
#define _SERVER_BASE_SERVER_MONITOR_H_

///
/// 初始化MonitorBase
/// [1/4/2015] create by jianglei.kinly
///
xgc_void InitializeMonitor( xgc_lpcstr serverName, xgc_lpcstr serverCore );

///
/// 连接到MonitorServer时候
/// [1/4/2015] create by jianglei.kinly
///
xgc_void MonitorConnect();

///
/// Connect页面的消息
/// params: 连接到本服务器的其他服务器的ServerID
///         是否连接(true:连接 false:断开)
/// [11/26/2014] create by jianglei.kinly
///
xgc_void MonitorConsoleConnect( xgc_lpcstr strTargetServerID, xgc_bool bIsConnect );

///
/// GateSession用IP连接，单独做这个接口
/// [1/7/2015] create by jianglei.kinly
///
xgc_void MonitorConsoleIPConnect( xgc_lpcstr strTargetServerIP, xgc_uint16 wTargetServerPort, xgc_bool bIsConnect );

///
/// GateSession连接后拿到PipeID，使用这个接口
/// [1/7/2015] create by jianglei.kinly
///
xgc_void MonitorConsoleIPPortToID( xgc_lpcstr strTargetServerIP, xgc_uint16 wTargetServerPort, xgc_lpcstr strTargetServerID );

#endif  // _SERVER_BASE_SERVER_MONITOR_H_