#include "XHeader.h"
#include "XActor.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// ��ɫ��������
	xAttrIndex XActor::Index;		///< ��ɫ��������
	xAttrIndex XActor::Type;		///< ��ɫ����
	xAttrIndex XActor::HP;			///< ��ǰ����ֵ
	xAttrIndex XActor::HP_Max;		///< ��������
	xAttrIndex XActor::BeatSpeed;	///< �����ٶ�
	xAttrIndex XActor::FaintTime;	///< ѣ��ʱ��
	xAttrIndex XActor::AbnormalTime;///< ����״̬ʱ��
	xAttrIndex XActor::BornTime;	///< ����ʱ��
	xAttrIndex XActor::GroupMask;	///< ������룬����������Ӫ
	xAttrIndex XActor::Status;		///< ��ɫ״̬
	xAttrIndex XActor::CanMove;		///< ���ƶ�
	xAttrIndex XActor::CanAttack;	///< �ɹ���
	xAttrIndex XActor::CanBeHit;	///< ���ܻ�
	xAttrIndex XActor::CanBeTanunt;	///< �ɳ���
	xAttrIndex XActor::CanDead;		///< ������
	xAttrIndex XActor::CanHurt;		///< ������

	IMPLEMENT_XCLASS_BEGIN( XActor, XGameObject )
		IMPLEMENT_ATTRIBUTE( Index, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// ��ɫ��������
		IMPLEMENT_ATTRIBUTE( Type, VT_U32, ATTR_FLAG_SAVE, "20140912" )			// ��ɫ����
		IMPLEMENT_ATTRIBUTE( HP, VT_U32, ATTR_FLAG_SAVE, "20140912" )			// ��ǰ����ֵ
		IMPLEMENT_ATTRIBUTE( HP_Max, VT_U32, ATTR_FLAG_SAVE, "20140912" )		// ��������
		IMPLEMENT_ATTRIBUTE( BeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// �����ٶ�
		IMPLEMENT_ATTRIBUTE( FaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ѣ��ʱ��
		IMPLEMENT_ATTRIBUTE( AbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ����״̬ʱ��
		IMPLEMENT_ATTRIBUTE( BornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )		// ����ʱ��
		IMPLEMENT_ATTRIBUTE( GroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// ������룬����������Ӫ
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
		// ��ע��Ķ�ʱ���Ӷ�ʱ���������Ƴ�
		getTimer().remove( mResetStatusTimerHandler );
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

		EmmitEvent( evt.cast, evt_actor_behit );

		// �Ƿ����У����к󷵻��˺��ͳ��
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked �п��ܵݹ������������������
			// ֮ǰû����֮������
			enActorState eBefore = getStatus();
			XActor *pAttack = ObjectCast<XActor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt.cast, evt_actor_attacked );
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
		EmmitEvent( evt.cast, evt_actor_dead );
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

		EmmitEvent( evt.cast, evt_actor_relive );
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

		setValue( Status, (xgc_uint32)eStatus );
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
