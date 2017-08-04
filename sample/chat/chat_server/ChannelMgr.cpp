#include "header.h"
#include "Channel.h"
#include "ChannelMgr.h"


CChannelMgr::CChannelMgr()
{
}


CChannelMgr::~CChannelMgr()
{
}

CChannel * CChannelMgr::CreateChannel( const xgc_string &strChannelName )
{
	auto it = mChannelMap.find( strChannelName );
	if( it != mChannelMap.end() )
	{
		auto pChannel = CChannel::handle_exchange( it->second );
		if( pChannel )
			return pChannel;

		mChannelMap.erase( it );
	}

	auto pChannel = XGC_NEW CChannel( strChannelName );
	if( pChannel )
	{
		try
		{
			mChannelMap[strChannelName] = pChannel->handle();
			return pChannel;
		}
		catch( ... )
		{
			SAFE_DELETE( pChannel );
		}
	}
	return nullptr;
}

CChannelMgr & getChannelMgr()
{
	// TODO: insert return statement here
	static CChannelMgr inst;
	return inst;
}
