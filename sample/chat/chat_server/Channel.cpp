#include "header.h"
#include "Channel.h"
#include "User.h"

CChannel::CChannel( const xgc_string &strName )
	: mName( strName )
	, mAutoDestory( true )
	, mAutoDestoryDelay( 10000 )
	, mAutoKickDelay( 5 * 60 * 1000 )
	, mPassword( "" )
{
}

CChannel::~CChannel()
{
}

///
/// \brief 进入频道
/// \author albert.xu
/// \date 2017/08/03
///
xgc_long CChannel::Enter( xgc_uint32 nChatID, xgc_lpcstr lpPassword )
{
	if( !mPassword.empty() && mPassword != ( lpPassword ? lpPassword : "" ) )
		return -1;

	// 检查用户是否存在
	CUser* pUser = CUser::handle_exchange( nChatID );
	if( xgc_nullptr == pUser )
		return -2;

	auto it = mUserConfigMap.find( nChatID );
	if( it != mUserConfigMap.end() )
		return 1;

	user newConfig;
	newConfig.forbid = false;
	newConfig.disconnect = false;
	newConfig.last_timestamp = current_time();

	auto ib = mUserConfigMap.insert( std::make_pair( nChatID, newConfig ) );
	if( false == ib.second )
		return -3;

	pUser->onEnterChannel( this );

	return 0;
}

///
/// \brief 离开频道
/// \author albert.xu
/// \date 2017/08/03
///
xgc_void CChannel::Leave( xgc_uint32 nChatID )
{
	auto it = mUserConfigMap.find( nChatID );
	if( it == mUserConfigMap.end() )
		return;

	mUserConfigMap.erase( it );

	// 检查用户是否存在
	CUser* pUser = CUser::handle_exchange( nChatID );
	if( xgc_nullptr == pUser )
		return;

	pUser->onLeaveChannel( this );

	if( mAutoDestory && mUserConfigMap.empty() )
	{
		// 将自己放到延迟删除列表中
		mDestoryTime = current_milliseconds() + mAutoDestoryDelay;
	}
}

///
/// \brief 检查活跃
/// \author albert.xu
/// \date 2017/08/03
///
xgc_void CChannel::eraseQuietUser()
{
	if( mAutoKickDelay == 0 )
		return;

	auto now = current_milliseconds();

	auto it = mUserConfigMap.begin();
	while( it != mUserConfigMap.end() )
	{
		// 用户已经失效的情况下
		auto id = it->first;
		auto &cfg = it->second;

		if( xgc_nullptr == CUser::handle_exchange( id ) )
		{
			cfg.disconnect = true;
		}

		if( now - cfg.last_timestamp > mAutoKickDelay )
		{
			it = mUserConfigMap.erase( it );
			continue;
		}

		++it;
	}
}

///
/// \brief 是否有效用户
/// \author albert.xu
/// \date 2017/08/03
///
xgc_bool CChannel::isValidUser( xgc_uint32 chat_id )
{
	auto it = mUserConfigMap.find( chat_id );
	if( it == mUserConfigMap.end() )
		return false;

	CUser* pUser = CUser::handle_exchange( chat_id );
	if( xgc_nullptr == pUser )
		return false;

	return true;
}

xgc_long CChannel::Chat( xgc_uint32 nChatID, xgc_lpcstr lpText, xgc_size nSize, xgc_uint32 nToken )
{
	auto it = mUserConfigMap.find( nChatID );
	if( it == mUserConfigMap.end() )
		return -1;

	auto im = it->second;
	im.last_timestamp = current_milliseconds();

	auto pUser = CUser::handle_exchange( nChatID );
	if( xgc_nullptr == pUser )
		return -2;

	chat::channel_chat_ntf ntf;
	ntf.set_token( nToken );
	ntf.set_channel_id( handle() );
	ntf.set_text( lpText, nSize );
	ntf.set_chat_id( nChatID );

	for( auto &pair : mUserConfigMap )
	{
		auto pUser = CUser::handle_exchange( pair.first );
		if( pUser )
		{
			pUser->Send( chat::MSG_USER_CHAT_NTF, ntf );
		}
	}

	return 0;
}
