#include "StdAfx.h"
#include "XCharactor.h"
#include "XSkillBase.h"
#include "xsystem.h"

namespace XGC
{
	//////////////////////////////////////////////////////////////////////////
	// 角色属性索引
	CORE_API xAttrIndex	attrActorIndex;			///< 角色配置索引
	CORE_API xAttrIndex	attrActorType;			///< 角色类型
	CORE_API xAttrIndex attrHP;					///< 当前生命值               
	CORE_API xAttrIndex attrMaxHP;				///< 生命上限    
	CORE_API xAttrIndex	attrActorBeatSpeed;		///< 击退速度
	CORE_API xAttrIndex	attrActorFaintTime;		///< 眩晕时间
	CORE_API xAttrIndex	attrActorAbnormalTime;	///< 特殊状态时间
	CORE_API xAttrIndex	attrActorBornTime;		///< 出生时间
	CORE_API xAttrIndex	attrActorGroupMask;		///< 组别掩码，用于区分阵营
	CORE_API xAttrIndex attrActorStatus;        ///< 角色状态

	BEGIN_IMPLEMENT_XCLASS( XCharactor, XGameObject, TypeXCharactor )
		IMPLEMENT_ATTRIBUTE( ActorIndex, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// 角色配置索引
		IMPLEMENT_ATTRIBUTE( ActorType, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// 角色类型
		IMPLEMENT_ATTRIBUTE( ActorBeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 击退速度
		IMPLEMENT_ATTRIBUTE( ActorFaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 眩晕时间
		IMPLEMENT_ATTRIBUTE( ActorAbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 特殊状态时间
		IMPLEMENT_ATTRIBUTE( ActorBornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )  // 出生时间
		IMPLEMENT_ATTRIBUTE( ActorGroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// 组别掩码，用于区分阵营
		IMPLEMENT_ATTRIBUTE( ActorStatus, VT_BYTE, ATTR_FLAG_SAVE, "20150122" )
		END_IMPLEMENT_XCLASS();


	static XVector3 Direction[] =
	{
		XVector3( 1.0f, 0.0f, 0.0f ),
		XVector3( 0.7f, 0.7f, 0.0f ),
		XVector3( 0.0f, 1.0f, 0.0f ),
		XVector3( -0.7f, 0.7f, 0.0f ),

		XVector3( -1.0f, 0.0f, 0.0f ),
		XVector3( -0.7f, -0.7f, 0.0f ),
		XVector3( 0.0f, -1.0f, 0.0f ),
		XVector3( 0.7f, -0.7f, 0.0f ),
	};

	xObjectSet XCharactor::static_error_res;  ///< 错误的

	XCharactor::XCharactor( void )
		: XGameObject()
		, mEyeshotGroup( -1 )
		, mResetStatusTimerHandler( INVALID_TIMER_HANDLE )
		, mActorRestonStatus( ActorStatus_Live )
		, mAttacker( INVALID_OBJECT_ID )
		, mFirstAttackMeTarget( INVALID_OBJECT_ID )
		, mMaxHateTarget( INVALID_OBJECT_ID )
		, mMinHateTarget( INVALID_OBJECT_ID )
		, mMaxHateValue( 0 )
		, mMinHateValue( 0 )
		, mRadius( 1.0f )
		, mBornPoint( XVector3::ZERO )
		, mTarget( INVALID_OBJECT_ID )
		, mCurrentSkill( INVALID_OBJECT_ID )
		, mCurrentSkillTarget( INVALID_OBJECT_ID )
		, mCurrentSkillPoint( XVector3::ZERO )
		, mIsIgnoreSkillCD( false )
		, mFightState( false )
	{
		mStatusFlags = { 0 }; // 设置状态标志位
	}

	XCharactor::~XCharactor( void )
	{
		getTimer().remove_event( mResetStatusTimerHandler );
	}

	//---------------------------------------------------//
	// [11/19/2010 Albert]
	// Description:	 
	//---------------------------------------------------//
	xgc_void XCharactor::UnderAttack( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext )
	{
		FUNCTION_BEGIN;
		// 不要鞭尸，太变态了
		if( GetActorStatus() == ActorStatus_Dead )
			return;

		xgc_long nDamage = 0;
		xgc_long nHatred = 0;

		// 是否命中，命中后返回伤害和仇恨
		if( OnHit( hAttacker, eMode, lpContext, nDamage, nHatred ) )
		{
			if( hAttacker != INVALID_OBJECT_ID )
			{
				mAttacker = hAttacker;
			}
		}
        //OnUnderAttacked 有可能递归调用死亡，会多次死亡
        //之前没死，之后死了
        ActorStatus eBefore = GetActorStatus();
        XCharactor *pAttack = ObjectCast<XCharactor>(hAttacker);
        if (pAttack)
            pAttack->OnAttacked(GetObjectID(), eMode, lpContext, nDamage);
        OnUnderAttacked(hAttacker, eMode, lpContext, nDamage);

        //之前死了才做死亡逻辑，没死则忽略
  		if( eBefore == ActorStatus_Dead && ActorStatus_Dead == GetActorStatus() )
		{
			Dead( hAttacker, eMode, lpContext );
		}
		FUNCTION_END;
	}

	/////
	/// [8/21/2009 Albert]
	/// 角色死亡
	/////
	xgc_void XCharactor::Dead( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext )
	{
		// 处理角色场景死亡事件
		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			IActorMapEventHandler* pHandler = pMap->GetActorEventHandler();
			if( pHandler )
			{
				pHandler->OnActorDead( this, hAttacker, eMode, lpContext );
			}
		}

		OnDead( hAttacker, eMode, lpContext );
	}

	///
	/// 角色重生
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XCharactor::Relive( xgc_lpvoid lpContext )
	{
		// 处理场景重生事件
		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			IActorMapEventHandler* pHandler = pMap->GetActorEventHandler();
			if( pHandler )
			{
				pHandler->OnActorRelive( this, lpContext );
			}
		}

		SetActorStatus( ActorStatus_Live );
		OnRelive( lpContext );
	}

	/// ------------------------------------------------ ///
	/// 设置生物状态
	/// [5/31/2014 jianglei.kinly]
	/// ------------------------------------------------ ///
	xgc_void XCharactor::SetActorStatus( ActorStatus eStatus )
	{
		if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( mResetStatusTimerHandler );
			mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
		}

		SetAttrValue<xgc_byte>( attrActorStatus, eStatus );
		mActorRestonStatus = eStatus;
	}

	//---------------------------------------------------//
	// [8/24/2009 Albert]
	// 设置角色状态
	//---------------------------------------------------//
	xgc_void XCharactor::SetActorStatus( ActorStatus eStatus, xgc_real32 fTime, xgc_int32 nMode )
	{
		if( GetActorStatus() == ActorStatus_Dead )
			return;

		if( fTime != 0.0f )
		{
			xgc_real64 fRelease = 0;
			if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
			{
				fRelease = getTimer().remove_event( mResetStatusTimerHandler );
				mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
			}

			if( eStatus == mActorRestonStatus )
			{
				switch( nMode )
				{
					case 0: // 覆盖
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) fTime );
					break;
					case 1: // 顺延
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) ( fTime + fRelease ) );
					break;
					case 2: // 取最小
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) XGC_MIN( fTime, fRelease ) );
					break;
					case 3: // 取最大
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) XGC_MAX( fTime, fRelease ) );
					break;
				}
			}
			else
			{
				mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) fTime );
				mActorRestonStatus = GetActorStatus();
				SetAttrValue<xgc_byte>( attrActorStatus, eStatus );
			}
		}
		else
		{
			if( eStatus != ActorStatus_Live )
				SetEnjoinMove();
			else
				AntiSetEnjoinMove();

			SetAttrValue<xgc_byte>( attrActorStatus, eStatus );
		}
	}

	//---------------------------------------------------//
	// [12/23/2010 Albert]
	// Description:	重置角色状态 
	//---------------------------------------------------//
	xgc_void XCharactor::ResetActorStatus( timer_t handle, ActorStatus eStatus )
	{
		if( GetActorStatus() != ActorStatus_Dead )
			SetAttrValue<xgc_byte>( attrActorStatus, eStatus );

		if( eStatus != ActorStatus_Live )
			SetEnjoinMove();
		else
			AntiSetEnjoinMove();

		mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
	}

	//---------------------------------------------------//
	// [11/19/2010 Albert]
	// Description:	当加入的对象为关注的类型，则进行管理。
	// 添加技能Child
	//---------------------------------------------------//
	xgc_void XCharactor::OnAddChild( XObject* pChild, xgc_lpcvoid lpContext )
	{
		if( pChild->IsInheritFrom( TypeXSkillBase ) )
		{
			mSkillMap.insert( std::make_pair( pChild->GetUnsignedAttr( attrSkillMainId ), pChild->GetObjectID() ) );
		}
		else if( pChild->IsInheritFrom( TypeXStateBase ) )
		{
			auto key = std::make_pair( pChild->GetUnsignedAttr( attrStateType ), pChild->GetUnsignedAttr( attrStateBuffID ) );
			mStateMap[key].insert( pChild->GetObjectID() );
		}

		__super::OnAddChild( pChild, lpContext );
	}

	//---------------------------------------------------//
	// [11/19/2010 Albert]
	// Description:	当移除的对象为关注对象，则从列表中取出 
	//---------------------------------------------------//
	void XCharactor::OnRemoveChild( XObject* pChild, bool bRelease )
	{
		if( pChild->IsInheritFrom( TypeXSkillBase ) )
		{
			auto it = mSkillMap.find( pChild->GetUnsignedAttr( attrSkillMainId ) );
			if( it != mSkillMap.end() )
			{
				mSkillMap.erase( it );
			}
			else
				XGC_ASSERT_MESSAGE( false, "想删掉一个不存在该对象身上的技能：%u", pChild->GetUnsignedAttr( attrSkillMainId ) );
		}
		else if( pChild->IsInheritFrom( TypeXStateBase ) )
		{
			auto key = std::make_pair( pChild->GetUnsignedAttr( attrStateType ), pChild->GetUnsignedAttr( attrStateBuffID ) );
			auto it = mStateMap.find( key );
			if( it != mStateMap.end() )
			{
				it->second.erase( pChild->GetObjectID() );
				// 如果这个的size == 0 清除这个标记
				if( it->second.empty() )
				{
					mStateMap.erase( it );
				}
			}
			else
				XGC_ASSERT_MESSAGE( false, "想删掉一个不存在该对象身上的状态：%u, %u", pChild->GetUnsignedAttr( attrStateType ), pChild->GetUnsignedAttr( attrStateBuffID ) );
		}

		__super::OnRemoveChild( pChild, bRelease );
	}

	xgc_void XCharactor::UpdateHateMap( xObject hAttacker, xgc_int32 nHateValue )
	{
		HateMap::iterator iter = mHateMap.find( hAttacker );
		xgc_int32 nTmp = nHateValue;
		HateInfo stHateInfo = { 0 };
		stHateInfo.tUpdateTime = current_time();
		if( iter != mHateMap.end() )
		{
			stHateInfo.iHateValue = XGC_MAX( 1, iter->second.iHateValue + nHateValue );
			iter->second = stHateInfo;
			nTmp = iter->second.iHateValue;
		}
		else
		{
			stHateInfo.iHateValue = XGC_MAX( 1, nHateValue );
			mHateMap.insert( HateMap::value_type( hAttacker, stHateInfo ) );
		}

		//插入第一个人，要同时设置最大最小目标
		if( 0 == mMaxHateValue || nTmp >= mMaxHateValue )
		{
			mMaxHateTarget = hAttacker;
			mMaxHateValue = nTmp;
		}
		//插入第一个人，要同时设置最大最小目标
		if( 0 == mMinHateValue || nTmp <= mMinHateValue )
		{
			mMinHateTarget = hAttacker;
			mMinHateValue = nTmp;
		}

		mAttacker = hAttacker;
	}

	xgc_void XCharactor::CopyHateMap( xObject xCopyTarget )
	{
		XCharactor* pCharacter = ObjectCast<XCharactor>( xCopyTarget );
		XGC_ASSERT_RETURN( pCharacter, xgc_void( 0 ) );
		const HateMap& hateMap = pCharacter->GetHateMap();
		for( auto& item : hateMap )
		{
			mHateMap.insert( HateMap::value_type( xCopyTarget, item.second ) );
		}
		if( mMaxHateValue < pCharacter->GetMaxHateValue() )
		{
			mMaxHateValue = pCharacter->GetMaxHateValue();
			mMaxHateTarget = pCharacter->GetMaxHateTarget();
		}
		if( mMinHateValue == 0 || mMinHateValue > pCharacter->GetMinHateValue() )
		{
			mMinHateValue = pCharacter->GetMinHateValue();
			mMinHateTarget = pCharacter->GetMinHateTarget();
		}
		mAttacker = pCharacter->GetAttacker();
	}

	xgc_void XCharactor::CheckOwnerChange( xgc_uint32 iTimeoutLimit )
	{
		if( mFirstAttackMeTarget == INVALID_OBJECT_ID )
		{
			return;
		}

		xgc_time64 iCurrentTime = current_time();
		const HateMap& IdToHateInfo = GetHateMap();
		auto item = IdToHateInfo.find( mFirstAttackMeTarget );
		if( item != IdToHateInfo.end() )
		{
			if( iCurrentTime - item->second.tUpdateTime >= iTimeoutLimit )
			{
				//获取除当前所有者之外的仇恨值最大者作为所有者
				if( mFirstAttackMeTarget != mMaxHateTarget )
				{
					//所有权归仇恨值最大者
					mFirstAttackMeTarget = mMaxHateTarget;
				}
				else if( mFirstAttackMeTarget == mMaxHateTarget && mFirstAttackMeTarget == mMinHateTarget )
				{
					//只有一个人，所有权不变更
					return;
				}
				else
				{
					//是所有者，并且是仇恨值最大，所有权归除自己之外的仇恨值最大者
					xgc_int32 iSecondMaxHateValue = 0;
					xObject oSecondMaxHateObj = INVALID_OBJECT_ID;
					for( auto& item : mHateMap )
					{
						if( item.second.iHateValue > iSecondMaxHateValue && item.first != mMaxHateTarget )
						{
							iSecondMaxHateValue = item.second.iHateValue;
							oSecondMaxHateObj = item.first;
						}
					}
					if( oSecondMaxHateObj != INVALID_OBJECT_ID )
					{
						mFirstAttackMeTarget = oSecondMaxHateObj;
					}
					return;
				}
			}
		}
	}

	xgc_void XCharactor::CheckSpecialHate( xObject hAttacker )
	{
		//删除最大最小仇恨目标时会重新遍历列表，找最大最小目标
		if( mHateMap.empty() )
		{
			return;
		}
		if( hAttacker == mMaxHateTarget )
		{
			mMaxHateValue = mHateMap.begin()->second.iHateValue;
			mMaxHateTarget = mHateMap.begin()->first;
			for( auto iter = mHateMap.begin(); iter != mHateMap.end(); ++iter )
			{
				if( iter->second.iHateValue > mMaxHateValue )
				{
					mMaxHateTarget = iter->first;
					mMaxHateValue = iter->second.iHateValue;
				}
			}
		}
		if( hAttacker == mMinHateTarget )
		{
			mMinHateValue = mHateMap.begin()->second.iHateValue;
			mMinHateTarget = mHateMap.begin()->first;
			for( auto iter = mHateMap.begin(); iter != mHateMap.end(); ++iter )
			{
				if( iter->second.iHateValue < mMinHateValue )
				{
					mMinHateTarget = iter->first;
					mMinHateValue = iter->second.iHateValue;
				}
			}
		}
		//如果删除的是第一个攻击我的目标，将仇恨值最大的设置为首击者
		if( hAttacker == mFirstAttackMeTarget )
		{
			mFirstAttackMeTarget = mMaxHateTarget;
		}
		if( hAttacker == mTarget )
		{
			mTarget = mMaxHateTarget;
		}
	}

	xgc_bool XCharactor::DeleteFromHateMap( xObject hAttacker )
	{
		HateMap::iterator iter = mHateMap.find( hAttacker );
		if( iter != mHateMap.end() )
		{
			mHateMap.erase( iter );
			if( mHateMap.empty() )
			{
				mMaxHateTarget = INVALID_OBJECT_ID;
				mMinHateTarget = INVALID_OBJECT_ID;
				mMaxHateValue = 0;
				mMinHateValue = 0;
			}
			else
			{
				CheckSpecialHate( hAttacker );
			}
			return true;
		}
		return false;
	}

	xgc_int32 XCharactor::GetHateValue( xObject hAttacker )
	{
		HateMap::iterator iter = mHateMap.find( hAttacker );
		if( iter != mHateMap.end() )
		{
			return iter->second.iHateValue;
		}
		return 0;
	}

	xObject XCharactor::GetRandomTarget()
	{
		if( mHateMap.empty() )
			return INVALID_OBJECT_ID;
		xgc_vector<xObject> vecRandom;
		for( auto iter : mHateMap )
		{
			vecRandom.push_back( iter.first );
		}
		xgc_size nRandom = RandomRange( 0, vecRandom.size() );
		return vecRandom[nRandom];
	}

	xObject XCharactor::GetNextAsTarget( xObject xTarget )
	{
		if( mHateMap.empty() )
			return INVALID_OBJECT_ID;

		HateMap::iterator iter = mHateMap.find( xTarget );
		if( iter != mHateMap.end() && ( ++iter != mHateMap.end() ) )
		{
			return iter->first;
		}
		return mHateMap.begin()->first;
	}

	xgc_void XCharactor::ClearHateMap()
	{
		mHateMap.clear();
		mFirstAttackMeTarget = INVALID_OBJECT_ID;
		mMaxHateTarget = INVALID_OBJECT_ID;
		mMinHateTarget = INVALID_OBJECT_ID;
		mMaxHateValue = 0;
		mMinHateValue = 0;
	}
}
