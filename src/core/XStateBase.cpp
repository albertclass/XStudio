/*******************************************************************/
//! \file XStateBase.cpp
//! \brief 状态基类
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
	// buff属性索引
	extern "C"
	{
		CORE_API xAttrIndex attrStateIndex;             ///< buff索引
		CORE_API xAttrIndex attrStateBuffID;            ///< 主ID
		CORE_API xAttrIndex attrStateType;              ///< 分类
		CORE_API xAttrIndex attrStateIsRing;            ///< 是否是光环类的buff
		CORE_API xAttrIndex attrStateTriggerTimeval;    ///< 触发时间间隔
		CORE_API xAttrIndex attrStateTriggerTimes;      ///< 触发次数
		CORE_API xAttrIndex attrStateOverlap;           ///< 叠加层数
		CORE_API xAttrIndex attrStateOverlapSubTimeval; ///< 可叠加类buff的持续时间
		CORE_API xAttrIndex attrStateOverlapType;       ///< buff叠加时是否刷新时间

		// 跳GS用的
		CORE_API xAttrIndex attrStateCurrTriggerTimeval; ///< 距离下次触发的时间间隔
		CORE_API xAttrIndex attrStateCurrTriggerTimes;   ///< 当前触发次数
		CORE_API xAttrIndex attrStateCurrOverlapNum;     ///< 当前叠加层数
		CORE_API xAttrIndex attrStateCurrLastTime;       ///< buff剩余时长，用于发给客户端
		CORE_API xAttrIndex attrStateCurrOverlapTimes;   ///< 当前叠加次数（用来组合定时器Key标示）
	}

	BEGIN_IMPLEMENT_XCLASS( XStateBase, XObject, TypeXStateBase )
		IMPLEMENT_ATTRIBUTE( StateIndex, VT_U32, ATTR_FLAG_NONE, "20140912" )       ///< buff索引
		IMPLEMENT_ATTRIBUTE( StateBuffID, VT_U32, ATTR_FLAG_NONE, "20140912" )       ///< 主ID
		IMPLEMENT_ATTRIBUTE( StateType, VT_U32, ATTR_FLAG_NONE, "20140912" )        ///< 分类
		IMPLEMENT_ATTRIBUTE( StateIsRing, VT_BOOL, ATTR_FLAG_NONE, "20140912" )     ///< 是否是光环类的
		IMPLEMENT_ATTRIBUTE( StateTriggerTimeval, VT_REAL, ATTR_FLAG_NONE, "20140912" )    ///< 触发时间间隔
		IMPLEMENT_ATTRIBUTE( StateTriggerTimes, VT_I32, ATTR_FLAG_NONE, "20140912" )      ///< 触发次数
		IMPLEMENT_ATTRIBUTE( StateOverlap, VT_U32, ATTR_FLAG_NONE, "20140912" )           ///< 叠加层数
		IMPLEMENT_ATTRIBUTE( StateOverlapSubTimeval, VT_REAL, ATTR_FLAG_NONE, "20140912" ) ///< 可叠加类buff的持续时间
		IMPLEMENT_ATTRIBUTE( StateOverlapType, VT_U32, ATTR_FLAG_NONE, "20140912" )       ///< buff叠加时是否刷新时间
		IMPLEMENT_ATTRIBUTE( StateCurrTriggerTimeval, VT_REAL, ATTR_FLAG_NONE, "20140912" ) ///< 距离下次触发的时间间隔
		IMPLEMENT_ATTRIBUTE( StateCurrTriggerTimes, VT_I32, ATTR_FLAG_NONE, "20140912" )   ///< 当前触发次数
		IMPLEMENT_ATTRIBUTE( StateCurrOverlapNum, VT_U32, ATTR_FLAG_NONE, "20140912" )     ///< 当前叠加层数
		IMPLEMENT_ATTRIBUTE( StateCurrLastTime, VT_U32, ATTR_FLAG_NONE, "20140912" )     ///< buff剩余时长，用于发给客户端
		IMPLEMENT_ATTRIBUTE( StateCurrOverlapTimes, VT_U32, ATTR_FLAG_NONE, "20140912" )     ///< 当前叠加次数（用来组合定时器Key标示）
	END_IMPLEMENT_XCLASS();

	XStateBase::XStateBase()
		: XObject()
		, m_StateTimer( INVALID_TIMER_HANDLE )
		, m_StateCheckTimer( INVALID_TIMER_HANDLE )
	{
	}

	enum EnRealCompareZero
	{
		// 大于
		enMoreThan,
		// 小于
		enLessThan,
		// 等于
		enEquap,
	};

	///
	/// 比较浮点型和0的比较
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
		// 删掉Overlap用的定时器
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
		XGC_ASSERT_MESSAGE( m_StateTimer == INVALID_TIMER_HANDLE, "已有该buff。" );

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

		// 真正增加的层数
		XVariant::Unsigned realAddNum = addNum == 0 ? 1 : addNum;

		// OverlapNum达到最大值得时候不再增加
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
			// 用realrealAddNum做添加
			if( GetAttribute( attrStateCurrOverlapNum ).toUnsigned() + addNum > GetAttribute( attrStateOverlap ).toUnsigned() )
				realAddNum = GetAttribute( attrStateOverlap ).toUnsigned() - GetAttribute( attrStateCurrOverlapNum ).toUnsigned();
			GetAttribute( attrStateCurrOverlapNum ) += realAddNum;
			GetAttribute( attrStateCurrOverlapTimes ) += realAddNum;
		}

		// 判断叠加是否刷新时间
		if( GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_update ||
			GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_accumulate )
		{
			xgc_real32 lastTime = 0.0f;
			// 需要刷新时间，删掉旧的定时器事件，重新新建一个
			Json::Value value_array = mJsonRoot[OVERLAP_TIMER_STR_HEAD];
			auto& it = mMapOverlapTimer.find( 1 );
			if( it != mMapOverlapTimer.end() )
			{
				lastTime = xgc_real32( getTimer().remaining_seconds( it->second ) );
				getTimer().remove_event( it->second );
			}
			if( FnRealCompareZeroInState( GetAttribute( attrStateOverlapSubTimeval ).toReal() ) == enMoreThan )
			{
				// 插入一个新的定时器，并且更新map
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
				// 设置Sub的定时器
				for( xgc_uint32 one = 0; one < realAddNum; ++one )
				{
					// 设定延迟多久调sub，tx保存下来，跳GS的时候拿到剩余时间传出去
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
		XGC_ASSERT_MESSAGE( m_StateTimer != INVALID_TIMER_HANDLE, "未有该buff。" );

		// ------------------------------------------------ //
		// [5/22/2014 jianglei.kinly]
		// TODO:这里有bug，触发的最后一次的时候，定时器删除了，但是buff是没有删掉了，也就是buff无效了
		// TODO:修改方案：attrStateCurrTriggerTimes改成Unsigned类型，与attrStateTriggerTimes的Unsigned类型比较，最大时候删除这个buff
		// ------------------------------------------------ //

		// 有最大trigger次数的buff，当前trigger才+1
		if( GetAttribute( attrStateTriggerTimes ).toInteger() != -1 )
			GetAttribute( attrStateCurrTriggerTimes ) += 1;

		xObject MyID = GetObjectID();

		// buff每次触发的不触发事件，具体通知、行为写在Action里面，因为Trigger的情况比较复杂，每个buff都不一样
		TriggerStateTrigger();

		if( false == IsValidObject( MyID ) )
			return;

		// 有trigger限制，当前trigger次数达到最大值，就把Overlap全部Sub掉
		if( GetAttribute( attrStateTriggerTimes ).toInteger() != -1 &&
			GetAttribute( attrStateCurrTriggerTimes ).toInteger() >= GetAttribute( attrStateTriggerTimes ).toInteger() )
		{
			OnOverlapSub( GetAttribute( attrStateCurrOverlapNum ).toUnsigned(), 0 );
		}
	}

	xgc_void XStateBase::OnOverlapSub( xgc_uint32 subNum, xgc_uint32 CurrSubKeyID )
	{
		XGC_ASSERT_RETURN( GetParent() != INVALID_OBJECT_ID, xgc_void( 0 ) );
		XGC_ASSERT_MESSAGE( GetAttribute( attrStateCurrOverlapNum ).toUnsigned() > 0, "Buff叠加层数已减少到0" );

		xgc_uint32 realSubNum = GetAttribute( attrStateCurrOverlapNum ).toUnsigned() > subNum ? subNum : GetAttribute( attrStateCurrOverlapNum ).toUnsigned();
		GetAttribute( attrStateCurrOverlapNum ) -= realSubNum;

		// 如果overlapnum == 0 可以删掉这个buff了
		if( GetAttribute( attrStateCurrOverlapNum ).toUnsigned() == 0 )
		{
			// 删掉Overlap用的定时器
			for( auto& iter : mMapOverlapTimer )
			{
				getTimer().remove_event( iter.second );
			}
			mMapOverlapTimer.clear();
			
			GetAttribute( attrStateCurrLastTime ) = 0;
			// 发送OverlapNum == 0的事件
			TriggerStateOverlapZero( realSubNum );
		}
		else
		{
			// 只有这种情况会有清除一定层数的可能
			if( GetAttribute( attrStateOverlapType ).toUnsigned() == en_overlaptype_update )
			{
				xgc_real32 lastTime = 0.0f;
				// 需要刷新时间，删掉旧的定时器事件，重新新建一个
				auto& it = mMapOverlapTimer.find( 1 );
				if( it != mMapOverlapTimer.end() )
				{
					lastTime = xgc_real32( getTimer().remaining_seconds( it->second ) );
					getTimer().remove_event( it->second );
				}
				// overlaptimeval == -1 是没有建立定时器的
// 				else
// 				{
// 					XGC_ASSERT_MESSAGE( false, "递减的过程没有找到原定时器" );
// 				}
				if( FnRealCompareZeroInState( GetAttribute( attrStateOverlapSubTimeval ).toReal() ) == enMoreThan )
				{
					// 插入一个新的定时器，并且更新map
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
				// 否则删掉定时器，map删掉元素，发送OverlapSub的事件
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
		// 获取trigger的剩余时间
		xgc_uint32 lastTime01 = 0;
		if( m_StateTimer == INVALID_TIMER_HANDLE || GetIntegerAttr( attrStateTriggerTimes ) == -1 )
			lastTime01 = std::numeric_limits<xgc_uint32>::max();
		else
			lastTime01 = xgc_uint32( getTimer().remaining_all_seconds( m_StateTimer ) );
		
		// 获取overlap的剩余时间
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