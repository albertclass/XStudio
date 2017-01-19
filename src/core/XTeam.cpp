#include "StdAfx.h"
#include "XTeam.h"
#include "XCharactor.h"

namespace XGC
{
	static XTeam::Member default_member = { xGlobal(INVALID_GLOBAL_ID), xObject(INVALID_OBJECT_ID), XVector2(), false };
	XTeam::XTeam( xgc_uint32 teamID, xgc_size nMemberCount, xgc_bool bSpinLockCount )
		: mTeamID(teamID)
		, mLeader( INVALID_GLOBAL_ID )
		, mMembers( nMemberCount, default_member )
		, mSpinLockCount( bSpinLockCount )
		, mUserdata( 0 )
		, mDestructor( xgc_nullptr )
	{
	}

	XTeam::~XTeam(void)
	{
		if( mDestructor )
			mDestructor( mUserdata );

		mUserdata = 0;
	}

	xgc_bool XTeam::appendMember( xGlobal hGlobal, xObject hMember, xgc_bool isOnline, 
		xgc_bool bLeader /*= false*/, const XVector2& vRelatively /*= XVector2::ZERO*/,
		bool ntfNearbyPlayers /*= true */)
	{
		// XGC_ASSERT_RETURN( hMember != INVALID_OBJECT_ID, false );

		Member member( hGlobal, hMember, vRelatively, isOnline );
		MemberVec::iterator emptySlot = mMembers.end();
		for( MemberVec::iterator iter = mMembers.begin(); iter != mMembers.end(); ++iter )
		{
			if( emptySlot == mMembers.end() && iter->hGlobal == INVALID_GLOBAL_ID )
				emptySlot = iter;

			if( iter->hGlobal == hGlobal )
			{
				if( bLeader ) mLeader = hGlobal;
				return true;
			}
		}

		if( emptySlot == mMembers.end() && mSpinLockCount )
		{
			return false;
		}
		else if( emptySlot == mMembers.end() && !mSpinLockCount )
		{
			MemberVec::size_type size = mMembers.size();
			mMembers.resize( mMembers.size() + 1, default_member );
			emptySlot = mMembers.begin() + size;
			*emptySlot = member;
		}
		else
		{
			*emptySlot = member;
		}

		if( bLeader )
		{
			mLeader = hGlobal;
			mLeaderObject = hMember;
		}

		XCharactor* pActor = static_cast< XCharactor* >( GetXObject( hMember, TypeXCharactor ) );
		if( pActor )
		{
			pActor->mTeamPtr = shared_from_this();
			pActor->mTeamSlot = (xgc_uint16)(emptySlot - mMembers.begin());
		}
		if( ntfNearbyPlayers )
		{
			broadcastHeaderIDToNearbyers( hGlobal, getHeaderID() );
		}
		return true;
	}

	xgc_void XTeam::removeMember( xGlobal hGlobal/*, xgc_bool bChangeLeader *//*= true*/ )
	{
		XGC_ASSERT_RETURN( hGlobal != INVALID_GLOBAL_ID, void(0) );

		// 防止提前删除自身导致的野指针问题。
		XTeamPtr thisTeam = shared_from_this();

		// 删除成员
		Member MatchValue = { hGlobal, 0, XVector2::ZERO, false };
		MemberVec::iterator iter = std::find( mMembers.begin(), mMembers.end(), MatchValue );
		if( iter != mMembers.end() )
		{
			XCharactor* pActor = static_cast< XCharactor* >( GetXObject( iter->hMember, TypeXCharactor ) );
			if( pActor )
			{
				pActor->mTeamPtr.reset();
			}

			iter->hGlobal = INVALID_GLOBAL_ID;
			iter->hMember = INVALID_OBJECT_ID;
			iter->vRelatively = XVector2::ZERO;
		}

		broadcastHeaderIDToNearbyers( hGlobal, 0 );
		// 替换队长
		/*if( hGlobal == mLeader && bChangeLeader )
		{
			for( iter = mMembers.begin(); iter != mMembers.end(); ++iter )
			{
				if( iter->hGlobal != INVALID_GLOBAL_ID )
				{
					mLeader = iter->hGlobal;
					mLeaderObject = iter->hMember;
					break;
				}
			}
		}*/
	}

	XVector2 XTeam::getTeamSlotPosition( xgc_uint32 nSlot )const
	{
		XGC_ASSERT_RETURN( nSlot < mMembers.size(), XVector2::ZERO );

		XCharactor* pLeader = static_cast< XCharactor* >( GetXObject( getHeaderObject(), TypeXCharactor ) );
		if( pLeader )
		{
			const XVector2& vDir = pLeader->GetDirection();
			xgc_real32 rad_xy = XMath::ATan2( vDir.y, vDir.x );
			float x = mMembers[nSlot].vRelatively.x*XMath::Cos( rad_xy ) - mMembers[nSlot].vRelatively.y*XMath::Sin( rad_xy );
			float y = mMembers[nSlot].vRelatively.x*XMath::Sin( rad_xy ) + mMembers[nSlot].vRelatively.y*XMath::Cos( rad_xy );
			return XVector2(x,y);
		}

		return XVector2::ZERO;
	}

	xgc_bool XTeam::setHeader( xGlobal hGlobal, xObject hObj )
	{
		/*Member MatchValue = { hGlobal, 0, XVector2::ZERO, false };
		auto iter = std::find( mMembers.begin(), mMembers.end(), MatchValue );
		if( iter != mMembers.end() )
		{
		mLeader = iter->hGlobal;
		mLeaderObject = iter->hMember;

		return true;
		}*/
		mLeader = hGlobal;
		mLeaderObject = hObj;
		return false;
	}

	xgc_void XTeam::resetLeader()
	{
		mLeader = INVALID_GLOBAL_ID;
		mLeaderObject = INVALID_OBJECT_ID;
	}

	xgc_void XTeam::clearMembers()
	{
		for (auto it : mMembers)
		{
			it.hGlobal = INVALID_GLOBAL_ID;
			it.hMember = INVALID_OBJECT_ID;
			it.isOnline = false;
		}
	}


	xgc_bool XTeam::dismissed()
	{
		FUNCTION_BEGIN;
		for( auto it : mMembers )
		{
			XCharactor* pActor = ObjectCast<XCharactor>( it.hMember );
			if( !pActor )
			{
				continue;
			}
			pActor->mTeamPtr.reset();

			broadcastHeaderIDToNearbyers( it.hGlobal, 0 );

			it = default_member;
		}
		setHeader(INVALID_GLOBAL_ID, INVALID_OBJECT_ID);

		return true;
		FUNCTION_END;
		return false;
	}


	xgc_bool XTeam::changeHeader( xGlobal headerXCBID )
	{
		FUNCTION_BEGIN;
		xGlobal oldHeadXCBID = getHeaderID();
		
		auto newMember = getMemberByID( headerXCBID );
		if( !newMember )
		{
			return false;
		}

		setHeader( headerXCBID, newMember->hMember );
		broadcastHeaderIDToNearbyers( headerXCBID, headerXCBID );

		auto oldMember = getMemberByID( oldHeadXCBID );
		if( oldMember && oldMember->hMember != INVALID_OBJECT_ID )
		{
			broadcastHeaderIDToNearbyers( oldHeadXCBID, headerXCBID );
		}


		return true;
		FUNCTION_END;
		return false;
	}

	/*xgc_bool XTeam::memberOnline( xGlobal xcbID )
	{
		FUNCTION_BEGIN;
		auto pMember = getMemberByID( xcbID );
		if( !pMember )
		{
			return false;
		}

		pMember->isOnline = true;
		
		XCharactor* pActor = ObjectCast<XCharactor>( pMember->hMember );
		if( !pActor )
		{
			return true;
		}
		pMember->hMember = pActor->GetObjectID();
		pActor->mTeamPtr = shared_from_this();
		return true;
		FUNCTION_END;
		return false;
	}

	xgc_bool XTeam::memberOffline( xgc_uint64 xcbID )
	{
		FUNCTION_BEGIN;
		auto pMember = getMemberByID( xcbID );
		if( !pMember )
		{
			return true;
		}

		pMember->isOnline = false;
		XCharactor* pActor = ObjectCast<XCharactor>( pMember->hMember );
		if( !pActor )
		{
			return true;
		}
		pActor->mTeamPtr.reset();
		return true;
		FUNCTION_END;
		return false;
	}*/
}
