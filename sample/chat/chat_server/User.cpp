#include "header.h"
#include "User.h"
#include "Channel.h"

CUser::CUser( xgc_uint64 nUserID )
	: mUserID( nUserID )
	, mNetHandle( INVALID_NETWORK_HANDLE )
{
}

CUser::~CUser()
{
}

///
/// \brief 设置角色服务器
/// \author albert.xu
/// \date 2017/08/03
///
xgc_lpcstr CUser::genToken()
{
	char Hex[] = "0123456789ABCDEF!@#$%^&*()-=<>?/\\~`";
	for( int i = 0; i < sizeof( mToken ) - 1; ++i )
	{
		auto idx = random_range( 0ULL, sizeof( Hex ) );
		XGC_ASSERT( idx < sizeof( mToken ) );
		mToken[i] = Hex[idx];
	}

	mToken[sizeof( mToken ) - 1] = '\0';

	return mToken;
}

///
/// \brief 检查角色Token
/// \author albert.xu
/// \date 2017/08/03
///
xgc_bool CUser::checkToken( const xgc_string &strToken )
{
	if( strToken.size() != sizeof( mToken ) - 1 )
		return false;

	return strToken == mToken;
}

///
/// \brief 角色加入频道
/// \author albert.xu
/// \date 2017/08/03
///
xgc_void CUser::onEnterChannel( CChannel* pChannel )
{
	if( xgc_nullptr == pChannel )
		return;

	auto ib = mChannelSet.insert( pChannel->handle() );
	XGC_ASSERT( ib.second );
}

///
/// \brief 角色离开频道
/// \author albert.xu
/// \date 2017/08/03
///
xgc_void CUser::onLeaveChannel( CChannel* pChannel )
{
	if( xgc_nullptr == pChannel )
		return;
	
	auto id = pChannel->handle();

	mChannelSet.erase( id );
}

xgc_void CUser::Send( xgc_uint16 msgid, ::google::protobuf::Message& msg )
{
	if( mNetHandle == INVALID_NETWORK_HANDLE )
		return;

#pragma pack(1)
	struct
	{
		MessageHeader h;
		char b[4096];
	} m;
#pragma pack()

	xgc_uint16 data_size = msg.ByteSize();
	xgc_uint16 pack_size = sizeof( MessageHeader ) + data_size;

	m.h.length = htons( pack_size );
	m.h.message = htons( chat::MSG_LOGIN_ACK );

	msg.SerializeToArray( m.b, sizeof( m.b ) );

	SendPacket( mNetHandle, &m, pack_size );
}
