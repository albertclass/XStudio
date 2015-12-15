#pragma once
#ifndef _SERVER_LOGGER_H_
#define _SERVER_LOGGER_H_

//-----------------------------------//
// [1/13/2014 albert.xu]
// 初始化日志模块
//-----------------------------------//
xgc_bool InitializeLogger( IniFile &ini );

//-----------------------------------//
// [1/13/2014 albert.xu]
// 清理日志模块
//-----------------------------------//
xgc_void FinializeLogger();

//-----------------------------------//
// [1/20/2014 albert.xu]
// 写日志
//-----------------------------------//
xgc_void WriteServerLog( xgc_lpcstr text );

///
/// 获取写日志的路径（shmlog用）
/// [1/12/2015] create by jianglei.kinly
///
xgc_lpcstr GetLogPath();

#endif
