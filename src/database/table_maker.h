#pragma once
#ifndef _TABLE_MAKER_H_
#define _TABLE_MAKER_H_
#include "Database.h"

///
/// \brief 根据配置文件创建数据库表
///
/// \param conn 数据库连接对象
/// \param fconfig 数据库配置文件路径
/// \param environment 环境变量 格式为 key=val;key=val;...key=val
/// \author albert.xu
/// \date 2015/12/21 18:10
///
xgc_bool make_tables( xgc::sql::connection_cfg cfg, xgc_lpcstr fconfig, xgc_lpcstr environment = xgc_nullptr );

#endif // _TABLE_MAKER_H_