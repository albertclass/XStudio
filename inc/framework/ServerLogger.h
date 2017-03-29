#pragma once
#ifndef _SERVER_LOGGER_H_
#define _SERVER_LOGGER_H_

//-----------------------------------//
// [1/13/2014 albert.xu]
// 初始化日志模块
//-----------------------------------//
xgc_bool InitializeLogger( ini_reader &ini );

//-----------------------------------//
// [1/13/2014 albert.xu]
// 清理日志模块
//-----------------------------------//
xgc_void FinializeLogger();

#endif
