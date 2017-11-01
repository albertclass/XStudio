#pragma once
#ifndef _SERVER_LOGGER_H_
#define _SERVER_LOGGER_H_

///
/// \brief 初始化日志模块
/// \date 1/13/2014 
/// \author albert.xu
///
xgc_bool InitializeLogger( ini_reader &ini );

///
/// \brief 清理化日志模块
/// \date 1/13/2014 
/// \author albert.xu
///
xgc_void FinializeLogger();

#endif
