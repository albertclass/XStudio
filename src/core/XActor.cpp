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
	xAttrIndex XActor::State;		///< ��ɫ״̬
	xAttrIndex XActor::Status;		///< ��ɫ�쳣״̬
	xAttrIndex XActor::StatusTime;	///< ��ɫ�쳣״̬����ʱ��
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
		IMPLEMENT_ATTRIBUTE( StatusTime, VT_U32, ATTR_FLAG_SAVE, "20150122" )
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
		, mRadius( 1.0f )
		, mBornPoint( XVector3::ZERO )
		, mAttacker( INVALID_OBJECT_ID )
		, mTarget( INVALID_OBJECT_ID )
		, mFriend( INVALID_OBJECT_ID )
		, mFightState( false )
	{
		AddComponent< XGameObject >();
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

		// �����ܻ��¼�
		AttackEvent evt;
		evt.hAttacker = hAttacker;
		evt.hTarget   = GetObjectID();
		evt.nDamage   = 0;
		evt.nHate     = 0;
		evt.nMode     = nMode;
		evt.lpContext = lpContext;

		// ���м���
		beHit( evt );

		// �Ƿ����У����к󷵻��˺��ͳ��
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked �п��ܵݹ������������������
			// ֮ǰû����֮������
			enActorState eBefore = getState();

			// �˺�����
			doAttack( evt );
			XActor *pAttack = ObjectCast<XActor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt.cast, evt_actor_attacked );
			}

			//֮ǰ���˲��������߼���û�������
			if( enActorState::sta_dead != eBefore &&
				enActorState::sta_dead == getState() )
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
		setState( enActorState::sta_dead );

		enableAbility( abl_actor_move, false );
		enableAbility( abl_actor_attack, false );
		enableAbility( abl_actor_be_hit, false );
		enableAbility( abl_actor_be_tanunt, false );
		enableAbility( abl_actor_dead, false );
		enableAbility( abl_actor_hurt, false );
		
		// ���ͽ�ɫ�����¼�
		AttackEvent evt;
		evt.lpContext = lpContext;
		evt.hAttacker = hAttacker;
		evt.hTarget   = GetObjectID();
		evt.nMode     = nMode;
		evt.lpContext = lpContext;

		EmmitEvent( evt.cast, evt_actor_dead );
	}

	///
	/// ��ɫ����
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XActor::Relive( xgc_lpvoid lpContext )
	{
		// �����������¼�
		setState( enActorState::sta_live );

		resetAbility( abl_actor_move );
		resetAbility( abl_actor_attack );
		resetAbility( abl_actor_be_hit );
		resetAbility( abl_actor_be_tanunt );
		resetAbility( abl_actor_dead );
		resetAbility( abl_actor_hurt );

		// ���ͽ�ɫ�����¼�
		NormalEvent evt;
		evt.lpContext = lpContext;

		EmmitEvent( InitEvent( evt.cast, evt_actor_relive ) );
	}

	///
	/// \brief ��������
	/// \author jianglei.kinly
	///
	xgc_void XActor::setStatus( enActorStatus eStatus )
	{
		if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mResetStatusTimerHandler );
			mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
		}

		setValue( Status, (xgc_uint32)eStatus );
	}

	///
	/// \brief ���ý�ɫ״̬
	/// \date 8/24/2009
	/// \author albert.xu
	///
	xgc_void XActor::setStatus( enActorStatus eStatus, timespan tsDuration, xgc_int32 nMode )
	{
		if( isState( enActorState::sta_dead ) )
			return;

		XGC_ASSERT_RETURN( tsDuration == 0, XGC_NONE );

		timespan duration = tsDuration;
		timespan remain;
		datetime reston;

		if( mResetStatusTimerHandler != INVALID_TIMER_HANDLE )
		{
			remain = getTimer().remove( mResetStatusTimerHandler );
			mResetStatusTimerHandler = INVALID_TIMER_HANDLE;
		}

		// ��ͬ��״̬�ظ���
		switch( nMode )
		{
			case 0: // ����
			reston = datetime::now() + duration;
			break;
			case 1: // ˳��
			reston = datetime::now() + duration + remain;
			break;
			case 2: // ȡ��С
			reston = datetime::now() + XGC_MIN( duration, remain );
			break;
			case 3: // ȡ���
			reston = datetime::now() + XGC_MAX( duration, remain );
			break;
		}

		// ��������
		setStatus( eStatus );

		// ����״̬�ָ��Ķ�ʱ��
		if( reston > datetime::now() )
		{
			mResetStatusTimerHandler = getTimer().insert(
				std::bind( (xgc_void (XActor::*)( enActorStatus ) )&XActor::setStatus, this, stu_actor_normal ),
				reston, 0, "once" );

			setValue( StatusTime, reston.to_milliseconds() );
		}
	}
}
