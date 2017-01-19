#include "StdAfx.h"
#include "XCharactor.h"
#include "XSkillBase.h"
#include "xsystem.h"

namespace XGC
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
	CORE_API xAttrIndex attrActorStatus;        ///< ��ɫ״̬

	BEGIN_IMPLEMENT_XCLASS( XCharactor, XGameObject, TypeXCharactor )
		IMPLEMENT_ATTRIBUTE( ActorIndex, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// ��ɫ��������
		IMPLEMENT_ATTRIBUTE( ActorType, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// ��ɫ����
		IMPLEMENT_ATTRIBUTE( ActorBeatSpeed, VT_I32, ATTR_FLAG_NONE, "20140912" )	// �����ٶ�
		IMPLEMENT_ATTRIBUTE( ActorFaintTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ѣ��ʱ��
		IMPLEMENT_ATTRIBUTE( ActorAbnormalTime, VT_I32, ATTR_FLAG_NONE, "20140912" )	// ����״̬ʱ��
		IMPLEMENT_ATTRIBUTE( ActorBornTime, VT_I32, ATTR_FLAG_NONE, "20140912" )  // ����ʱ��
		IMPLEMENT_ATTRIBUTE( ActorGroupMask, VT_I32, ATTR_FLAG_SAVE, "20140912" )	// ������룬����������Ӫ
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

	xObjectSet XCharactor::static_error_res;  ///< �����

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
		mStatusFlags = { 0 }; // ����״̬��־λ
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
		// ��Ҫ��ʬ��̫��̬��
		if( GetActorStatus() == ActorStatus_Dead )
			return;

		xgc_long nDamage = 0;
		xgc_long nHatred = 0;

		// �Ƿ����У����к󷵻��˺��ͳ��
		if( OnHit( hAttacker, eMode, lpContext, nDamage, nHatred ) )
		{
			if( hAttacker != INVALID_OBJECT_ID )
			{
				mAttacker = hAttacker;
			}
		}
        //OnUnderAttacked �п��ܵݹ������������������
        //֮ǰû����֮������
        ActorStatus eBefore = GetActorStatus();
        XCharactor *pAttack = ObjectCast<XCharactor>(hAttacker);
        if (pAttack)
            pAttack->OnAttacked(GetObjectID(), eMode, lpContext, nDamage);
        OnUnderAttacked(hAttacker, eMode, lpContext, nDamage);

        //֮ǰ���˲��������߼���û�������
  		if( eBefore == ActorStatus_Dead && ActorStatus_Dead == GetActorStatus() )
		{
			Dead( hAttacker, eMode, lpContext );
		}
		FUNCTION_END;
	}

	/////
	/// [8/21/2009 Albert]
	/// ��ɫ����
	/////
	xgc_void XCharactor::Dead( xObject hAttacker, ActorAttackMode eMode, xgc_lpvoid lpContext )
	{
		// �����ɫ���������¼�
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
	/// ��ɫ����
	/// [11/20/2014] create by albert.xu
	///
	xgc_void XCharactor::Relive( xgc_lpvoid lpContext )
	{
		// �����������¼�
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
	/// ��������״̬
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
	// ���ý�ɫ״̬
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
					case 0: // ����
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) fTime );
					break;
					case 1: // ˳��
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) ( fTime + fRelease ) );
					break;
					case 2: // ȡ��С
					mResetStatusTimerHandler = getTimer().insert_event( bind( &XCharactor::ResetActorStatus, this, _1, eStatus ), 1, 0.0f, (xgc_real32) XGC_MIN( fTime, fRelease ) );
					break;
					case 3: // ȡ���
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
	// Description:	���ý�ɫ״̬ 
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
	// Description:	������Ķ���Ϊ��ע�����ͣ�����й���
	// ��Ӽ���Child
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
	// Description:	���Ƴ��Ķ���Ϊ��ע��������б���ȡ�� 
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
				XGC_ASSERT_MESSAGE( false, "��ɾ��һ�������ڸö������ϵļ��ܣ�%u", pChild->GetUnsignedAttr( attrSkillMainId ) );
		}
		else if( pChild->IsInheritFrom( TypeXStateBase ) )
		{
			auto key = std::make_pair( pChild->GetUnsignedAttr( attrStateType ), pChild->GetUnsignedAttr( attrStateBuffID ) );
			auto it = mStateMap.find( key );
			if( it != mStateMap.end() )
			{
				it->second.erase( pChild->GetObjectID() );
				// ��������size == 0 ���������
				if( it->second.empty() )
				{
					mStateMap.erase( it );
				}
			}
			else
				XGC_ASSERT_MESSAGE( false, "��ɾ��һ�������ڸö������ϵ�״̬��%u, %u", pChild->GetUnsignedAttr( attrStateType ), pChild->GetUnsignedAttr( attrStateBuffID ) );
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
				//��ȡ����ǰ������֮��ĳ��ֵ�������Ϊ������
				if( mFirstAttackMeTarget != mMaxHateTarget )
				{
					//����Ȩ����ֵ�����
					mFirstAttackMeTarget = mMaxHateTarget;
				}
				else if( mFirstAttackMeTarget == mMaxHateTarget && mFirstAttackMeTarget == mMinHateTarget )
				{
					//ֻ��һ���ˣ�����Ȩ�����
					return;
				}
				else
				{
					//�������ߣ������ǳ��ֵ�������Ȩ����Լ�֮��ĳ��ֵ�����
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
		//ɾ�������С���Ŀ��ʱ�����±����б��������СĿ��
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
		//���ɾ�����ǵ�һ�������ҵ�Ŀ�꣬�����ֵ��������Ϊ�׻���
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
