#pragma once
#ifndef _SERVER_EVENT_LOG_H_
#define _SERVER_EVENT_LOG_H_

namespace XGC
{
	namespace common
	{
		class COMMON_API IniFile;
	}
}

///
/// ��ʼ��EventLog
/// [12/12/2014] create by albert.xu
///
xgc_bool InitializeEventLog( IniFile &ini );

///
/// д�¼���־�������ڴ���
/// [12/12/2014] create by albert.xu
///
xgc_bool LogEvent( xgc_lpvoid lpData );

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