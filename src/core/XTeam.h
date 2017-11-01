#pragma once
#ifndef _XTEAM_H_
#define _XTEAM_H_

class CRole;

namespace xgc
{
	class CORE_API XTeam : public std::enable_shared_from_this< XTeam >
	{
	public:
		struct Member
		{
			xGlobal			hGlobal;		// 全局ID
			xObject			hMember;		// 临时ID
			XVector2		vRelatively;	// 坐标
			xgc_bool        isOnline;       // 是否在线

			Member() 
				: hGlobal( INVALID_GLOBAL_ID )
				, hMember( INVALID_OBJECT_ID )
				, isOnline( false ) 
			{
			}

			Member( xGlobal hGlobal_, xObject hMember_, XVector2 vRelatively_, xgc_bool isOnline_ ) 
				: hGlobal( hGlobal_ )
				, hMember( hMember_ )
				, vRelatively( vRelatively_ )
				, isOnline( isOnline_ )
			{
			}

			bool operator==( const XTeam::Member &rhs )const
			{
				return hGlobal == rhs.hGlobal;
			}

			bool operator==( xGlobal _hGlobal )const
			{
				return hGlobal == _hGlobal;
			}
		};

		typedef xgc_void (*TeamDataDestructor)( xgc_uintptr );

	protected:
		XTeam( xgc_uint32 teamID, xgc_size nMemberCount, xgc_bool bSpinLockCount );

		virtual ~XTeam(void);

		xgc_void Release()
		{ 
			delete this; 
		}


	public:
		///
		/// 获取队伍ID
		/// [6/18/2014] create by guqiwei.weir
		///
		virtual xgc_uint32 getTeamID()const 
		{ 
			return mTeamID; 
		}

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// add member to team
		//----------------------------------------//
		virtual xgc_bool appendMember( const Member &member, xgc_bool bLeader = false, bool ntfNearbyPlayers = true );

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// remove member from team
		//----------------------------------------//
		virtual xgc_void removeMember( xGlobal hGlobal/*, xgc_bool bChangeLeader = true*/ );

		//----------------------------------------//
		//
		// [11/23/2012 Albert.xu]
		// set userdata
		//----------------------------------------//
		xgc_void SetUserdata( xgc_uintptr pUserdata, TeamDataDestructor Fn )
		{
			mUserdata = pUserdata;
			mDestructor = Fn;
		}

		//----------------------------------------//
		//
		// [11/13/2012 Albert.xu]
		// set leader global id
		//----------------------------------------//
		virtual xgc_bool setLeader( xGlobal hGlobal, xObject hObj );

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// get leader global id
		//----------------------------------------//
		virtual xGlobal getLeaderGlobalID()const { return mLeaderGlobal; }

		//----------------------------------------//
		//
		// [11/22/2012 Albert.xu]
		// get leader object id
		//----------------------------------------//
		virtual xObject getLeaderObjectID()const { return mLeaderObject; }

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// get members count include leader
		//----------------------------------------//
		xgc_size getMemberCount()const
		{
			xgc_size nCount = 0;
			for( auto it : mMembers )
			{
				if( it.hMember != INVALID_OBJECT_ID )
					nCount++;
			}
			return nCount;
		}

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// get member obejct id by slot
		//----------------------------------------//
		xgc_bool getMember( xgc_size nSlot, Member& member )const
		{ 
			return nSlot < mMembers.size()?(member = mMembers[nSlot], true):false; 
		}

		///
		///  通过ID 获得成员信息
		/// [8/29/2014] create by guqiwei.weir
		///
		Member* getMemberByID( xGlobal hXcbID ) const
		{
			Member MatchValue = { hXcbID, 0, XVector2::ZERO, false };

			auto it = std::find( mMembers.begin(), mMembers.end(), MatchValue );
			return ( it == mMembers.end() ) ? xgc_nullptr : const_cast<Member *>( &( *it ) );
		}

		//----------------------------------------//
		//
		// [11/22/2012 Albert.xu]
		// get member relatively position
		//----------------------------------------//
		XVector2 getTeamSlotPosition( xgc_uint32 nSlot )const;

		//----------------------------------------//
		//
		// [11/13/2012 Albert.xu]
		// judge this id is team memeber
		//----------------------------------------//
		xgc_bool isMember( xGlobal hGlobal )const
		{
			Member MatchValue = { hGlobal, 0, XVector2::ZERO, false };

			return std::find( mMembers.begin(), mMembers.end(), MatchValue ) != mMembers.end(); 
		}

		//----------------------------------------//
		//
		// [11/23/2012 Albert.xu]
		// get team userdata
		//----------------------------------------//
		xgc_uintptr getUserdata()const
		{ 
			return mUserdata; 
		}

		///
		///  解散队伍
		/// [8/29/2014] create by guqiwei.weir
		///
		virtual xgc_bool dismissed();

		///
		///  队长变更
		/// @param pMsSession Mserver连接会话
		/// @return 队员被踢出返回true
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		xgc_bool changeLeader( xGlobal hLeader );

		///
		/// 队员上线
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		xgc_bool memberOnline( xGlobal hGlobal );

		///
		/// 队员离线
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		xgc_bool memberOffline( xGlobal hGlobal );

	protected:
		xgc_void setTeamID( xgc_uint32 teamID ) 
		{ 
			mTeamID = teamID; 
		}

		xgc_void clearMembers();

		///
		/// 向队员附近玩家发送队伍跟新信息 
		/// [6/21/2014] create by guqiwei.weir
		///
		virtual xgc_void updateToNearby( xGlobal hNew, xGlobal hOld ) = 0;

    private:
		///
		///  队伍解散
		/// @return 队员被踢出返回true
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		virtual xgc_void resetLeader();

	protected:
		typedef xgc_vector< Member > MemberVec;
		MemberVec mMembers; // members

	private:
		/// 队伍ID
		xgc_uint32 mTeamID;

		/// 领队的全局ID
		xGlobal mLeaderGlobal;
		/// 领队的对象ID
		xObject mLeaderObject;

		/// 锁定队伍最大人数
		xgc_bool mSpinLockCount;
		/// 队伍自带的数据
		xgc_uintptr mUserdata;
		/// 队伍数据清理回调
		TeamDataDestructor mDestructor;
	};

	typedef std::shared_ptr< XTeam > XTeamPtr;
}
#endif // _XTEAM_H_