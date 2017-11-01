#include "XHeader.h"
#include "XCharactor.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// ��ɫ��������
	CORE_API xAttrIndex	attrActorIndex;			///< ��ɫ��������
	CORE_API xAttrIndex	attrActorType;			///< ��ɫ����
	CORE_API xAttrIndex attrHP;					///< ��ǰ����ֵ               
	CORE_API xAttrIndex attrMaxHP;				///< ��������    
	CORE_API xAttrIndex	attrActorBeatSpeed;		///< �����ٶ�
	CORE_API xAttrIndex	attrActorFaintTime;		///< ѣ��ʱ��
	CORE_API xAttrIndex	attrActorAbnormalTime;	///< ����״̬ʱ��
	CORE_API xAttrIndex	attrActorBornTime;		///< ����ʱ��
	CORE_API xAttrIndex	attrActorGroupMask;		///< ������룬����������Ӫ
	CORE_API xAttrIndex attrActorState;         ///< ��ɫ״̬
	CORE_API xAttrIndex attrActorAbility;       ///< ��ɫ����

	IMPLEMENT_XCLASS_BEGIN( XCharactor, XObjectNode )
		IMPLEMENT_ATTRIBUTE( ActorIndex, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// ��ɫ��������
		IMPLEMENT_ATTRIBUTE( ActorType, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// ��ɫ����
		IMPLEMENT_ATTRIBUTE( ActorBeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// �����ٶ�
		IMPLEMENT_ATTRIBUTE( ActorFaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ѣ��ʱ��
		IMPLEMENT_ATTRIBUTE( ActorAbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ����״̬ʱ��
		IMPLEMENT_ATTRIBUTE( ActorBornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )  // ����ʱ��
		IMPLEMENT_ATTRIBUTE( ActorGroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// ������룬����������Ӫ
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
		mStatusFlags = { 0 }; // ����״̬��־λ
	}

	XCharactor::~XCharactor( void )
	{
		getTimer().remove( mResetStatusTimerHandler );
	}

	///
	/// \brief �յ�����
	/// \author albert.xu
	/// \date 11/19/2010
	///
	xgc_void XCharactor::UnderAttack( xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext )
	{
		FUNCTION_BEGIN;
		// ��Ҫ��ʬ��̫��̬��
		if( isState( enActorState::sta_dead ) )
			return;

		xgc_long nDamage = 0;
		xgc_long nHatred = 0;

		// �����ܻ��¼�
		XCharactorEvent evt;
		evt.hAttacker = hAttacker;
		evt.lpContext = lpContext;
		evt.attack.nDamage = nDamage;
		evt.attack.eMode = eMode;

		EmmitEvent( evt_actor_behit, evt.cast );

		// �Ƿ����У����к󷵻��˺��ͳ��
		if( evt.cast.result == 0 )
		{
			// OnUnderAttacked �п��ܵݹ������������������
			// ֮ǰû����֮������
			enActorState eBefore = getStatus();
			XCharactor *pAttack = ObjectCast<XCharactor>( hAttacker );
			if( pAttack )
			{
				pAttack->EmmitEvent( evt_actor_attacked, evt.cast );
			}

			//֮ǰ���˲��������߼���û�������
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
	/// ��ɫ����
	/////
	xgc_void XCharactor::Dead( xObject hAttacker, enAttackMode eMode, xgc_lpvoid lpContext )
	{
		// �����ɫ���������¼�
		SetState( enActorState::sta_dead );
		SetEnjoinAttack();
		SetEnjoinMove();
		SetEnjoinUnderAttack();

		// ���ͽ�ɫ�����¼�
		XCharactorEvent evt;
		evt.lpContext = lpContext;
		evt.hAttacker = hAttacker;

		evt.dead.eMode = eMode;
		EmmitEvent( evt_actor_dead, evt.cast );
	}

	///
	/// ��ɫ����
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XCharactor::Relive( xgc_lpvoid lpContext )
	{
		// �����������¼�
		SetState( enActorState::sta_live );

		// ���ͽ�ɫ�����¼�
		XCharactorEvent evt;
		evt.lpContext = lpContext;

		EmmitEvent( evt_actor_relive, evt.cast );
	}

	/// ------------------------------------------------ ///
	/// ��������״̬
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
	// ���ý�ɫ״̬
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
					case 0: // ����
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + duration,
						0,
						"once" );
					break;
					case 1: // ˳��
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + duration + remain,
						timespan( 0 ),
						"once" );
					break;
					case 2: // ȡ��С
					mResetStatusTimerHandler = getTimer().insert( 
						std::bind( &XCharactor::ResetActorState, this, eStatus ), 
						datetime::now() + XGC_MIN( duration, remain ),
						0,
						"once" );
					break;
					case 3: // ȡ���
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
	// Description:	���ý�ɫ״̬ 
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

		//�����һ���ˣ�Ҫͬʱ���������СĿ��
		if( 0 == mMaxHateValue || nTmp >= mMaxHateValue )
		{
			mMaxHateTarget = hAttacker;
			mMaxHateValue = nTmp;
		}
		//�����һ���ˣ�Ҫͬʱ���������СĿ��
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
		//ɾ�������С���Ŀ��ʱ�����±����б��������СĿ��
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

		//���ɾ�����ǵ�һ�������ҵ�Ŀ�꣬�����ֵ��������Ϊ�׻���
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
