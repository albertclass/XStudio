#include "XHeader.h"
#include "XSkill.h"
#include "XCharactor.h"
//////////////////////////////////////////////////////////////////////////
namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// ������������
	extern "C"
	{
		CORE_API xAttrIndex	 attrSkillIndex;	   // ������������
		CORE_API xAttrIndex  attrSkillMainId;      // ������ID
		CORE_API xAttrIndex  attrSkillUseTime;     // ����ʱ��
		CORE_API xAttrIndex  attrSkillKeepTime;    // ����ʱ��
		CORE_API xAttrIndex  attrSkillCoolDown;    // ��ȴʱ��
		CORE_API xAttrIndex  attrSkillRepeat;      // �ظ����� 
		/*CORE_API xAttrIndex  attrSkillRepeatDelay; // �ظ�ʱ����*/

		CORE_API xAttrIndex  attrSkillCurrCoolTs;  // ��ǰ��ȴʱ��
		CORE_API xAttrIndex  attrSkillCurrRepeat;  // ��ǰ�ظ����������ֶܷι���ʹ�ã�
	}

	IMPLEMENT_XCLASS_BEGIN( XSkill, XObject )
		IMPLEMENT_ATTRIBUTE( SkillIndex,	VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillMainId,	VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillUseTime,	VT_REAL32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillKeepTime, VT_REAL32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillCoolDown, VT_REAL32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillRepeat,	VT_U32, ATTR_FLAG_NONE, "20140912" )
		/*IMPLEMENT_ATTRIBUTE( SkillRepeatDelay, VT_REAL, ATTR_FLAG_NONE, "20140912" )*/
		IMPLEMENT_ATTRIBUTE( SkillCurrCoolTs, VT_REAL32, ATTR_FLAG_NONE, "20140912" ) // ��ǰʣ����ȴʱ��
		IMPLEMENT_ATTRIBUTE( SkillCurrRepeat, VT_U32, ATTR_FLAG_NONE, "20140912" )
	IMPLEMENT_XCLASS_END();

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
		// ����״̬��Զ��Ҫ����
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
	// Description:	��ʼʹ�ü��� 
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
	// Description:	����Ŀ�� ���ܵ��õ�Action�б�
	// return : �����ͷ���Ҫʹ�õ�ʱ��
	// ------------------------------------------------ //
	xgc_void XSkill::Attack( xObject hTarget )
	{
		TriggerSkillAttackStep( hTarget );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü����ͷ�״̬ 
	// ���ö�ʱ��������������ص�ResetCasting
	//---------------------------------------------------//
	xgc_void XSkill::MarkCasting()
	{
		XGC_ASSERT_MESSAGE( mCasting == INVALID_TIMER_HANDLE, "�����Ѿ����ͷŹ����С�" );
		// �������ģ���֪ͨ�ͻ�������
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
			// ����ֱ�ӵ�������������߼�
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
		// ��ʼ������״̬
		mCastingStatus = en_casting_init;

		if( bTrigger )
		{
			// ��ʼ�ͷż���
			CastSkill();
			// �������������ü�����ȴ
			MarkCooldown();
			if( getAttr( attrSkillRepeat ).toInteger() > 0 )
			{
				xgc_real32 repeatDelay = GetTriggerDelay( 0 );
				XGC_ASSERT_RETURN( repeatDelay >= 0, xgc_void( 0 ) );
				// ���뼼�ܹ������¼��������ظ�ʱ��ʹ���
				mAttack = getTimer().insert(
					std::bind( &XSkill::MarkAttack, this, _1 ),
					getAttr( attrSkillRepeat ).toInteger(),
					repeatDelay,
					repeatDelay );
			}
		}
		else
			TriggerSkillEndCasting( false );  // ��Ҫ����gateԤ���Ķ���
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	�����ȴ״̬ 
	// res : ���´δ������ж�����
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
	// Description:	����ͷ�״̬
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
		// �����߼���һ������Attack
		if( mAttack != INVALID_TIMER_HANDLE )
		{
			getTimer().remove( mAttack );
			mAttack = INVALID_TIMER_HANDLE;
		}
		// ��ʼ������״̬
		mCastingStatus = en_casting_init;
		// �����ϴι������µĺۼ�
		mJsonRoot = Json::Value::null;
		setValue( attrSkillCurrRepeat, 0U );
		setValue( attrSkillCurrCoolTs, 0.0f );

		XCharactor* pActor = ObjectCast<XCharactor>( GetParent() );
		XGC_ASSERT_RETURN( pActor, xgc_void( 0 ) );

		// �����ǰ�ļ����Ǹü���
		if( pActor->GetCurrentSkill() == GetObjectID() )
			pActor->SetCurrentSkill( INVALID_OBJECT_ID, INVALID_OBJECT_ID, XVector3::ZERO );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü�����ȴ״̬  
	//---------------------------------------------------//
	xgc_void XSkill::MarkCooldown()
	{
		XGC_ASSERT_MESSAGE( mCooldown == INVALID_TIMER_HANDLE, "����[%u]������ȴ��", getAttr( attrSkillIndex ).toUnsigned() );
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
	// Description:	���ü����ͷ�״̬ 
	// �������״̬��clearCasting��, ֪ͨ TriggerCode_SkillCasting �¼�������¼�Ѱ�Ҿ���Ŀ�ִ꣬��Attack��
	// clearCasting Ӧ�ô���һ������ʱ����ʱ�����ļ��ֵ������ ����5s,ÿ1s��Ҫ�½�һ��TriggerCode_SkillCasting�¼�
	// �½���ʱ�����ܳ���ʱ������ResetLefttime
	//---------------------------------------------------//
	xgc_void XSkill::ResetCasting()
	{
		// �����涨����attack���¼�
		ClearCasting( true );

		XGC_ASSERT_MESSAGE( mLifetime == INVALID_TIMER_HANDLE, "�����Ѿ����ͷŹ����С�" );
		// ���ü��ܽ������¼�
		mLifetime = getTimer().insert(
			std::bind( &XSkill::ResetLifetime, this, _1 ),
			1,
			0.0f,
			getValue< xgc_real32 >( attrSkillKeepTime );
	}

	// ------------------------------------------------ //
	// [3/11/2014 jianglei.kinly]
	// �����������ñ�־λ���ȴ��ͻ��˷�������������Ϣ
	// ------------------------------------------------ //
	xgc_void XSkill::EndCasting( timer_h handle )
	{
		getTimer().remove( mCasting );
		mCasting = INVALID_TIMER_HANDLE;
		// ��ʼ������״̬
		mCastingStatus = en_casting_out;
		TriggerSkillEndCasting( true );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü�����ȴ״̬ 
	//---------------------------------------------------//
	xgc_void XSkill::ResetCooldown( timer_h handle )
	{
		ClearCooldown( true );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü����ͷ�״̬ 
	//---------------------------------------------------//
	xgc_void XSkill::ResetLifetime( timer_h handle )
	{
		ClearLifetime( true, true );
	}

	// ------------------------------------------------ //
	// [1/24/2014 jianglei.kinly]
	// �趨���ܲ���Ч���е�״̬
	// TriggerCode_SkillCasting ����¼�������Ŀ��ѡȡ
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
