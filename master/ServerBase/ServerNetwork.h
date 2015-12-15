#pragma once

#include "ServerDefines.h"

///
/// [12/30/2013 albert.xu]
/// 初始化网络
///
xgc_bool InitializeNetwork( IniFile &ini );

///
/// [2/17/2014 albert.xu]
/// 处理网络数据
///
xgc_bool ProcessNetwork();

///
/// [12/30/2013 albert.xu]
/// 终止网络
///
xgc_void FinializeNetwork();

///
/// 初始化Monitor网络
/// [12/30/2014] create by jianglei.kinly
///
xgc_bool InitializeMTNetwork( IniFile& ini );