#pragma once
#ifndef _SERVER_LOGGER_H_
#define _SERVER_LOGGER_H_

//-----------------------------------//
// [1/13/2014 albert.xu]
// ��ʼ����־ģ��
//-----------------------------------//
xgc_bool InitializeLogger( IniFile &ini );

//-----------------------------------//
// [1/13/2014 albert.xu]
// ������־ģ��
//-----------------------------------//
xgc_void FinializeLogger();

//-----------------------------------//
// [1/20/2014 albert.xu]
// д��־
//-----------------------------------//
xgc_void WriteServerLog( xgc_lpcstr text );

///
/// ��ȡд��־��·����shmlog�ã�
/// [1/12/2015] create by jianglei.kinly
///
xgc_lpcstr GetLogPath();

#endif
