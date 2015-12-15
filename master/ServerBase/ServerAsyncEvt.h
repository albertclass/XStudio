#ifndef __SERVER_ASYNCEVT_H__
#define __SERVER_ASYNCEVT_H__

///
/// 初始化刷新服务 
/// @return 初始化结果
/// [1/16/2015] create by zhangyupeng
///
xgc_bool InitServerAsyncEvt();

///
/// 投递异步事件事件
/// [1/20/2015] create by zhangyupeng
///
xgc_void PostServerEvent( const function<xgc_void()>& func );

///
/// 处理异步事件事件 
/// @return 初始化结果
/// [1/16/2015] create by zhangyupeng
///
xgc_void StepServerEvent();

///
/// 结束前处理队列里面的所有事件 
/// @return 初始化结果
/// [1/16/2015] create by zhangyupeng
///
xgc_void FiniServerAsyncEvt();

#endif