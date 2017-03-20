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
/// ��ʼ��������
/// [11/26/2014] create by albert.xu
/// @param lpConfigPath �����ļ�·��
/// @param InitConfiguration ���ó�ʼ���ص�
/// @param lpParam ���ó�ʼ������
///
xgc_bool InitServer( xgc_lpcstr lpConfigPath, xgc_bool( *InitConfiguration )( IniFile &, xgc_lpvoid ), xgc_lpvoid lpParam );

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
/// �������״̬
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceStatus( xgc_uint32 nState, xgc_uint32 nExitCode, xgc_uint32 nWaitHint );

///
/// ����������¼�
/// @param nEventType EVENTLOG_
/// @param nErrorCode ServerService.h defined.
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceEvent( xgc_uint16 nEventType, xgc_uint32 nErrorCode, xgc_lpcstr lpInfomation );

///
/// �������Ƿ���ֹͣ
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerStoped();

///
/// �������Ƿ�����ͣ
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServerPaused();

///
/// �Ƿ�ͨ����������
/// [12/5/2014] create by albert.xu
///
xgc_bool IsServerService();

///
/// ��ȡ��������
/// [11/27/2014] create by albert.xu
///
xgc_lpcstr GetServerName();

///
/// ת��������IDΪ��ֵ
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeType( xgc_uint32 nPipeID );

///
/// ת��������IDΪ��ֵ
/// [11/27/2014] create by albert.xu
///
xgc_byte GetPipeIndex( xgc_uint32 nPipeID );

///
/// ��ȡ����·��
/// [11/29/2014] create by albert.xu
///
xgc_lpcstr GetConfPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

///
///  ��ȡ����·��
/// [1/4/2015] create by wuhailin.jerry
///
xgc_string GetConfPath( xgc_lpcstr lpRelativePath, ... );

///
/// ��ȡ����·��
/// [8/6/2015] create by jianglei.kinly
///
xgc_string LuaGetConfPath( xgc_lpcstr lpRelativePath );

///
/// ��ȡ��־·��
/// [12/3/2014] create by albert.xu
///
xgc_lpcstr GetLogPath( xgc_lpstr szPath, xgc_size nSize, xgc_lpcstr lpRelativePath, ... );

//////////////////////////////////////////////////////////////////////////
// ������صĺ���
//////////////////////////////////////////////////////////////////////////
// �ܵ���ص����纯��
///
/// ��ȡPipeSession
/// [12/1/2014] create by albert.xu
///
CPipeSession* GetPipeSession( ServerType eType, xgc_byte byIndex = 1 );

///
/// ��ȡPipeSession
/// [12/9/2014] create by albert.xu
///
CPipeSession* GetPipeSession( xgc_uint32 nPipeID );

void GetPipeSession(ServerType eType, vector<CPipeSession*>&);

///
/// ���ݹ���������ȡ���������Ӷ���
/// [2/20/2014 albert.xu]
///
CPipeSession* GetPipeSession( const std::function< xgc_bool( xgc_uint32 ) > &fnFilter );

///
/// ��ȡPipeSession��PipeID
/// [12/9/2014] create by albert.xu
///
xgc_uint32 GetPipeID( CPipeSession* pSession );

///
/// ��ȡPipeSession��PipeID���ַ�����ʽ��
/// [12/11/2014] create by albert.xu
///
xgc_lpcstr GetPipeID( CPipeSession* pSession, xgc_lpstr lpOutput, xgc_size nSize = 0 );

//////////////////////////////////////////////////////////////////////////
// GameSession ��ص����纯��
//////////////////////////////////////////////////////////////////////////
///
/// �����û�����
/// [12/1/2014] create by albert.xu
///
xgc_void SetUserdata( CGameSession* pSession, xgc_lpvoid pUserdata );

///
/// ��ȡ�û�����
/// [12/1/2014] create by albert.xu
///
xgc_lpvoid GetUserdata( CGameSession* pSession );

///
/// �Ͽ�����
/// [12/1/2014] create by albert.xu
///
xgc_void Disconnect( CGameSession* pSession );

///
/// ��ȡԶ�˵�ַ
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetRemoteAddr( CGameSession* pSession );

///
/// ��ȡԶ�˶˿�
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetRemotePort( CGameSession* pSession );

///
/// ��ȡԶ�˵�ַ
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetLocalAddr( CGameSession* pSession );

///
/// ��ȡԶ�˶˿�
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetLocalPort( CGameSession* pSession );

///
/// ��ȡGameSession��GateID
/// [1/16/2015] create by jianglei.kinly
///
xgc_uint32 GetGateID( CGameSession* pSession );

///
/// ����GameSession��GateID
/// [1/16/2015] create by albert.xu
///
xgc_void SetGateID( CGameSession* pSession, xgc_uint32 nGateID );

///
/// ��ȡGameSession��TransID
/// [1/16/2015] create by jianglei.kinly
///
xgc_uint32 GetTransID( CGameSession* pSession );

///
/// ����GameSession��TransID
/// [1/16/2015] create by jianglei.kinly
///
xgc_void SetTransID( CGameSession* pSession, xgc_uint32 nTransID );

//////////////////////////////////////////////////////////////////////////
// GateSession ��ص����纯��
//////////////////////////////////////////////////////////////////////////
///
/// �����û�����
/// [12/1/2014] create by albert.xu
///
xgc_void SetUserdata( CGateSession* pSession, xgc_lpvoid pUserdata );

///
/// ��ȡ�û�����
/// [12/1/2014] create by albert.xu
///
xgc_lpvoid GetUserdata( CGateSession* pSession );

///
/// �Ͽ�����
/// [12/1/2014] create by albert.xu
///
xgc_void Disconnect( CGateSession* pSession );

///
/// ��ȡԶ�˵�ַ
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetRemoteAddr( CGateSession* pSession );

///
/// ��ȡԶ�˶˿�
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetRemotePort( CGateSession* pSession );

///
/// ��ȡԶ�˵�ַ
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetLocalAddr( CGateSession* pSession );

///
/// ��ȡԶ�˶˿�
/// [6/28/2015] create by albert.xu
///
xgc_uint16 GetLocalPort( CGateSession* pSession );

///
/// ����Gate������ϵ
/// [12/1/2014] create by albert.xu
///
xgc_void RegistGateSession( xgc_uint32 nIndex, CGateSession* pSession );

///
/// ��ȡGateSession
/// [12/1/2014] create by albert.xu
///
CGateSession* GetGateSession( xgc_uint32 nIndex );

///
/// ��ȡGate���������
/// [6/28/2015] create by albert.xu
///
xgc_uint32 GetGateIndex( CGateSession* pSession );

//////////////////////////////////////////////////////////////////////////
// �����ز���
//////////////////////////////////////////////////////////////////////////
xgc_void WriteToMonitor( xgc_lpcstr format, ... );

///
/// ��ȡ�Ƿ�����Monitor����
/// [12/30/2014] create by jianglei.kinly
///
xgc_bool GetMonitorIsOpen();

//////////////////////////////////////////////////////////////////////////
// ˢ��ϵͳ���ú���
//////////////////////////////////////////////////////////////////////////

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