#include "XHeader.h"
#include "XActor.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// ��ɫ��������
	CORE_API xAttrIndex	attrActorIndex;			///< ��ɫ��������
	CORE_API xAttrIndex	attrActorType;			///< ��ɫ����
	CORE_API xAttrIndex attrActorHP;			///< ��ǰ����ֵ               
	CORE_API xAttrIndex attrActorHP_Max;		///< ��������    
	CORE_API xAttrIndex	attrActorBeatSpeed;		///< �����ٶ�
	CORE_API xAttrIndex	attrActorFaintTime;		///< ѣ��ʱ��
	CORE_API xAttrIndex	attrActorAbnormalTime;	///< ����״̬ʱ��
	CORE_API xAttrIndex	attrActorBornTime;		///< ����ʱ��
	CORE_API xAttrIndex	attrActorGroupMask;		///< ������룬����������Ӫ
	CORE_API xAttrIndex attrActorState;         ///< ��ɫ״̬
	CORE_API xAttrIndex attrActorCanMove;		///< ���ƶ�
	CORE_API xAttrIndex attrActorCanAttack;		///< �ɹ���
	CORE_API xAttrIndex attrActorCanBeHit;		///< ���ܻ�
	CORE_API xAttrIndex attrActorCanBeTanunt;	///< �ɳ���
	CORE_API xAttrIndex attrActorCanDead;		///< ������
	CORE_API xAttrIndex attrActorCanHurt;		///< ������

	IMPLEMENT_XCLASS_BEGIN( XActor, XObjectNode )
		IMPLEMENT_ATTRIBUTE( ActorIndex, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// ��ɫ��������
		IMPLEMENT_ATTRIBUTE( ActorType, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// ��ɫ����
		IMPLEMENT_ATTRIBUTE( ActorHP, VT_U32, ATTR_FLAG_SAVE, "20140912" )			// ��ǰ����ֵ
		IMPLEMENT_ATTRIBUTE( ActorHP_Max, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// ��������
		IMPLEMENT_ATTRIBUTE( ActorBeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// �����ٶ�
		IMPLEMENT_ATTRIBUTE( ActorFaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ѣ��ʱ��
		IMPLEMENT_ATTRIBUTE( ActorAbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )// ����״̬ʱ��
		IMPLEMENT_ATTRIBUTE( ActorBornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ����ʱ��
		IMPLEMENT_ATTRIBUTE( ActorGroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// ������룬����������Ӫ
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
		// ��ʼ����������
		mGameObject.InitObject();
		// �����¼�ת����Ŀ��Ϊ�Լ�
		mGameObject.RegistEvent( -1, std::bind( (XEventBind1)&XObject::EmmitEvent, this, _1 ), GetObjectID() );
	}

	XActor::~XActor( void )
	{
		// ��ע��Ķ�ʱ���Ӷ�ʱ���������Ƴ�
		getTimer().remove( mResetStatusTimerHandler );
		// ���Լ�ע�ᵽ��������е��¼�ȫ���Ƴ���
		mGameObject.RemoveEvent( GetObjectID() );
	}

	///
	/// \brief �յ�����
	/// \author albert.xu
	/// \date 11/19/2010
	///
	xgc_void XActor::UnderAttack( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext )
	{
		FUNCTION_BEGIN;
		// ��Ҫ��ʬ��̫��̬��
		if( isState( enActorState::sta_dead ) )
			return;

		xgc_long nDamage = 0;
		xgc_long nHatred = 0;

		// �����ܻ��¼�
		XActorEvent evt;
		evt.hAttacker = hAttacker;
		evt.lpContext = lpContext;
		evt.attack.nDamage = nDamage;
		evt.attack.nMode = nMode;

		EmmitEvent( evt_actor_behit, evt.cast );

		// �Ƿ����У����к󷵻��˺��ͳ��
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked �п��ܵݹ������������������
			// ֮ǰû����֮������
			enActorState eBefore = getStatus();
			XActor *pAttack = ObjectCast<XActor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt_actor_attacked, evt.cast );
			}

			//֮ǰ���˲��������߼���û�������
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
	/// ��ɫ����
	/////
	xgc_void XActor::Dead( xObject hAttacker, xgc_long nMode, xgc_lpvoid lpContext )
	{
		// �����ɫ���������¼�
		SetState( enActorState::sta_dead );

		resetAbility( abl_actor_move );
		resetAbility( abl_actor_attack );
		resetAbility( abl_actor_be_hit );
		resetAbility( abl_actor_be_tanunt );
		resetAbility( abl_actor_dead );
		resetAbility( abl_actor_hurt );
		
		// ���ͽ�ɫ�����¼�
		XActorEvent evt;
		evt.lpContext = lpContext;
		evt.hAttacker = hAttacker;

		evt.dead.nMode = nMode;
		EmmitEvent( evt_actor_dead, evt.cast );
	}

	///
	/// ��ɫ����
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XActor::Relive( xgc_lpvoid lpContext )
	{
		// �����������¼�
		SetState( enActorState::sta_live );

		// ���ͽ�ɫ�����¼�
		XActorEvent evt;
		evt.lpContext = lpContext;

		EmmitEvent( evt_actor_relive, evt.cast );
	}

	/// ------------------------------------------------ ///
	/// ��������״̬
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
	// ���ý�ɫ״̬
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
					case 0: // ����
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + duration,
						0,
						"once" );
					break;
					case 1: // ˳��
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + duration + remain,
						timespan( 0 ),
						"once" );
					break;
					case 2: // ȡ��С
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XActor::ResetActorState, this, eStatus ), 
						datetime::now() + XGC_MIN( duration, remain ),
						0,
						"once" );
					break;
					case 3: // ȡ���
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
	// Description:	���ý�ɫ״̬ 
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
