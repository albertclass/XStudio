#include "header.h"
#include "User.h"
#include "UserMgr.h"

CUserMgr::CUserMgr()
{
}


CUserMgr::~CUserMgr()
{
}

///
/// \brief ½ÇÉ«µÇÂ½
/// \author albert.xu
/// \date 2017/08/03
///
xgc_long CUserMgr::UserLogin( xgc_uint64 nUserID, const xgc_string &strServer, const xgc_string &strNick, const xgc_string &strExtra )
{
	CUser* pUser = xgc_nullptr;

	auto it1 = mUserMap.find( nUserID );
	if( it1 != mUserMap.end() )
		return 1; // ½ÇÉ«ÒÑµÇÂ¼

	auto it2 = mNickMap.find( strNick );
	if( it2 != mNickMap.end() )
		return -1; // ½ÇÉ«êÇ³ÆÖØ¸´

	if( xgc_nullptr == pUser )
	{
		pUser = XGC_NEW CUser( nUserID );
	}
	
	if( xgc_nullptr == pUser )
	{
		return -2;
	}

	pUser->setServer( strServer );
	pUser->setNickName( strNick );
	pUser->setExtra( strExtra );

	// regist user
	try
	{
		mUserMap[nUserID] = pUser->handle();
		mNickMap[strNick] = pUser->handle();
	}
	catch( ... )
	{
		mUserMap.erase( nUserID );
		mNickMap.erase( strNick );
		return -3;
	}

	return 0;
}


///
/// \brief ½ÇÉ«µÇ³ö
/// \author albert.xu
/// \date 2017/08/03
///
xgc_void CUserMgr::UserLogout( xgc_uint64 nUserID )
{
	auto it1 = mUserMap.find( nUserID );
	if( it1 != mUserMap.end() )
	{
		CUser* pUser = CUser::handle_exchange( it1->second );
		auto it2 = mNickMap.find( pUser->getNickName() );
		if( it2 != mNickMap.end() )
			mNickMap.erase( it2 );

		SAFE_DELETE( pUser );
		mUserMap.erase( it1 );
	}
}

///
/// \brief ½ÇÉ«µÇ³ö
/// \author albert.xu
/// \date 2017/08/03
///
CUser* CUserMgr::GetUserByUID( xgc_uint64 nUserID )
{
	auto it = mUserMap.find( nUserID );
	if( it != mUserMap.end() )
	{
		return CUser::handle_exchange( it->second );
	}

	return xgc_nullptr;
}

CUserMgr & getUserMgr()
{
	// TODO: insert return statement here
	static CUserMgr inst;
	return inst;
}
