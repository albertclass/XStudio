#include "XHeader.h"
#include "XSkill.h"
#include "XCharactor.h"
//////////////////////////////////////////////////////////////////////////
namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// 技能属性索引
	extern "C"
	{
		CORE_API xAttrIndex	 attrSkillIndex;	   // 技能配置索引
		CORE_API xAttrIndex  attrSkillMainId;      // 技能主ID
		CORE_API xAttrIndex  attrSkillUseTime;     // 吟唱时长
		CORE_API xAttrIndex  attrSkillKeepTime;    // 持续时长
		CORE_API xAttrIndex  attrSkillCoolDown;    // 冷却时长
		CORE_API xAttrIndex  attrSkillRepeat;      // 重复次数 
		/*CORE_API xAttrIndex  attrSkillRepeatDelay; // 重复时间间隔*/

		CORE_API xAttrIndex  attrSkillCurrCoolTs;  // 当前冷却时间
		CORE_API xAttrIndex  attrSkillCurrRepeat;  // 当前重复次数（技能分段攻击使用）
	}

	IMPLEMENT_XCLASS_BEGIN( XSkill, XObject )
		IMPLEMENT_ATTRIBUTE( SkillIndex,	VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillMainId,	VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillUseTime,	VT_REAL32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillKeepTime, VT_REAL32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillCoolDown, VT_REAL32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillRepeat,	VT_U32, ATTR_FLAG_NONE, "20140912" )
		/*IMPLEMENT_ATTRIBUTE( SkillRepeatDelay, VT_REAL, ATTR_FLAG_NONE, "20140912" )*/
		IMPLEMENT_ATTRIBUTE( SkillCurrCoolTs, VT_REAL32, ATTR_FLAG_NONE, "20140912" ) // 当前剩余冷却时间
		IMPLEMENT_ATTRIBUTE( SkillCurrRepeat, VT_U32, ATTR_FLAG_NONE, "20140912" )
	IMPLEMENT_XCLASS_END();

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
	auto FnRealCompareZeroInSkill = []( xgc_real32 real )->EnRealCompareZero
	{
		return xgc_int32( real * 100 ) > 0 ? enMoreThan : xgc_int32( real * 100 ) < 0 ? enLessThan : enEquap;
	};

	XSkill::XSkill()
		: XObject()
		, mCasting( INVALID_TIMER_HANDLE )
		, mLifetime( INVALID_TIMER_HANDLE )
		, mCooldown( INVALID_TIMER_HANDLE )
		, mAttack( INVALID_TIMER_HANDLE )
	{
	}

	XSkill::~XSkill( void )
	{
		// if( IsCasting() )
		// 吟唱状态永远都要清理
		if( mCasting != INVALID_TIMER_HANDLE )
		{
			ClearCasting( false );
		}

		if( IsCooldown() )
			ClearCooldown();

		if( !IsOver() )
			ClearLifetime( false, false );
	}

	//---------------------------------------------------//
	// [12/29/2010 Albert]
	// Description:	开始使用技能 
	//---------------------------------------------------//
	xgc_bool XSkill::CastSkill()
	{
		XCharactor* pActor = ObjectCast< XCharactor >( GetParent() );
		XGC_ASSERT_RETURN( pActor, false );
		if( pActor )
		{
			return TriggerSkillBegin();
		}

		return false;
	}

	// ------------------------------------------------ //
	// [2/19/2014 jianglei.kinly]
	// Description:	攻击目标 技能调用的Action列表
	// return : 技能释放需要使用的时间
	// ------------------------------------------------ //
	xgc_void XSkill::Attack( xObject hTarget )
	{
		TriggerSkillAttackStep( hTarget );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	设置技能释放状态 
	// 设置定时器，吟唱结束后回调ResetCasting
	//---------------------------------------------------//
	xgc_void XSkill::MarkCasting()
	{
		XGC_ASSERT_MESSAGE( mCasting == INVALID_TIMER_HANDLE, "技能已经在释放过程中。" );
		// 有吟唱的，就通知客户端吟唱
		if( FnRealCompareZeroInSkill( getAttr( attrSkillUseTime ).toReal32() ) == enMoreThan )
		{
			mCasting = getTimer().insert(
				std::bind( &XSkill::EndCasting, this, _1 ),
				1,
				0.0f,
				(xgc_real32) getAttr( attrSkillUseTime ).toReal32() );
			mCastingStatus = en_casting_in;
			TriggerSkillCasting();
		}
		else
		{
			// 否则直接调吟唱结束后的逻辑
			ResetCasting();
		}
	}

	xgc_void XSkill::ClearCasting( bool bTrigger )
	{
		if( mCasting != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mCasting );
			mCasting = INVALID_TIMER_HANDLE;
		}
		// 初始化吟唱状态
		mCastingStatus = en_casting_init;

		if( bTrigger )
		{
			// 开始释放技能
			CastSkill();
			// 吟唱结束，设置技能冷却
			MarkCooldown();
			if( getAttr( attrSkillRepeat ).toInteger() > 0 )
			{
				xgc_real32 repeatDelay = GetTriggerDelay( 0 );
				XGC_ASSERT_RETURN( repeatDelay >= 0, xgc_void( 0 ) );
				// 插入技能攻击的事件，设置重复时间和次数
				mAttack = getTimer().insert(
					std::bind( &XSkill::MarkAttack, this, _1 ),
					getAttr( attrSkillRepeat ).toInteger(),
					repeatDelay,
					repeatDelay );
			}
		}
		else
			TriggerSkillEndCasting( false );  // 需要处理gate预警的东西
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	清除冷却状态 
	// res : 离下次触发还有多少秒
	//---------------------------------------------------//
	xgc_real32 XSkill::ClearCooldown( bool bTrigger, bool bSendMsg )
	{
		if( mCooldown == INVALID_TIMER_HANDLE )
		{
			return 0.0f;
		}
		xgc_real32 res = xgc_real32( getTimer().remaining_seconds( mCooldown ) );
		getTimer().remove_event( mCooldown );
		mCooldown = INVALID_TIMER_HANDLE;
		if( bTrigger )
		{
			TriggerSkillCooldown( bSendMsg );
		}
		return res;
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	清除释放状态
	//---------------------------------------------------//
	xgc_void XSkill::ClearLifetime( xgc_bool isNormalEnd, bool bTrigger )
	{
		if( bTrigger )
		{
			TriggerSkillOver( isNormalEnd );
		}

		if( mLifetime != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mLifetime );
			mLifetime = INVALID_TIMER_HANDLE;
		}
		// 这里逻辑上一起清理Attack
		if( mAttack != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mAttack );
			mAttack = INVALID_TIMER_HANDLE;
		}
		// 初始化吟唱状态
		mCastingStatus = en_casting_init;
		// 清理上次攻击留下的痕迹
		mJsonRoot = Json::Value::null;
		setValue( attrSkillCurrRepeat, 0U );
		setValue( attrSkillCurrCoolTs, 0.0f );

		XCharactor* pActor = ObjectCast<XCharactor>( GetParent() );
		XGC_ASSERT_RETURN( pActor, xgc_void( 0 ) );

		// 如果当前的技能是该技能
		if( pActor->GetCurrentSkill() == GetObjectID() )
			pActor->SetCurrentSkill( INVALID_OBJECT_ID, INVALID_OBJECT_ID, XVector3::ZERO );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	设置技能冷却状态  
	//---------------------------------------------------//
	xgc_void XSkill::MarkCooldown()
	{
		XGC_ASSERT_MESSAGE( mCooldown == INVALID_TIMER_HANDLE, "技能[%u]正在冷却。", getAttr( attrSkillIndex ).toUnsigned() );
		xgc_real32 coolts = getValue<xgc_real32>( attrSkillCoolDown );
		xgc_real32 currcollts = getValue< xgc_real32 >( attrSkillCurrCoolTs );
		XCharactor* pxCharactor = ObjectCast<XCharactor>( GetParent() );
		XGC_ASSERT_RETURN( pxCharactor, xgc_void( 0 ) );
		if( FnRealCompareZeroInSkill(coolts) == enMoreThan && !pxCharactor->GetIsIgnoreSkillCD() )
		{
			mCooldown = getTimer().insert(
				std::bind( &XSkill::ResetCooldown, this, _1 ),
				1,
				0.0f,
				FnRealCompareZeroInSkill( currcollts ) == enEquap ? coolts : currcollts );

			setValue( attrSkillCurrCoolTs, 0.0f );
		}
		MarkPublicCD();
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	重置技能释放状态 
	// 清除吟唱状态（clearCasting）, 通知 TriggerCode_SkillCasting 事件（这个事件寻找具体目标，执行Attack）
	// clearCasting 应该传入一个持续时长内时间间隔的间隔值，比如 持续5s,每1s需要新建一个TriggerCode_SkillCasting事件
	// 新建定时：技能持续时长过后ResetLefttime
	//---------------------------------------------------//
	xgc_void XSkill::ResetCasting()
	{
		// 这里面定义了attack的事件
		ClearCasting( true );

		XGC_ASSERT_MESSAGE( mLifetime == INVALID_TIMER_HANDLE, "技能已经在释放过程中。" );
		// 设置技能结束的事件
		mLifetime = getTimer().insert(
			std::bind( &XSkill::ResetLifetime, this, _1 ),
			1,
			0.0f,
			getValue< xgc_real32 >( attrSkillKeepTime );
	}

	// ------------------------------------------------ //
	// [3/11/2014 jianglei.kinly]
	// 结束聚气，置标志位，等待客户端发聚气结束的消息
	// ------------------------------------------------ //
	xgc_void XSkill::EndCasting( timer_h handle )
	{
		getTimer().remove( mCasting );
		mCasting = INVALID_TIMER_HANDLE;
		// 初始化吟唱状态
		mCastingStatus = en_casting_out;
		TriggerSkillEndCasting( true );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	重置技能冷却状态 
	//---------------------------------------------------//
	xgc_void XSkill::ResetCooldown( timer_h handle )
	{
		ClearCooldown( true );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	重置技能释放状态 
	//---------------------------------------------------//
	xgc_void XSkill::ResetLifetime( timer_h handle )
	{
		ClearLifetime( true, true );
	}

	// ------------------------------------------------ //
	// [1/24/2014 jianglei.kinly]
	// 设定技能产生效果中的状态
	// TriggerCode_SkillCasting 这个事件里面做目标选取
	// ------------------------------------------------ //
	xgc_void XSkill::MarkAttack( timer_h handle )
	{
		TriggerSkillAttack();
		auto _event = getTimer().get_event( handle );
		if( _event )
		{
			auto _delay = GetTriggerDelay( getValue< xgc_uint32 >( attrSkillCurrRepeat ) );
			if( _delay )
			{
				_event->SetInterval( _delay );
			}
		}
	}

	xgc_uint32 XSkill::GetLastCoolTs()
	{
		return xgc_uint32( getTimer().remaining_seconds( mCooldown ) );
	}

	xgc_bool XSkill::IsCastingEndFuzzy() const
	{
		if( IsCasting() )
		{
			if( xgc_int32( getTimer().remaining_seconds( mCasting ) * 1000 ) <= 500 )
				return true;
			return false;
		}
		return true;
	}

	xgc_bool XSkill::IsCooldownFuzzy() const
	{
		if( IsCooldown() )
		{
			if( xgc_int32( getTimer().remaining_seconds( mCooldown ) * 1000 ) <= 300 )
				return false;
			return true;
		}
		return false;
	}
}
