#ifndef __SERVER_REFRESH_H__
#define __SERVER_REFRESH_H__

///
/// 需要用户实现，处理刷新事件
/// @param lpInvoke 调用参数
/// @param nPrevInvokeTime 上一次调用时间
/// @param lpContext 调用上下文
/// [1/20/2015] create by albert.xu
///
xgc_void OnServerRefresh( xgc_lpcstr lpInvoke, datetime dtPrevInvokeTime, xgc_lpvoid lpContext );

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
/// 结束刷新服务 
/// @return 初始化结果
/// [1/16/2015] create by guqiwei.weir
///
xgc_void FiniServerRefresh();
#endif