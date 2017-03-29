#pragma once
#ifndef _SERVER_EVENT_LOG_H_
#define _SERVER_EVENT_LOG_H_

///
/// ��ʼ��EventLog
/// [12/12/2014] create by albert.xu
///
xgc_bool InitializeEventLog( ini_reader &ini );

///
/// д�¼���־�������ڴ���
/// [12/12/2014] create by albert.xu
///
xgc_bool LogEvent( xgc_lpvoid pData, xgc_size nSize );

///
/// д�¼���־�������ڴ���
/// [12/12/2014] create by albert.xu
///
template< typename Package >
xgc_bool LogEvent( xgc_uint16 nEvent, const Package& stEvent )
{
	return LogEvent( BuildEventLog( nEvent, stEvent ) );
}

///
/// �ؼ�����־
/// [12/12/2014] create by albert.xu
///
xgc_bool LogKeyFlow( xgc_lpcstr lpUserName, xgc_uint32 nType );

///
/// ����EventLog
/// [12/12/2014] create by albert.xu
///
xgc_void FinializeEventLog();
#endif // _SERVER_EVENT_LOG_H_