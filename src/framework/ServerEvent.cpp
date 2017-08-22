#include "ServerDefines.h"
#include "ServerEvent.h"

/// 可调用的事件对象
struct InvokeEventObject
{
	EventObject base;

	/// 延迟调用
	std::function< xgc_void() > invoke;
};

/// 服务器事件类，用于管理事件
class ServerEvent
{
private:
	typedef std::function< xgc_void( EventObject& ) > listener;
	typedef std::vector< listener > listeners;
	typedef std::tuple< int, int > event_id;
	typedef std::map< event_id, listeners > observers;

	observers _observers;

	std::vector< EventObject* > post_event;
	std::mutex post_event_guard;
public:
	xgc_bool RegistEventListener( int id, int event, listener &&invoke )
	{
		auto key = event_id( { id, event } );
		try
		{
			_observers[key].emplace_back( std::forward< listener >( invoke ) );
		}
		catch( ... )
		{
			return false;
		}

		return true;
	}

	xgc_void Emit( EventObject &evt )
	{
		auto it = _observers.find( { evt.id, evt.event } );
		if( it != _observers.end() )
		{
			for( auto &invoke : it->second )
				invoke( evt );
		}
	}

	xgc_void Post( EventObject *evt )
	{
		std::lock_guard< std::mutex > _locker( post_event_guard );
		post_event.push_back( evt );
	}

	xgc_void Exec()
	{
		std::lock_guard< std::mutex > _locker( post_event_guard );

		for( auto evt : post_event )
		{
			Emit( *evt );

		}

		post_event.clear();
	}
};

static ServerEvent* g_ServerEvent = xgc_nullptr;

xgc_bool InitServerEvent()
{
	g_ServerEvent = XGC_NEW ServerEvent;
	if( g_ServerEvent )
		return true;

	return false;
}

///
/// \brief 事件注册
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_bool RegistEventListener( int id, int event, std::function< xgc_void( EventObject& ) > &&invoke )
{
	XGC_ASSERT_RETURN( g_ServerEvent, false );
	return g_ServerEvent->RegistEventListener( id, event, std::forward< std::function< xgc_void( EventObject& ) > >( invoke ) );
}

///
/// \brief 触发事件（立即执行)
/// 
/// \author albert.xu
/// \date 2017/08/21
///
xgc_void EmitEvent( EventObject &evt )
{
	XGC_ASSERT_RETURN( g_ServerEvent, XGC_NONE );
}

xgc_void PostEvent( EventObject *evt )
{
	XGC_ASSERT_RETURN( g_ServerEvent, XGC_NONE );

	g_ServerEvent->Post( evt );
}

xgc_void ExecEvent()
{
	XGC_ASSERT_RETURN( g_ServerEvent, XGC_NONE );
	g_ServerEvent->Exec();
}

xgc_void FiniServerEvent()
{
	XGC_ASSERT_RETURN( g_ServerEvent, XGC_NONE );
	ExecEvent();
	SAFE_DELETE( g_ServerEvent );
}