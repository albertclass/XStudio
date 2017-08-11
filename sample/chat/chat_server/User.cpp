#include "header.h"
#include "User.h"
#include "Channel.h"

#include "ClientSession.h"

CUser::CUser( xgc_uint64 nUserID )
	: mUserID( nUserID )
	, mClientSession( xgc_nullptr )
{
}

CUser::~CUser()
{
}

///
/// \brief ���ý�ɫ������
/// \author albert.xu
/// \date 2017/08/03
///
xgc_lpcstr CUser::genToken()
{
	char Hex[] = "0123456789ABCDEF!@#$%^&*()-=<>?/~`";
	for( int i = 0; i < sizeof( mToken ) - 1; ++i )
	{
		auto idx = random_range( 0ULL, sizeof( Hex ) - 2 );
		XGC_ASSERT( idx < sizeof( Hex ) - 1 );
		mToken[i] = Hex[idx];
	}

	mToken[sizeof( mToken ) - 1] = '\0';

	return mToken;
}

///
/// \brief ����ɫToken
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
/// \brief ��ɫ����Ƶ��
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
/// \brief ��ɫ�뿪Ƶ��
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
	if( xgc_nullptr == mClientSession )
		return;

	mClientSession->Send2Client( msgid, msg );
}
