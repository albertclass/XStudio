#ifndef __SERVER_REFRESH_H__
#define __SERVER_REFRESH_H__

///
/// 初始化刷新服务 
/// @return 初始化结果
/// [1/16/2015] create by guqiwei.weir
///
xgc_bool InitServerRefresh( ini_reader &ini );

///
/// 更新检查
/// [1/21/2015] create by albert.xu
///
xgc_void ExecServerRefresh( datetime now );

///
/// 结束刷新服务 
/// @return 初始化结果
/// [1/16/2015] create by guqiwei.weir
///
xgc_void FiniServerRefresh();

#endif