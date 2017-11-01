#include "XHeader.h"
#include "XTeam.h"
#include "XCharactor.h"

namespace xgc
{
	static XTeam::Member default_member = 
	{ 
		INVALID_GLOBAL_ID, 
		INVALID_OBJECT_ID, 
		XVector2(),
		false 
	};

	XTeam::XTeam( xgc_uint32 teamID, xgc_size nMemberCount, xgc_bool bSpinLockCount )
		: mTeamID(teamID)
		, mLeaderGlobal( INVALID_GLOBAL_ID )
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

	xgc_bool XTeam::appendMember( const Member &member, xgc_bool bLeader, bool ntfNearbyPlayers )
	{
		auto it = mMembers.begin();
		while( it != mMembers.end() )
		{
			if( it->hGlobal == INVALID_GLOBAL_ID )
				break;

			if( it->hGlobal == member.hGlobal )
			{
				if( bLeader )
				{
					mLeaderGlobal = member.hGlobal;
					mLeaderObject = member.hMember;
				}

				return true;
			}

			++it;
		}

		/// 没有空位，且已锁定队伍人数，则返回
		if( it == mMembers.end() )
		{
			if( mSpinLockCount )
				return false;
			
			mMembers.push_back( member );
		}
		else
		{
			it->hGlobal = member.hGlobal;
			it->hMember = member.hMember;
			it->vRelatively = member.vRelatively;
		}

		if( bLeader )
		{
			mLeaderGlobal = member.hGlobal;
			mLeaderObject = member.hMember;
		}

		XCharactor* pActor = ObjectCast< XCharactor >( member.hMember );
		if( pActor )
		{
			pActor->setTeam( shared_from_this() );
		}

		if( ntfNearbyPlayers )
		{
			updateToNearby( member.hGlobal, getLeaderGlobalID() );
		}
		return true;
	}

	xgc_void XTeam::removeMember( xGlobal hGlobal/*, xgc_bool bChangeLeader *//*= true*/ )
	{
		XGC_ASSERT_RETURN( hGlobal != INVALID_GLOBAL_ID, void(0) );

		// 防止提前删除自身导致的野指针问题。
		XTeamPtr thisTeam = shared_from_this();

		// 删除成员
		auto it = std::find( mMembers.begin(), mMembers.end(), hGlobal );
		if( it != mMembers.end() )
		{
			XCharactor* pActor = ObjectCast< XCharactor >( it->hMember );
			if( pActor )
			{
				pActor->mTeamPtr.reset();
			}

			it->hGlobal = INVALID_GLOBAL_ID;
			it->hMember = INVALID_OBJECT_ID;
			it->vRelatively = XVector2::ZERO;
		}

		updateToNearby( hGlobal, 0 );
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

		XCharactor* pLeader = ObjectCast< XCharactor >( getLeaderObjectID() );
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

	xgc_bool XTeam::setLeader( xGlobal hGlobal, xObject hObject )
	{
		mLeaderGlobal = hGlobal;
		mLeaderObject = hObject;
		return false;
	}

	xgc_void XTeam::resetLeader()
	{
		mLeaderGlobal = INVALID_GLOBAL_ID;
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
		for( auto it : mMembers )
		{
			XCharactor* pActor = ObjectCast<XCharactor>( it.hMember );
			if( !pActor )
				continue;

			pActor->setTeam( xgc_nullptr );

			updateToNearby( it.hGlobal, 0 );
		}

		setLeader(INVALID_GLOBAL_ID, INVALID_OBJECT_ID);

		return true;
	}

	xgc_bool XTeam::changeLeader( xGlobal hLeader )
	{
		xGlobal hOld = getLeaderGlobalID();
		
		auto newMember = getMemberByID( hLeader );
		if( !newMember )
			return false;

		setLeader( hLeader, newMember->hMember );
		updateToNearby( hLeader, hLeader );

		auto oldMember = getMemberByID( hOld );
		if( oldMember && oldMember->hMember != INVALID_OBJECT_ID )
		{
			updateToNearby( hOld, hLeader );
		}


		return true;
	}

	xgc_bool XTeam::memberOnline( xGlobal hGlobal )
	{
		auto pMember = getMemberByID( hGlobal );
		if( !pMember )
			return false;

		pMember->isOnline = true;
		
		return true;
	}

	xgc_bool XTeam::memberOffline( xgc_uint64 hGlobal )
	{
		auto pMember = getMemberByID( hGlobal );
		if( !pMember )
			return false;

		pMember->isOnline = false;
		return true;
	}
}
