#ifndef __SERVER_ASYNCEVT_H__
#define __SERVER_ASYNCEVT_H__

struct EventObject
{
	int id;
	int event;
};

//template< class R, class T... >
//R Emit( const xgc_string &system, const xgc_string &event, T... )
//{
//	auto invoke = GetInvoke( system, event );
//	return invoke( std::forward< T >( ...T ) );
//}

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
xgc_void PostServerEvent( const std::function<xgc_void()>& func );

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