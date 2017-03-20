#pragma once
#ifndef _SERVER_CONSOLE_H_
#define _SERVER_CONSOLE_H_

#include "ini.h"

//-----------------------------------//
// [1/13/2014 albert.xu]
// ��ʼ������̨ģ��
//-----------------------------------//
xgc_void InitializeConsole( XGC::common::IniFile &ini );

//-----------------------------------//
// [1/13/2014 albert.xu]
// �������̨ģ��
//-----------------------------------//
xgc_void FinializeConsole();

//-----------------------------------//
// [1/13/2014 albert.xu]
// ��Ļ��־��ӡ
//-----------------------------------//
xgc_void ConsoleLog( xgc_lpcstr pszLogText );

//-----------------------------------//
// [3/5/2014 albert.xu]
// ��Ļ�߼�
//-----------------------------------//
xgc_int32 ConsoleUpdate();

#endif // _SERVER_CONSOLE_H_
