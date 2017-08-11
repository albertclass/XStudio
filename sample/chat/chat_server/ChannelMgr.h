#pragma once
#ifndef _CHANNEL_MGR_H_
#define _CHANNEL_MGR_H_

class CChannel;

class CChannelMgr
{
	friend CChannelMgr& getChannelMgr();
private:
	/// 频道映射表
	xgc_map< xgc_string, xgc_uint32 > mChannelMap;

private:
	CChannelMgr();
	~CChannelMgr();

public:
	///
	/// \brief 创建频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CChannel* CreateChannel( const xgc_string &strChannelName );

	///
	/// \brief 获取频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	CChannel* getChannelByName( const xgc_string &strChannelName );

	///
	/// \brief 加入频道
	/// \author albert.xu
	/// \date 2017/08/03
	///
	template< template< class, class > class _Container, template< class > class _Ax = xgc_allocator >
	_Container< CChannel*, _Ax< CChannel* > > getChannelByWildcard( const xgc_string &strWildcard )
	{
		_Container< CChannel*, _Ax< CChannel* > > C;
		auto it = std::back_inserter( C );

		for( auto &pair : mChannelMap )
		{
			if( string_match( strWildcard.c_str(), pair.first.c_str(), 1 ) )
			{
				auto pChannel = CChannel::handle_exchange( pair.second );
				if( pChannel )
					it = pChannel;
			}
		}

		return C;
	}

	///
	/// \brief 检查延迟删除
	/// \author albert.xu
	/// \date 2017/08/03
	///
	xgc_long eraseEmptyChannel();
};

CChannelMgr &getChannelMgr();

#endif // _CHANNEL_MGR_H_