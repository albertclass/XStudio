#include "stdafx.h"
#include "ServerMonitor.h"
#include "protomtserver.h"
#include "ServerNetwork.h"
#include "MessageHandler.h"
#include "ServerBase.h"

class CMonitorMsgData
{
private:
	xgc_string m_strServerID;             ///< �Լ���ServerID ��eg:2-1-1-1��
	xgc_string m_strServerName;           ///< �Լ���ServerName ��eg:MS��
	xgc_bool m_bReconnectHasSend;         ///< ����֮���Ƿ��͹���Ϣ��
	
	xgc_unordered_map<xgc_string, xgc_bool> m_mapTargetConnect; ///< �������������������������״̬�����MTServer����֮����ò�����Щ������
	xgc_map<std::pair<xgc_string, xgc_uint16>, xgc_bool > m_mapTargetIPConnect;  ///< �������������������IP���ӵ�״̬
	xgc_map<std::pair<xgc_string, xgc_uint16>, xgc_string > m_mapTargetIPPortToID;  ///< �������������������IP���ӵ�״̬
public:
	///
	/// ����������ʼ��֮ǰ��ʼ�������������Ǳ߿���ֱ�ӵ�InsertTargetConnect
	/// [12/31/2014] create by jianglei.kinly
	///
	xgc_void Init( xgc_lpcstr serverID, xgc_lpcstr serverName )
	{
		m_strServerID = serverID;
		m_strServerName = serverName;
		m_bReconnectHasSend = false;
	}
	xgc_void InsertTargetConnect( xgc_lpcstr targetID, xgc_bool isConnect )
	{
		m_mapTargetConnect[targetID] = isConnect;

		OnInsertTargetConnect( targetID, isConnect );
	}
	xgc_void InsertTargetIPConnect( xgc_lpcstr targetIP, xgc_uint16 targetPort, xgc_bool isConnect )
	{
		m_mapTargetIPConnect[std::make_pair( targetIP, targetPort )] = isConnect;

		OnInsertTargetIPConnect( targetIP, targetPort, isConnect );
	}
	xgc_void InsertTargetIPPortToID( xgc_lpcstr targetIP, xgc_uint16 targetPort, xgc_lpcstr targetID )
	{
		m_mapTargetIPPortToID[std::make_pair( targetIP, targetPort )] = targetID;

		OnInsertTargetIPPortToID( targetIP, targetPort, targetID );
	}
	xgc_unordered_map<xgc_string, xgc_bool>& GetTargetConnect()
	{
		return m_mapTargetConnect;
	}
	xgc_void OnConnect()
	{
		// ��������֮�����Լ�����Ϣ��MT
		MT_NAMEID_PKG_NTF stNtf = { 0 };
		strcpy_s( stNtf.szServerName, m_strServerName.c_str() );
		strcpy_s( stNtf.szServerID, m_strServerID.c_str() );
        vector<CPipeSession*> pPipeVec;
        GetPipeSession(ServerType::SD_MONITOR_SERVER, pPipeVec);
        for (auto pSession: pPipeVec)
        {
            if (pSession)
                SendPackage(pSession, BuildPackage(MT_NAMEID_ID_NTF, stNtf));
        }
		// ���ͻ����з���������Ϣ��MT
		for( auto& it : m_mapTargetConnect )
		{
			OnInsertTargetConnect( it.first.c_str(), it.second );
		}
		for( auto& it : m_mapTargetIPConnect )
		{
			OnInsertTargetIPConnect( it.first.first.c_str(), it.first.second, it.second );
		}
		for( auto& it : m_mapTargetIPPortToID )
		{
			OnInsertTargetIPPortToID( it.first.first.c_str(), it.first.second, it.second.c_str() );
		}
	}
private:
	xgc_void OnInsertTargetConnect( xgc_lpcstr targetID, xgc_bool isConnect )
	{
		// MT_CONSOLE_CONNECT_PKG_NTF
		MT_CONSOLE_CONNECT_PKG_NTF stNtf = { 0 };
		strncpy_s( stNtf.szTargetServerID, targetID, sizeof( stNtf.szTargetServerID ) );
		stNtf.byIsConnect = ( isConnect ? 1 : 0 );
        vector<CPipeSession*> pPipeVec;
        GetPipeSession(ServerType::SD_MONITOR_SERVER, pPipeVec);
        for (auto pSession : pPipeVec)
        {
            if (pSession)
                SendPackage(pSession, BuildPackage(MT_CONSOLE_CONNECT_ID_NTF, stNtf));
        }
	}
	xgc_void OnInsertTargetIPConnect( xgc_lpcstr targetIP, xgc_uint16 targetPort, xgc_bool isConnect )
	{
		MT_CONSOLE_CONNECT_IP_PKG_NTF stNtf = { 0 };
		strncpy_s( stNtf.szTargetServerIP, targetIP, sizeof( stNtf.szTargetServerIP ) );
		stNtf.wTargetServerPort = targetPort;
		stNtf.byIsConnect = ( isConnect ? 1 : 0 );
        vector<CPipeSession*> pPipeVec;
        GetPipeSession(ServerType::SD_MONITOR_SERVER, pPipeVec);
        for (auto pSession : pPipeVec)
        {
            if (pSession)
                SendPackage(pSession, BuildPackage(MT_CONSOLE_CONNECT_IP_ID_NTF, stNtf));
        }
	}
	xgc_void OnInsertTargetIPPortToID( xgc_lpcstr targetIP, xgc_uint16 targetPort, xgc_lpcstr targetID )
	{
		MT_IPPortID_PKG_NTF stNtf = { 0 };
		strncpy_s( stNtf.szServerIP, targetIP, sizeof( stNtf.szServerIP ) );
		stNtf.wServerPort = targetPort;
		strncpy_s( stNtf.szServerID, targetID, sizeof( stNtf.szServerID ) );
        vector<CPipeSession*> pPipeVec;
        GetPipeSession(ServerType::SD_MONITOR_SERVER, pPipeVec);
        for (auto pSession : pPipeVec)
        {
            if (pSession)
                SendPackage(pSession, BuildPackage(MT_IPPortID_ID_NTF, stNtf));
        }
	}
};

CMonitorMsgData& getMonitorMsgData()
{
	static CMonitorMsgData Inst;
	return Inst;
}

xgc_void InitializeMonitor( xgc_lpcstr serverName, xgc_lpcstr serverCore )
{
	getMonitorMsgData().Init( serverCore, serverName );
}

xgc_void MonitorConnect()
{
	getMonitorMsgData().OnConnect();
}

xgc_void MonitorConsoleConnect( xgc_lpcstr strTargetServerID, xgc_bool bIsConnect )
{
	getMonitorMsgData().InsertTargetConnect( strTargetServerID, bIsConnect );
}

xgc_void MonitorConsoleIPConnect( xgc_lpcstr strTargetServerIP, xgc_uint16 wTargetServerPort, xgc_bool bIsConnect )
{
	getMonitorMsgData().InsertTargetIPConnect( strTargetServerIP, wTargetServerPort, bIsConnect );
}

xgc_void MonitorConsoleIPPortToID( xgc_lpcstr strTargetServerIP, xgc_uint16 wTargetServerPort, xgc_lpcstr strTargetServerID )
{
	getMonitorMsgData().InsertTargetIPPortToID( strTargetServerIP, wTargetServerPort, strTargetServerID );
}
