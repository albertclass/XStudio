#include "ServerDefines.h"
#include "ServerBase.h"
#include "ServerAsyncEvt.h"

/// @var ╤сап
struct ServerEvent
{
	xgc_queue< function<xgc_void()> > gAsyncEvt;
	std::mutex mSecAsyncEvt;
};

static struct ServerEvent * g_ServerEvent = xgc_nullptr;

xgc_bool InitServerAsyncEvt()
{
	g_ServerEvent = XGC_NEW ServerEvent;
	if( g_ServerEvent )
		return true;

	return false;
}

xgc_void PostServerEvent( const function<xgc_void()>& func )
{
	std::lock_guard< std::mutex > lock( g_ServerEvent->mSecAsyncEvt );
	g_ServerEvent->gAsyncEvt.push( std::move( func ) );
}

xgc_void StepServerEvent()
{
	if( xgc_nullptr == g_ServerEvent )
		return;

	std::lock_guard< std::mutex > lock( g_ServerEvent->mSecAsyncEvt );
	while( !g_ServerEvent->gAsyncEvt.empty() )
	{
		auto postCall = g_ServerEvent->gAsyncEvt.front();
		if( postCall )
			postCall();
		g_ServerEvent->gAsyncEvt.pop();
	}
}

xgc_void FiniServerAsyncEvt()
{
	StepServerEvent();
	SAFE_DELETE( g_ServerEvent );
}