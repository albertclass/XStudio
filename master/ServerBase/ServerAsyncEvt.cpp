#include "stdafx.h"
#include "ServerBase.h"
#include "ServerAsyncEvt.h"

/// @var ╤сап
static xgc_queue< function<xgc_void()> > gAsyncEvt;
static sync_section mSecAsyncEvt;

xgc_bool InitServerAsyncEvt()
{
	return true;
}

xgc_void PostServerEvent( const function<xgc_void()>& func )
{
	autolock lock( mSecAsyncEvt );
	gAsyncEvt.push( std::move( func ) );
}

xgc_void StepServerEvent()
{
	autolock lock( mSecAsyncEvt );
	while( !gAsyncEvt.empty() )
	{
		auto postCall = gAsyncEvt.front();
		if( postCall )
			postCall();
		gAsyncEvt.pop();
	}
}

xgc_void FiniServerAsyncEvt()
{
	StepServerEvent();
}