#include "XHeader.h"
#include "XCharactor.h"

namespace xgc
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
	CORE_API xAttrIndex attrActorState;         ///< 角色状态
	CORE_API xAttrIndex attrActorAbility;       ///< 角色能力

	IMPLEMENT_XCLASS_BEGIN( XCharactor, XObjectNode )
		IMPLEMENT_ATTRIBUTE( ActorIndex, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// 角色配置索引
		IMPLEMENT_ATTRIBUTE( ActorType, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// 角色类型
		IMPLEMENT_ATTRIBUTE( ActorBeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 击退速度
		IMPLEMENT_ATTRIBUTE( ActorFaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 眩晕时间
		IMPLEMENT_ATTRIBUTE( ActorAbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 特殊状态时间
		IMPLEMENT_ATTRIBUTE( ActorBornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )  // 出生时间
		IMPLEMENT_ATTRIBUTE( ActorGroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// 组别掩码，用于区分阵营
		IMPLEMENT_ATTRIBUTE( ActorState, VT_U32, ATTR_FLAG_SAVE, "20150122" )
		IMPLEMENT_ATTRIBUTE_ARRAY( ActorAbility, VT_U32, abl_actor_count, ATTR_FLAG_SAVE, "20150122" )
	IMPLEMENT_XCLASS_END();

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

	XCharactor::XCharactor()
		: mEyeshotGroup( -1 )
		, mResetStatusTimerHandler( INVALID_TIMER_HANDLE )
		, mActorRestonState( enActorState::sta_live )
		, mAttacker( INVALID_OBJECT_ID )
		, mFirstAttackMeTarget( INVALID_OBJECT_ID )
		, mMaxHateTarget( INVALID_OBJECT_ID )
		, mMinHateTarget( INVALID_OBJECT_ID )
		, mMaxHateValue( 0 )
		, mMinHateValue( 0 )
		, mRadius( 1.0f )
		, mBornPoint( XVector3::ZERO )
		, mTarget( INVALID_OBJECT_ID )
		, mFightState( false )
	{
		mStatusFlags = { 0 }; // 设置状态标志位
	}

	XCharactor::~XCharactor( void )
	{
		getTimer().remove( mResetStatusTimerHandler );
	}

	///
	/// \brief 收到攻击
	/// \author albert.xu
	/// \date 11/19/2010
	///
	xgc_void XCharactor::UnderAttack( xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext )
	{
		FUNCTION_BEGIN;
		// 不要鞭尸，太变态了
		if( isState( enActorState::sta_dead ) )
			return;

		xgc_long nDamage = 0;
		xgc_long nHatred = 0;

		// 发送受击事件
		XCharactorEvent evt;
		evt.hAttacker = hAttacker;
		evt.lpContext = lpContext;
		evt.attack.nDamage = nDamage;
		evt.attack.eMode = eMode;

		EmmitEvent( evt_actor_behit, evt.cast );

		// 是否命中，命中后返回伤害和仇恨
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked 有可能递归调用死亡，会多次死亡
			// 之前没死，之后死了
			enActorState eBefore = getStatus();
			XCharactor *pAttack = ObjectCast<XCharactor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt_actor_attacked, evt.cast );
			}

			//之前死了才做死亡逻辑，没死则忽略
			if( enActorState::sta_dead == eBefore &&
				enActorState::sta_dead == getStatus() )
			{
				Dead( hAttacker, eMode, lpContext );
			}

		}
		FUNCTION_END;
	}

	/////
	/// [8/21/2009 Albert]
	/// 角色死亡
	/////
	xgc_void XCharactor::Dead( xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext )
	{
		// 处理角色场景死亡事件
		SetState( enActorState::sta_dead );
		SetEnjoinAttack();
		SetEnjoinMove();
		SetEnjoinUnderAttack();

		// 发送角色死亡事件
		XCharactorEvent evt;
		evt.lpContext = lpContext;
		evt.hAttacker = hAttacker;

		evt.dead.eMode = eMode;
		EmmitEvent( evt_actor_dead, evt.cast );
	}

	///
	/// 角色重生
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XCharactor::Relive( xgc_lpvoid lpContext )
	{
		// 处理场景重生事件
		SetState( enActorState::sta_live );

		// 发送角色重生事件
		XCharactorEvent evt;
		evt.lpContext = lpContext;

		EmmitEvent( evt_actor_relive, evt.cast );
	}

	/// ------------------------------------------------ ///
	/// 设置生物状态
	/// [5/31/2014 jianglei.kinly]
	/// ------------------------------------------------ ///
	xgc_void XCharactor::SetState( enActorState eStatus )
	{
		if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mResetStatusTimerHandler );
			mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
		}

		setValue( attrActorState, (xgc_uint32)eStatus );
		mActorRestonState = eStatus;
	}

	//---------------------------------------------------//
	// [8/24/2009 Albert]
	// 设置角色状态
	//---------------------------------------------------//
	xgc_void XCharactor::SetState( enActorState eStatus, timespan tsDuration, xgc_int32 nMode )
	{
		if( isState( enActorState::sta_dead ) )
			return;

		if( tsDuration != 0 )
		{
			timespan duration = tsDuration;
			timespan remain;
			if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
			{
				remain = getTimer().remove( mResetStatusTimerHandler );
				mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
			}

			if( eStatus == mActorRestonState )
			{
				switch( nMode )
				{
					case 0: // 覆盖
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + duration,
						0,
						"once" );
					break;
					case 1: // 顺延
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + duration + remain,
						timespan( 0 ),
						"once" );
					break;
					case 2: // 取最小
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + XGC_MIN( duration, remain ),
						0,
						"once" );
					break;
					case 3: // 取最大
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + XGC_MAX( duration, remain ),
						0,
						"once" );
					break;
				}
			}
			else
			{
				mResetStatusTimerHandler = getTimer().insert( 
					std::bind( &XCharactor::ResetActorState, this, eStatus ), 
					datetime::now() + duration,
					timespan( 0 ),
					"once" );
				mActorRestonState = getStatus();
				SetState( eStatus );
			}
		}
		else
		{
			if( eStatus != enActorState::sta_live )
				SetEnjoinMove();
			else
				AntiSetEnjoinMove();

			SetState( eStatus );
		}
	}

	//---------------------------------------------------//
	// [12/23/2010 Albert]
	// Description:	重置角色状态 
	//---------------------------------------------------//
	xgc_void XCharactor::ResetActorState( enActorState eStatus )
	{
		if( !isState( enActorState::sta_dead ) )
			SetState( eStatus );

		if( !isState( enActorState::sta_live ) )
			SetEnjoinMove();
		else
			AntiSetEnjoinMove();

		mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
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
			mHateMap.insert( { hAttacker, stHateInfo } );
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

	xgc_void XCharactor::CopyHateMap( xObject hTarget )
	{
		XCharactor* pTarget = ObjectCast<XCharactor>( hTarget );
		XGC_ASSERT_RETURN( pTarget, xgc_void( 0 ) );

		for( auto& item : pTarget->mHateMap )
			mHateMap.insert( { hTarget, item.second } );

		if( mMaxHateValue < pTarget->GetMaxHateValue() )
		{
			mMaxHateValue = pTarget->GetMaxHateValue();
			mMaxHateTarget = pTarget->GetMaxHateTarget();
		}

		if( mMinHateValue == 0 || mMinHateValue > pTarget->GetMinHateValue() )
		{
			mMinHateValue = pTarget->GetMinHateValue();
			mMinHateTarget = pTarget->GetMinHateTarget();
		}
		mAttacker = pTarget->GetAttacker();
	}

	xgc_void XCharactor::CheckSpecialHate( xObject hAttacker )
	{
		//删除最大最小仇恨目标时会重新遍历列表，找最大最小目标
		if( mHateMap.empty() )
			return;

		if( hAttacker == mMaxHateTarget )
		{
			auto it1 = mHateMap.begin();
			mMaxHateValue  = it1->second.iHateValue;
			mMaxHateTarget = it1->first;

			for( auto iter = it1; iter != mHateMap.end(); ++iter )
			{
				if( iter->second.iHateValue > mMaxHateValue )
				{
					mMaxHateTarget = iter->first;
					mMaxHateValue  = iter->second.iHateValue;
				}
			}
		}

		if( hAttacker == mMinHateTarget )
		{
			auto it1 = mHateMap.begin();

			mMinHateValue  = it1->second.iHateValue;
			mMinHateTarget = it1->first;
			for( auto iter = it1; iter != mHateMap.end(); ++iter )
			{
				if( iter->second.iHateValue < mMinHateValue )
				{
					mMinHateTarget = iter->first;
					mMinHateValue  = iter->second.iHateValue;
				}
			}
		}

		//如果删除的是第一个攻击我的目标，将仇恨值最大的设置为首击者
		if( hAttacker == mFirstAttackMeTarget )
			mFirstAttackMeTarget = mMaxHateTarget;

		if( hAttacker == mTarget )
			mTarget = mMaxHateTarget;
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

		auto it = mHateMap.begin();
		std::advance( it, random_range( 0ULL, mHateMap.size() ) );

		return it->first;
	}

	xObject XCharactor::GetNextAsTarget( xObject xTarget )
	{
		if( mHateMap.empty() )
			return INVALID_OBJECT_ID;

		auto it2 = mHateMap.find( xTarget );
		if( it2 == mHateMap.end() )
			return INVALID_OBJECT_ID;

		++it2;

		if( it2 == mHateMap.end() )
			return INVALID_OBJECT_ID;

		return it2->first;
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
