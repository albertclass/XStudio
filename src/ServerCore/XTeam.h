#pragma once
#ifndef _XTEAM_H_
#define _XTEAM_H_

#ifdef CORE_EXPORTS
	#define CORE_API __declspec(dllexport)
#else
	#define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS

#if (_MSC_VER >= 1500) && (_MSC_FULL_VER >= 150030729) && !defined( _USE_BOOST )
	#include <memory>
	using namespace std;
	#if (_MSC_VER >= 1600)
		#pragma message("XTeam using std::shared_ptr")
	#else
		#pragma message("XTeam using std::tr1::shared_ptr")
		using namespace std::tr1;
	#endif
#else
	#pragma message("XTeam using boost::shared_ptr")
	#include "boost/shared_ptr.hpp"
	#include "boost/enable_shared_from_this.hpp"
	using namespace boost;
#endif

class CRole;

namespace XGC
{
	class XTeam;
	typedef shared_ptr< XTeam > XTeamPtr;

	class CORE_API XTeam : public enable_shared_from_this< XTeam >
	{
	public:
		struct Member
		{
			xGlobal			hGlobal;		// ȫ��ID
			xObject			hMember;		// ��ʱID
			XVector2		vRelatively;	// ����
			xgc_bool        isOnline;       // �Ƿ�����

			Member() : hGlobal( INVALID_GLOBAL_ID ), hMember( INVALID_OBJECT_ID ), isOnline( false ) {}

			Member( xGlobal hGlobal_, xObject hMember_, XVector2 vRelatively_, xgc_bool isOnline_ ) :
				hGlobal( hGlobal_ ), hMember( hMember_ ), vRelatively( vRelatively_ ), isOnline( isOnline_ )
			{
			}
		};

		typedef xgc_void (*TeamDataDestructor)( xgc_uintptr );

	protected:
		explicit XTeam( xgc_uint32 teamID, xgc_size nMemberCount, xgc_bool bSpinLockCount );
		virtual ~XTeam(void);

		xgc_void Release(){ delete this; }


	public:
		friend bool operator==( const XTeam::Member& lhs, const XTeam::Member &rhs )
		{
			return lhs.hGlobal == rhs.hGlobal;
		}

		///
		/// ��ȡ����ID
		/// [6/18/2014] create by guqiwei.weir
		///
		virtual xgc_uint32 getTeamID()const { return mTeamID; }

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// add member to team
		//----------------------------------------//
		virtual xgc_bool appendMember( xGlobal hGlobal, xObject hMember, xgc_bool isOnline,
			xgc_bool bLeader = false, const XVector2& vRelatively = XVector2::ZERO,
			bool ntfNearbyPlayers = true );

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
		virtual xgc_bool setHeader( xGlobal hXcbID, xObject hObj );

		//----------------------------------------//
		//
		// [11/12/2012 Albert.xu]
		// get leader global id
		//----------------------------------------//
		virtual xGlobal getHeaderID()const { return mLeader; }

		//----------------------------------------//
		//
		// [11/22/2012 Albert.xu]
		// get leader object id
		//----------------------------------------//
		virtual xObject getHeaderObject()const { return mLeaderObject; }

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
		///  ͨ��ID ��ó�Ա��Ϣ
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
		///  ��ɢ����
		/// [8/29/2014] create by guqiwei.weir
		///
		virtual xgc_bool dismissed();

		///
		///  �ӳ����
		/// @param pMsSession Mserver���ӻỰ
		/// @return ��Ա���߳�����true
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		xgc_bool changeHeader( xGlobal headerXCBID );

		///
		/// ��Ա����
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		// xgc_bool memberOnline( xGlobal xcbID );

		///
		/// ��Ա����
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		// virtual xgc_bool memberOffline( xGlobal xcbID );

		///
		/// ֪ͨ�����λ����Ϣ 
		/// @param pMsSession Mserver���ӻỰ
		/// @param pRole �����Ϣ
		/// [6/19/2014] create by guqiwei.weir
		///
		virtual xgc_void notifyPosChangeToOthers( CRole* pRole ) = 0;

		///
		/// ֪ͨ�������������ɫ״̬ 
		/// @param pMsSession Mserver���ӻỰ
		/// @param pRole �����Ϣ
		/// [6/19/2014] create by guqiwei.weir
		///
		virtual xgc_void notifyStatusToOthers( CRole* pRole, xObject cancelSteate ) = 0;

		///
		/// �����Ҹ����Ķ�Ա 
		/// [8/8/2014] create by guqiwei.weir
		///
		virtual xgc_void getNearbyMembers( xObject hRole, xObjectSet& members ) = 0;

	protected:
		xgc_void setTeamID( xgc_uint32 teamID ) { mTeamID = teamID; }

		xgc_void clearMembers();

		///
		/// ���Ա������ҷ��Ͷ��������Ϣ 
		/// [6/21/2014] create by guqiwei.weir
		///
		virtual xgc_void broadcastHeaderIDToNearbyers( xgc_uint64 xcbID, xgc_uint64 headerID ) = 0;

    private:
		///
		///  �����ɢ
		/// @return ��Ա���߳�����true
		/// @exception None
		/// [6/19/2014] create by guqiwei.weir
		///
		virtual xgc_void resetLeader();

	protected:
		typedef xgc_vector< Member > MemberVec;
		MemberVec mMembers; // members

	private:
		xgc_uint32 mTeamID;

		xGlobal mLeader;		// leader global id
		xObject mLeaderObject;	// leader object id

		xgc_bool mSpinLockCount;
		xgc_uintptr mUserdata;
		TeamDataDestructor mDestructor;
	};
}
#endif // _XTEAM_H_