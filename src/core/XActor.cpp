#include "XHeader.h"
#include "XActor.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 角色属性索引
	CORE_API xAttrIndex	attrActorIndex;			///< 角色配置索引
	CORE_API xAttrIndex	attrActorType;			///< 角色类型
	CORE_API xAttrIndex attrActorHP;			///< 当前生命值               
	CORE_API xAttrIndex attrActorHP_Max;		///< 生命上限    
	CORE_API xAttrIndex	attrActorBeatSpeed;		///< 击退速度
	CORE_API xAttrIndex	attrActorFaintTime;		///< 眩晕时间
	CORE_API xAttrIndex	attrActorAbnormalTime;	///< 特殊状态时间
	CORE_API xAttrIndex	attrActorBornTime;		///< 出生时间
	CORE_API xAttrIndex	attrActorGroupMask;		///< 组别掩码，用于区分阵营
	CORE_API xAttrIndex attrActorState;         ///< 角色状态
	CORE_API xAttrIndex attrActorCanMove;		///< 可移动
	CORE_API xAttrIndex attrActorCanAttack;		///< 可攻击
	CORE_API xAttrIndex attrActorCanBeHit;		///< 可受击
	CORE_API xAttrIndex attrActorCanBeTanunt;	///< 可嘲讽
	CORE_API xAttrIndex attrActorCanDead;		///< 可死亡
	CORE_API xAttrIndex attrActorCanHurt;		///< 可受伤

	IMPLEMENT_XCLASS_BEGIN( XActor, XObjectNode )
		IMPLEMENT_ATTRIBUTE( ActorIndex, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// 角色配置索引
		IMPLEMENT_ATTRIBUTE( ActorType, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// 角色类型
		IMPLEMENT_ATTRIBUTE( ActorHP, VT_U32, ATTR_FLAG_SAVE, "20140912" )			// 当前生命值
		IMPLEMENT_ATTRIBUTE( ActorHP_Max, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// 生命上限
		IMPLEMENT_ATTRIBUTE( ActorBeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 击退速度
		IMPLEMENT_ATTRIBUTE( ActorFaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 眩晕时间
		IMPLEMENT_ATTRIBUTE( ActorAbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )// 特殊状态时间
		IMPLEMENT_ATTRIBUTE( ActorBornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// 出生时间
		IMPLEMENT_ATTRIBUTE( ActorGroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// 组别掩码，用于区分阵营
		IMPLEMENT_ATTRIBUTE( ActorState, VT_U32, ATTR_FLAG_SAVE, "20150122" )
		IMPLEMENT_ATTRIBUTE( ActorCanMove, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( ActorCanAttack, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( ActorCanBeHit, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( ActorCanBeTanunt, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( ActorCanDead, VT_I32, ATTR_FLAG_NONE, "20150122" )
		IMPLEMENT_ATTRIBUTE( ActorCanHurt, VT_I32, ATTR_FLAG_NONE, "20150122" )
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
		: XObjectNode()
		, mGameObject( GetObjectID() )
		, mResetStatusTimerHandler( INVALID_TIMER_HANDLE )
		, mActorRestonState( enActorState::sta_live )
		, mRadius( 1.0f )
		, mBornPoint( XVector3::ZERO )
		, mAttacker( INVALID_OBJECT_ID )
		, mTarget( INVALID_OBJECT_ID )
		, mFriend( INVALID_OBJECT_ID )
		, mFightState( false )
	{
		// 初始化对象属性
		mGameObject.InitObject();
		// 设置事件转发的目标为自己
		mGameObject.RegistEvent( -1, std::bind( (XEventBind1)&XObject::EmmitEvent, this, _1 ), GetObjectID() );
	}

	XActor::~XActor( void )
	{
		// 将注册的定时器从定时器队列里移除
		getTimer().remove( mResetStatusTimerHandler );
		// 将自己注册到代理对象中的事件全部移除。
		mGameObject.RemoveEvent( GetObjectID() );
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

		EmmitEvent( evt_actor_behit, evt.cast );

		// 是否命中，命中后返回伤害和仇恨
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked 有可能递归调用死亡，会多次死亡
			// 之前没死，之后死了
			enActorState eBefore = getStatus();
			XActor *pAttack = ObjectCast<XActor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt_actor_attacked, evt.cast );
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
		EmmitEvent( evt_actor_dead, evt.cast );
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

		EmmitEvent( evt_actor_relive, evt.cast );
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

		setValue( attrActorState, (xgc_uint32)eStatus );
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
