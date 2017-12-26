#include "XHeader.h"
#include "XActor.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 角色属性索引
	xAttrIndex XActor::Index;		///< 角色配置索引
	xAttrIndex XActor::Type;		///< 角色类型
	xAttrIndex XActor::HP;			///< 当前生命值
	xAttrIndex XActor::HP_Max;		///< 生命上限
	xAttrIndex XActor::BeatSpeed;	///< 击退速度
	xAttrIndex XActor::FaintTime;	///< 眩晕时间
	xAttrIndex XActor::AbnormalTime;///< 特殊状态时间
	xAttrIndex XActor::BornTime;	///< 出生时间
	xAttrIndex XActor::GroupMask;	///< 组别掩码，用于区分阵营
	xAttrIndex XActor::Status;		///< 角色状态
	xAttrIndex XActor::CanMove;		///< 可移动
	xAttrIndex XActor::CanAttack;	///< 可攻击
	xAttrIndex XActor::CanBeHit;	///< 可受击
	xAttrIndex XActor::CanBeTanunt;	///< 可嘲讽
	xAttrIndex XActor::CanDead;		///< 可死亡
	xAttrIndex XActor::CanHurt;		///< 可受伤

	IMPLEMENT_XCLASS_BEGIN( XActor, XGameObject )
		IMPLEMENT_ATTRIBUTE( Index, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// 角色配置索引
		IMPLEMENT_ATTRIBUTE( Type, VT_U32, ATTR_FLAG_SAVE, "20140912" )			// 角色类型
		IMPLEMENT_ATTRIBUTE( HP, VT_U32, ATTR_FLAG_SAVE, "20140912" )			// 当前生命值
		IMPLEMENT_ATTRIBUTE( HP_Max, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// 生命上限
		IMPLEMENT_ATTRIBUTE( BeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 击退速度
		IMPLEMENT_ATTRIBUTE( FaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 眩晕时间
		IMPLEMENT_ATTRIBUTE( AbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 特殊状态时间
		IMPLEMENT_ATTRIBUTE( BornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )		// 出生时间
		IMPLEMENT_ATTRIBUTE( GroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// 组别掩码，用于区分阵营
		IMPLEMENT_ATTRIBUTE( Status, VT_U32, ATTR_FLAG_SAVE, "20150122" )
		IMPLEMENT_ATTRIBUTE( CanMove, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( CanAttack, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( CanBeHit, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( CanBeTanunt, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( CanDead, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( CanHurt, VT_I32, ATTR_FLAG_NONE, "20150122" )
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

	XActor::XActor()
		: mResetStatusTimerHandler( INVALID_TIMER_HANDLE )
		, mActorRestonState( enActorState::sta_live )
		, mRadius( 1.0f )
		, mBornPoint( XVector3::ZERO )
		, mAttacker( INVALID_OBJECT_ID )
		, mTarget( INVALID_OBJECT_ID )
		, mFriend( INVALID_OBJECT_ID )
		, mFightState( false )
	{
	}

	XActor::~XActor( void )
	{
		// 将注册的定时器从定时器队列里移除
		getTimer().remove( mResetStatusTimerHandler );
	}

	///
	/// \brief 收到攻击
	/// \author albert.xu
	/// \date 11/19/2010
	///
	xgc_void XActor::UnderAttack( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext )
	{
		FUNCTION_BEGIN;
		// 不要鞭尸，太变态了
		if( isState( enActorState::sta_dead ) )
			return;

		xgc_long nDamage = 0;
		xgc_long nHatred = 0;

		// 发送受击事件
		XActorEvent evt;
		evt.hAttacker = hAttacker;
		evt.lpContext = lpContext;
		evt.attack.nDamage = nDamage;
		evt.attack.nMode = nMode;

		EmmitEvent( evt.cast, evt_actor_behit );

		// 是否命中，命中后返回伤害和仇恨
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked 有可能递归调用死亡，会多次死亡
			// 之前没死，之后死了
			enActorState eBefore = getStatus();
			XActor *pAttack = ObjectCast<XActor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt.cast, evt_actor_attacked );
			}

			//之前死了才做死亡逻辑，没死则忽略
			if( enActorState::sta_dead == eBefore &&
				enActorState::sta_dead == getStatus() )
			{
				Dead( hAttacker, nMode, lpContext );
			}
		}
		FUNCTION_END;
	}

	/////
	/// [8/21/2009 Albert]
	/// 角色死亡
	/////
	xgc_void XActor::Dead( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext )
	{
		// 处理角色场景死亡事件
		SetState( enActorState::sta_dead );

		resetAbility( abl_actor_move );
		resetAbility( abl_actor_attack );
		resetAbility( abl_actor_be_hit );
		resetAbility( abl_actor_be_tanunt );
		resetAbility( abl_actor_dead );
		resetAbility( abl_actor_hurt );
		
		// 发送角色死亡事件
		XActorEvent evt;
		evt.lpContext = lpContext;
		evt.hAttacker = hAttacker;

		evt.dead.nMode = nMode;
		EmmitEvent( evt.cast, evt_actor_dead );
	}

	///
	/// 角色重生
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XActor::Relive( xgc_lpvoid lpContext )
	{
		// 处理场景重生事件
		SetState( enActorState::sta_live );

		// 发送角色重生事件
		XActorEvent evt;
		evt.lpContext = lpContext;

		EmmitEvent( evt.cast, evt_actor_relive );
	}

	/// ------------------------------------------------ ///
	/// 设置生物状态
	/// [5/31/2014 jianglei.kinly]
	/// ------------------------------------------------ ///
	xgc_void XActor::SetState( enActorState eStatus )
	{
		if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mResetStatusTimerHandler );
			mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
		}

		setValue( Status, (xgc_uint32)eStatus );
		mActorRestonState = eStatus;
	}

	//---------------------------------------------------//
	// [8/24/2009 Albert]
	// 设置角色状态
	//---------------------------------------------------//
	xgc_void XActor::SetState( enActorState eStatus, timespan tsDuration, xgc_int32 nMode )
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
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + duration,
						0,
						"once" );
					break;
					case 1: // 顺延
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + duration + remain,
						timespan( 0 ),
						"once" );
					break;
					case 2: // 取最小
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + XGC_MIN( duration, remain ),
						0,
						"once" );
					break;
					case 3: // 取最大
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + XGC_MAX( duration, remain ),
						0,
						"once" );
					break;
				}
			}
			else
			{
				mResetStatusTimerHandler = getTimer().insert( 
					std::bind( &XActor::ResetActorState, this, eStatus ), 
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
				enableAbility( abl_actor_move );
			else
				enableAbility( abl_actor_move, false );

			SetState( eStatus );
		}
	}

	//---------------------------------------------------//
	// [12/23/2010 Albert]
	// Description:	重置角色状态 
	//---------------------------------------------------//
	xgc_void XActor::ResetActorState( enActorState eStatus )
	{
		if( !isState( enActorState::sta_dead ) )
			SetState( eStatus );

		if( !isState( enActorState::sta_live ) )
			enableAbility( abl_actor_move );
		else
			enableAbility( abl_actor_move, false );

		mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
	}
}
