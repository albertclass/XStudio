#include "ServerDefines.h"
#include "ServerEvent.h"

/// 服务器事件类，用于管理事件
struct ServerEvent
{
	typedef std::vector< EventListener > listeners;
	typedef std::tuple< int, int > event_id;
	typedef std::map< event_id, listeners > observers;

	observers _observers;

	std::vector< EventObject* > post_event;
	std::mutex post_event_guard;
};

static ServerEvent G_ServerEvent;

xgc_bool RegistEventListener( int id, int event, const EventListener &invoke )
{
	auto key = ServerEvent::event_id( std::make_tuple( id, event ) );
	try
	{
		G_ServerEvent._observers[key].push_back( invoke );
	}
	catch( ... )
	{
		return false;
	}

	return true;
}

xgc_void EmitEvent( EventObject &evt )
{
	auto it = G_ServerEvent._observers.find( std::make_tuple( evt.id, evt.event ) );
	if( it != G_ServerEvent._observers.end() )
	{
		for( auto &invoke : it->second )
			invoke( evt );
	}
}

xgc_void PostEvent( EventObject *evt )
{
	std::lock_guard< std::mutex > _locker( G_ServerEvent.post_event_guard );
	G_ServerEvent.post_event.push_back( evt );
}

xgc_void ExecEvent()
{
	std::lock_guard< std::mutex > _locker( G_ServerEvent.post_event_guard );

	for( auto evt : G_ServerEvent.post_event )
	{
		EmitEvent( *evt );
	}

	G_ServerEvent.post_event.clear();
}

xgc_void FiniServerEvent()
{
	ExecEvent();
}