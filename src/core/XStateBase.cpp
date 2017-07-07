/*******************************************************************/
//! \file XStateBase.cpp
//! \brief ״̬����
//! 2014/06/11 by jianglei.kinly
/*******************************************************************/
#include "stdafx.h"
#include "XStateBase.h"
#include "XAction.h"
#include "XCharactor.h"

namespace XGC
{
#define OVERLAP_TIMER_STR_HEAD "StateOverlap_"
#define JUMPGS_CURROVERLAPTIMEVALUE "JumpGsCurrOverlapTimeVal_"
	//////////////////////////////////////////////////////////////////////////
	// buff��������
	extern "C"
	{
		CORE_API xAttrIndex attrStateIndex;             ///< buff����
		CORE_API xAttrIndex attrStateBuffID;            ///< ��ID
		CORE_API xAttrIndex attrStateType;              ///< ����
		CORE_API xAttrIndex attrStateIsRing;            ///< �Ƿ��ǹ⻷���buff
		CORE_API xAttrIndex attrStateTriggerTimeval;    ///< ����ʱ����
		CORE_API xAttrIndex attrStateTriggerTimes;      ///< ��������
		CORE_API xAttrIndex attrStateOverlap;           ///< ���Ӳ���
		CORE_API xAttrIndex attrStateOverlapSubTimeval; ///< �ɵ�����buff�ĳ���ʱ��
		CORE_API xAttrIndex attrStateOverlapType;       ///< buff����ʱ�Ƿ�ˢ��ʱ��

		// ��GS�õ�
		CORE_API xAttrIndex attrStateCurrTriggerTimeval; ///< �����´δ�����ʱ����
		CORE_API xAttrIndex attrStateCurrTriggerTimes;   ///< ��ǰ��������
		CORE_API xAttrIndex attrStateCurrOverlapNum;     ///< ��ǰ���Ӳ���
		CORE_API xAttrIndex attrStateCurrLastTime;       ///< buffʣ��ʱ�������ڷ����ͻ���
		CORE_API xAttrIndex attrStateCurrOverlapTimes;   ///< ��ǰ���Ӵ�����������϶�ʱ��Key��ʾ��
	}

	BEGIN_IMPLEMENT_XCLASS( XStateBase, XObject, TypeXStateBase )
		IMPLEMENT_ATTRIBUTE( StateIndex, VT_U32, ATTR_FLAG_NONE, "20140912" )       ///< buff����
		IMPLEMENT_ATTRIBUTE( StateBuffID, VT_U32, ATTR_FLAG_NONE, "20140912" )       ///< ��ID
		IMPLEMENT_ATTRIBUTE( StateType, VT_U32, ATTR_FLAG_NONE, "20140912" )        ///< ����
		IMPLEMENT_ATTRIBUTE( StateIsRing, VT_BOOL, ATTR_FLAG_NONE, "20140912" )     ///< �Ƿ��ǹ⻷���
		IMPLEMENT_ATTRIBUTE( StateTriggerTimeval, VT_REAL, ATTR_FLAG_NONE, "20140912" )    ///< ����ʱ����
		IMPLEMENT_ATTRIBUTE( StateTriggerTimes, VT_I32, ATTR_FLAG_NONE, "20140912" )      ///< ��������
		IMPLEMENT_ATTRIBUTE( StateOverlap, VT_U32, ATTR_FLAG_NONE, "20140912" )           ///< ���Ӳ���
		IMPLEMENT_ATTRIBUTE( StateOverlapSubTimeval, VT_REAL, ATTR_FLAG_NONE, "20140912" ) ///< �ɵ�����buff�ĳ���ʱ��
		IMPLEMENT_ATTRIBUTE( StateOverlapType, VT_U32, ATTR_FLAG_NONE, "20140912" )       ///< buff����ʱ�Ƿ�ˢ��ʱ��
		IMPLEMENT_ATTRIBUTE( StateCurrTriggerTimeval, VT_REAL, ATTR_FLAG_NONE, "20140912" ) ///< �����´δ�����ʱ����
		IMPLEMENT_ATTRIBUTE( StateCurrTriggerTimes, VT_I32, ATTR_FLAG_NONE, "20140912" )   ///< ��ǰ��������
		IMPLEMENT_ATTRIBUTE( StateCurrOverlapNum, VT_U32, ATTR_FLAG_NONE, "20140912" )     ///< ��ǰ���Ӳ���
		IMPLEMENT_ATTRIBUTE( StateCurrLastTime, VT_U32, ATTR_FLAG_NONE, "20140912" )     ///< buffʣ��ʱ�������ڷ����ͻ���
		IMPLEMENT_ATTRIBUTE( StateCurrOverlapTimes, VT_U32, ATTR_FLAG_NONE, "20140912" )     ///< ��ǰ���Ӵ�����������϶�ʱ��Key��ʾ��
	END_IMPLEMENT_XCLASS();

	XStateBase::XStateBase()
		: XObject()
		, m_StateTimer( INVALID_TIMER_HANDLE )
		, m_StateCheckTimer( INVALID_TIMER_HANDLE )
	{
	}

	enum EnRealCompareZero
	{
		// ����
		enMoreThan,
		// С��
		enLessThan,
		// ����
		enEquap,
	};

	///
	/// �Ƚϸ����ͺ�0�ıȽ�
	/// [8/15/2014] create by jianglei.kinly
	///
	auto FnRealCompareZeroInState = []( xgc_real32 real )->EnRealCompareZero
	{
		return xgc_int32( real * 100 ) > 0 ? enMoreThan : xgc_int32( real * 100 ) < 0 ? enLessThan : enEquap;
	};

	XStateBase::~XStateBase( void )
	{
		if( m_StateTimer != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( m_StateTimer );
			m_StateTimer = INVALID_TIMER_HANDLE;
		}
		if( m_StateCheckTimer != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( m_StateCheckTimer );
			m_StateCheckTimer = INVALID_TIMER_HANDLE;
		}
		// ɾ��Overlap�õĶ�ʱ��
		for( auto iter = mMapOverlapTimer.begin(); iter != mMapOverlapTimer.end(); ++iter )
		{
			getTimer().remove_event( iter->second );
		}
		mMapOverlapTimer.clear();
	}

	xgc_void XStateBase::OnCheck()
	{
		TriggerStateCheck();
	}

	xgc_void XStateBase::OnAdd( xgc_uint32 overlapNum /*= 1 */, xgc_uint32 overlapTimeVal, xgc_bool isUpdate )
	{
		XGC_ASSERT_MESSAGE( m_StateTimer == INVALID_TIMER_HANDLE, "���и�buff��" );

		if( FnRealCompareZeroInState( GetAttribute( attrStateTriggerTimeval ).toReal() ) == enMoreThan )
		{
			XVariant::Real triggertimedelay = FnRealCompareZeroInState( GetAttribute( attrStateCurrTriggerTimeval ).toReal() ) == enMoreThan ?
				GetAttribute( attrStateCurrTriggerTimeval ).toReal() : FnRealCompareZeroInState( GetAttribute( attrStateTriggerTimeval ).toReal() ) == enMoreThan ? GetAttribute( attrStateTriggerTimeval ).toReal() : 0.0f;

			m_StateTimer = getTimer().insert_event(
				bind( &XStateBase::OnTrigger, this ),
				GetAttribute( attrStateCurrTriggerTimes ).toInteger() != 0 ? GetAttribute( attrStateTriggerTimes ).toInteger() - GetAttribute( attrStateCurrTriggerTimes ).toInteger() : GetAttribute( attrStateTriggerTimes ).toInteger(),
				GetAttribute( attrStateTriggerTimeval ).toReal(),
				triggertimedelay );

			GetAttribute( attrStateCurrLastTime ) = ( GetAttribute( attrStateCurrTriggerTimes ).toInteger() != 0 ? GetAttribute( attrStateTriggerTimes ).toInteger() - GetAttribute( attrStateCurrTriggerTimes ).toInteger() : GetAttribute( attrStateTriggerTimes ).toInteger() - 1 )
				* GetAttribute( attrStateTriggerTimeval ).toReal() + triggertimedelay;
		}
		else
		{
			GetAttribute( attrStateCurrLastTime ) = std::numeric_limits<xgc_uint32>::max();
		}

		if( GetBoolAttr( attrStateIsRing ) )
		{
			m_StateCheckTimer = getTimer().insert_event(
				bind( &XStateBase::OnCheck, this ), -1, 2, 1 );
		}

		TriggerStateAdd( overlapNum, overlapTimeVal, isUpdate );
	}

	xgc_void XStateBase::OnOverlapAdd( xgc_uint32 addNum, xgc_uint32 overlapTimeVal, xgc_bool bTrigger )
	{
		XGC_ASSERT_RETURN( GetParent() != INVALID_OBJECT_ID, xgc_void( 0 ) );

		// �������ӵĲ���
		XVariant::Unsigned realAddNum = addNum == 0 ? 1 : addNum;

		// OverlapNum�ﵽ���ֵ��ʱ��������
		if( GetAttribute( attrStateOverlap ).toUnsigned() <= GetAttribute( attrStateCurrOverlapNum ).toUnsigned()
			&& GetAttribute( attrStateOverlapType ).toUnsigned() != en_overlaptype_update )
		{
			return;
		}
		if( GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_accumulate )
		{
			GetAttribute( attrStateCurrOverlapNum ) = 1;
			GetAttribute( attrStateCurrOverlapTimes ) = 1;
		}
		else
		{
			// ��realrealAddNum�����
			if( GetAttribute( attrStateCurrOverlapNum ).toUnsigned() + addNum > GetAttribute( attrStateOverlap ).toUnsigned() )
				realAddNum = GetAttribute( attrStateOverlap ).toUnsigned() - GetAttribute( attrStateCurrOverlapNum ).toUnsigned();
			GetAttribute( attrStateCurrOverlapNum ) += realAddNum;
			GetAttribute( attrStateCurrOverlapTimes ) += realAddNum;
		}

		// �жϵ����Ƿ�ˢ��ʱ��
		if( GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_update ||
			GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_accumulate )
		{
			xgc_real32 lastTime = 0.0f;
			// ��Ҫˢ��ʱ�䣬ɾ���ɵĶ�ʱ���¼��������½�һ��
			Json::Value value_array = mJsonRoot[OVERLAP_TIMER_STR_HEAD];
			auto& it = mMapOverlapTimer.find( 1 );
			if( it != mMapOverlapTimer.end() )
			{
				lastTime = xgc_real32( getTimer().remaining_seconds( it->second ) );
				getTimer().remove_event( it->second );
			}
			if( FnRealCompareZeroInState( GetAttribute( attrStateOverlapSubTimeval ).toReal() ) == enMoreThan )
			{
				// ����һ���µĶ�ʱ�������Ҹ���map
				timer_h tx = getTimer().insert_event(
					bind( &XStateBase::OnOverlapSub, this, GetAttribute( attrStateCurrOverlapNum ).toUnsigned(), 1 ),
					1,
					0.0f,
					overlapTimeVal == 0 ? GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_update ?
					GetAttribute( attrStateOverlapSubTimeval ).toReal() : GetAttribute( attrStateOverlapSubTimeval ).toReal() * realAddNum + lastTime : overlapTimeVal );

				mMapOverlapTimer[1] = tx;

				GetAttribute( attrStateCurrLastTime ) = overlapTimeVal == 0 ? GetUnsignedAttr( attrStateOverlapType ) == en_overlaptype_update ?
					GetRealAttr( attrStateOverlapSubTimeval ) : GetRealAttr( attrStateOverlapSubTimeval ) * realAddNum + lastTime : overlapTimeVal;
			}
		}
		else if( GetUnsignedAttr( attrStateOverlapType ) == en_overlaptype_unupdate )
		{
			if( FnRealCompareZeroInState( GetRealAttr( attrStateOverlapSubTimeval ) ) == enMoreThan )
			{
				// ����Sub�Ķ�ʱ��
				for( xgc_uint32 one = 0; one < realAddNum; ++one )
				{
					// �趨�ӳٶ�õ�sub��tx������������GS��ʱ���õ�ʣ��ʱ�䴫��ȥ
					timer_h tx = getTimer().insert_event(
						bind( &XStateBase::OnOverlapSub, this, 1, GetUnsignedAttr( attrStateCurrOverlapTimes ) ),
						1,
						0.0f,
						one == 0 ? overlapTimeVal == 0 ? GetRealAttr( attrStateOverlapSubTimeval ) : overlapTimeVal : GetRealAttr( attrStateOverlapSubTimeval ) );

					mMapOverlapTimer[GetUnsignedAttr( attrStateCurrOverlapTimes )] = tx;
				}

				GetAttribute( attrStateCurrLastTime ) +=
					overlapTimeVal == 0 ? GetRealAttr( attrStateOverlapSubTimeval ) : overlapTimeVal + GetRealAttr( attrStateOverlapSubTimeval ) * ( realAddNum - 1 );
			}
		}
		else
		{
			SYS_ERROR( "[%d]overlapType ERROR", GetAttribute( attrStateIndex ).toInteger() );
			return;
		}

		if( bTrigger )
		{
			TriggerStateOverlapAdd( realAddNum );
		}
	}

	xgc_void XStateBase::OnTrigger()
	{
		XGC_ASSERT_MESSAGE( m_StateTimer != INVALID_TIMER_HANDLE, "δ�и�buff��" );

		// ------------------------------------------------ //
		// [5/22/2014 jianglei.kinly]
		// TODO:������bug�����������һ�ε�ʱ�򣬶�ʱ��ɾ���ˣ�����buff��û��ɾ���ˣ�Ҳ����buff��Ч��
		// TODO:�޸ķ�����attrStateCurrTriggerTimes�ĳ�Unsigned���ͣ���attrStateTriggerTimes��Unsigned���ͱȽϣ����ʱ��ɾ�����buff
		// ------------------------------------------------ //

		// �����trigger������buff����ǰtrigger��+1
		if( GetAttribute( attrStateTriggerTimes ).toInteger() != -1 )
			GetAttribute( attrStateCurrTriggerTimes ) += 1;

		xObject MyID = GetObjectID();

		// buffÿ�δ����Ĳ������¼�������֪ͨ����Ϊд��Action���棬��ΪTrigger������Ƚϸ��ӣ�ÿ��buff����һ��
		TriggerStateTrigger();

		if( false == IsValidObject( MyID ) )
			return;

		// ��trigger���ƣ���ǰtrigger�����ﵽ���ֵ���Ͱ�Overlapȫ��Sub��
		if( GetAttribute( attrStateTriggerTimes ).toInteger() != -1 &&
			GetAttribute( attrStateCurrTriggerTimes ).toInteger() >= GetAttribute( attrStateTriggerTimes ).toInteger() )
		{
			OnOverlapSub( GetAttribute( attrStateCurrOverlapNum ).toUnsigned(), 0 );
		}
	}

	xgc_void XStateBase::OnOverlapSub( xgc_uint32 subNum, xgc_uint32 CurrSubKeyID )
	{
		XGC_ASSERT_RETURN( GetParent() != INVALID_OBJECT_ID, xgc_void( 0 ) );
		XGC_ASSERT_MESSAGE( GetAttribute( attrStateCurrOverlapNum ).toUnsigned() > 0, "Buff���Ӳ����Ѽ��ٵ�0" );

		xgc_uint32 realSubNum = GetAttribute( attrStateCurrOverlapNum ).toUnsigned() > subNum ? subNum : GetAttribute( attrStateCurrOverlapNum ).toUnsigned();
		GetAttribute( attrStateCurrOverlapNum ) -= realSubNum;

		// ���overlapnum == 0 ����ɾ�����buff��
		if( GetAttribute( attrStateCurrOverlapNum ).toUnsigned() == 0 )
		{
			// ɾ��Overlap�õĶ�ʱ��
			for( auto& iter : mMapOverlapTimer )
			{
				getTimer().remove_event( iter.second );
			}
			mMapOverlapTimer.clear();
			
			GetAttribute( attrStateCurrLastTime ) = 0;
			// ����OverlapNum == 0���¼�
			TriggerStateOverlapZero( realSubNum );
		}
		else
		{
			// ֻ����������������һ�������Ŀ���
			if( GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_update )
			{
				xgc_real32 lastTime = 0.0f;
				// ��Ҫˢ��ʱ�䣬ɾ���ɵĶ�ʱ���¼��������½�һ��
				auto& it = mMapOverlapTimer.find( 1 );
				if( it != mMapOverlapTimer.end() )
				{
					lastTime = xgc_real32( getTimer().remaining_seconds( it->second ) );
					getTimer().remove_event( it->second );
				}
				// overlaptimeval == -1 ��û�н�����ʱ����
// 				else
// 				{
// 					XGC_ASSERT_MESSAGE( false, "�ݼ��Ĺ���û���ҵ�ԭ��ʱ��" );
// 				}
				if( FnRealCompareZeroInState( GetAttribute( attrStateOverlapSubTimeval ).toReal() ) == enMoreThan )
				{
					// ����һ���µĶ�ʱ�������Ҹ���map
					timer_h tx = getTimer().insert_event(
						bind( &XStateBase::OnOverlapSub, this, GetAttribute( attrStateCurrOverlapNum ).toUnsigned(), 1 ),
						1,
						0.0f,
						lastTime );

					mMapOverlapTimer[1] = tx;

					GetAttribute( attrStateCurrLastTime ) = lastTime;
				}
			}
			else
			{
				// ����ɾ����ʱ����mapɾ��Ԫ�أ�����OverlapSub���¼�
				auto& it = mMapOverlapTimer.find( CurrSubKeyID );
				if( it != mMapOverlapTimer.end() )
				{
					getTimer().remove_event( it->second );
					mMapOverlapTimer.erase( it );
					GetAttribute( attrStateCurrLastTime ) -= GetAttribute( attrStateOverlapSubTimeval ).toReal();
				}
			}
			TriggerStateOverlapSub( realSubNum );
		}
	}

	xgc_void XStateBase::OnDelete( xgc_uint32 subNum, xgc_bool isServerCancel /*= false*/ )
	{
		XGC_ASSERT_RETURN( GetParent() != INVALID_OBJECT_ID, xgc_void( 0 ) );

		if( m_StateTimer != INVALID_TIMER_HANDLE )
		{
			GetAttribute( attrStateCurrTriggerTimeval ) = getTimer().remaining_seconds( m_StateTimer );
			getTimer().remove_event( m_StateTimer );
			m_StateTimer = INVALID_TIMER_HANDLE;
		}
		if( m_StateCheckTimer != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( m_StateCheckTimer );
			m_StateCheckTimer = INVALID_TIMER_HANDLE;
		}
		TriggerStateDelete( subNum, isServerCancel );
	}

	xgc_uint32 XStateBase::GetRealLastTime()
	{
		FUNCTION_BEGIN;
		// ��ȡtrigger��ʣ��ʱ��
		xgc_uint32 lastTime01 = 0;
		if( m_StateTimer == INVALID_TIMER_HANDLE || GetIntegerAttr( attrStateTriggerTimes ) == -1 )
			lastTime01 = std::numeric_limits<xgc_uint32>::max();
		else
			lastTime01 = xgc_uint32( getTimer().remaining_all_seconds( m_StateTimer ) );
		
		// ��ȡoverlap��ʣ��ʱ��
		xgc_uint32 lastTime02 = 0;
		for( auto& iter : mMapOverlapTimer )
		{
			lastTime02 += xgc_uint32( getTimer().remaining_seconds( iter.second ) );
		}
		return lastTime02 == 0 ? lastTime01 : XGC_MIN( lastTime01, lastTime02 );
		FUNCTION_END;
		return 0;
	}
}