#ifndef _SERVER_BASE_SERVER_MONITOR_H_
#define _SERVER_BASE_SERVER_MONITOR_H_

///
/// ��ʼ��MonitorBase
/// [1/4/2015] create by jianglei.kinly
///
xgc_void InitializeMonitor( xgc_lpcstr serverName, xgc_lpcstr serverCore );

///
/// ���ӵ�MonitorServerʱ��
/// [1/4/2015] create by jianglei.kinly
///
xgc_void MonitorConnect();

///
/// Connectҳ�����Ϣ
/// params: ���ӵ�����������������������ServerID
///         �Ƿ�����(true:���� false:�Ͽ�)
/// [11/26/2014] create by jianglei.kinly
///
xgc_void MonitorConsoleConnect( xgc_lpcstr strTargetServerID, xgc_bool bIsConnect );

///
/// GateSession��IP���ӣ�����������ӿ�
/// [1/7/2015] create by jianglei.kinly
///
xgc_void MonitorConsoleIPConnect( xgc_lpcstr strTargetServerIP, xgc_uint16 wTargetServerPort, xgc_bool bIsConnect );

///
/// GateSession���Ӻ��õ�PipeID��ʹ������ӿ�
/// [1/7/2015] create by jianglei.kinly
///
xgc_void MonitorConsoleIPPortToID( xgc_lpcstr strTargetServerIP, xgc_uint16 wTargetServerPort, xgc_lpcstr strTargetServerID );

#endif  // _SERVER_BASE_SERVER_MONITOR_H_