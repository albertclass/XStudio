#ifndef __SERVER_EVENT_H__
#define __SERVER_EVENT_H__

/// 事件对象
struct EventObject
{
	/// 系统ID
	int id;
	/// 事件ID
	int event;
	/// 执行结果  < 0 - 执行错误， = 0 执行成功， > 0 执行警告
	int result;
};

template< class _EventType >
_EventType* make_event( int id, int event )
{
	return XGC_NEW _EventType{ id, event, 0 };
}

///
/// \brief 事件注册
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_bool InitServerEvent();

///
/// \brief 事件注册
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_bool RegistEventListener( int id, int event, std::function< xgc_void( EventObject& ) > &&invoke );

///
/// \brief 触发事件（立即执行)
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void EmitEvent( EventObject &evt );

///
/// \brief 投递事件（延迟执行）
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void PostEvent( EventObject *evt );

///
/// \brief 执行事件
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void ExecEvent();

///
/// \brief 清理事件系统
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void FiniServerEvent();

#endif // __SERVER_EVENT_H__