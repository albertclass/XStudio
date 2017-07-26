#ifndef __SERVER_REFRESH_H__
#define __SERVER_REFRESH_H__

///
/// 初始化刷新服务 
/// @return 初始化结果
/// [1/16/2015] create by guqiwei.weir
///
xgc_bool InitServerRefresh( ini_reader &ini );

///
/// 载入刷新服务的配置
/// [1/20/2015] create by albert.xu
///
xgc_bool LoadServerRefresh();

///
/// 更新检查
/// [1/21/2015] create by albert.xu
///
xgc_void StepServerRefresh();

///
/// 处理刷新子系统事件
/// @param lpSystem 系统名称
/// @param lpContext 调用上下文
/// [1/21/2015] create by albert.xu
///
xgc_void UpdateServerRefresh( xgc_lpcstr lpSystem, xgc_lpvoid lpContext );

///
/// 结束刷新服务 
/// @return 初始化结果
/// [1/16/2015] create by guqiwei.weir
///
xgc_void FiniServerRefresh();
#endif