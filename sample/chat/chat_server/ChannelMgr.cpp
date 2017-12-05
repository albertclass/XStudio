#include "header.h"
#include "Channel.h"
#include "ChannelMgr.h"


CChannelMgr::CChannelMgr()
{
}


CChannelMgr::~CChannelMgr()
{
	for( auto &pair : mChannelMap )
	{
		auto channel_id = pair.second;
		auto channel = CChannel::handle_exchange( channel_id );
		delete channel;
	}
}

CChannel * CChannelMgr::CreateChannel( const xgc::string &strChannelName )
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
			auto channel_id = pChannel->handle();
			mChannelMap[strChannelName] = channel_id;

			return pChannel;
		}
		catch( ... )
		{
			SAFE_DELETE( pChannel );
		}
	}
	return nullptr;
}

///
/// \brief 获取频道
/// \author albert.xu
/// \date 2017/08/03
///

CChannel * CChannelMgr::getChannelByName( const xgc::string & strChannelName )
{
	auto it = mChannelMap.find( strChannelName );
	if( it == mChannelMap.end() )
		return xgc_nullptr;

	return CChannel::handle_exchange( it->second );
}

///
/// \brief 检查延迟删除
/// \author albert.xu
/// \date 2017/08/03
///
xgc_long CChannelMgr::eraseEmptyChannel()
{
	xgc_long eraseCount = 0;

	auto it = mChannelMap.begin();
	auto now = current_milliseconds();

	while( it != mChannelMap.end() )
	{
		auto channel_id = it->second;
		auto channel = CChannel::handle_exchange( channel_id );
		if( xgc_nullptr == channel )
		{
			++eraseCount;
			it = mChannelMap.erase( it );
			continue;
		}

		channel->eraseQuietUser();

		if( channel->getDestoryTime() > now )
		{
			++eraseCount;
			XGC_ASSERT( channel->getUserCount() == 0 );
			it = mChannelMap.erase( it );

			delete channel;
			continue;
		}

		++it;
	}

	return eraseCount;
}

CChannelMgr & getChannelMgr()
{
	// TODO: insert return statement here
	static CChannelMgr inst;
	return inst;
}
