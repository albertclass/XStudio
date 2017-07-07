#include "StdAfx.h"
#include "XAction.h"
#include "XSkillBase.h"
#include "XCharactor.h"
#include "json/json.h"
//////////////////////////////////////////////////////////////////////////
namespace XGC
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

	BEGIN_IMPLEMENT_XCLASS( XSkillBase, XObject, TypeXSkillBase )
		IMPLEMENT_ATTRIBUTE( SkillIndex, VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillMainId, VT_U32, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillUseTime, VT_REAL, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillKeepTime, VT_REAL, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillCoolDown, VT_REAL, ATTR_FLAG_NONE, "20140912" )
		IMPLEMENT_ATTRIBUTE( SkillRepeat, VT_U32, ATTR_FLAG_NONE, "20140912" )
		/*IMPLEMENT_ATTRIBUTE( SkillRepeatDelay, VT_REAL, ATTR_FLAG_NONE, "20140912" )*/
		IMPLEMENT_ATTRIBUTE( SkillCurrCoolTs, VT_REAL, ATTR_FLAG_NONE, "20140912" ) // ��ǰʣ����ȴʱ��
		IMPLEMENT_ATTRIBUTE( SkillCurrRepeat, VT_U32, ATTR_FLAG_NONE, "20140912" )
	END_IMPLEMENT_XCLASS();

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

	XSkillBase::XSkillBase()
		: XObject()
		, mCasting( INVALID_TIMER_HANDLE )
		, mLifetime( INVALID_TIMER_HANDLE )
		, mCooldown( INVALID_TIMER_HANDLE )
		, mAttack( INVALID_TIMER_HANDLE )
	{
	}

	XSkillBase::~XSkillBase( void )
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
	xgc_bool XSkillBase::CastSkill()
	{
		XCharactor* pActor = static_cast<XCharactor*>( GetXObject( GetParent(), TypeXCharactor ) );
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
	xgc_void XSkillBase::Attack( xObject hTarget )
	{
		TriggerSkillAttackStep( hTarget );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü����ͷ�״̬ 
	// ���ö�ʱ��������������ص�ResetCasting
	//---------------------------------------------------//
	xgc_void XSkillBase::MarkCasting()
	{
		XGC_ASSERT_MESSAGE( mCasting == INVALID_TIMER_HANDLE, "�����Ѿ����ͷŹ����С�" );
		// �������ģ���֪ͨ�ͻ�������
		if( FnRealCompareZeroInSkill( GetAttribute( attrSkillUseTime ).toReal() ) == enMoreThan )
		{
			mCasting = getTimer().insert_event(
				bind( &XSkillBase::EndCasting, this, _1 ),
				1,
				0.0f,
				(xgc_real32) GetAttribute( attrSkillUseTime ).toReal() );
			mCastingStatus = en_casting_in;
			TriggerSkillCasting();
		}
		else
		{
			// ����ֱ�ӵ�������������߼�
			ResetCasting();
		}
	}

	xgc_void XSkillBase::ClearCasting( bool bTrigger )
	{
		if( mCasting != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( mCasting );
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
			if( GetAttribute( attrSkillRepeat ).toInteger() > 0 )
			{
				xgc_real32 repeatDelay = GetTriggerDelay( 0 );
				XGC_ASSERT_RETURN( repeatDelay >= 0, xgc_void( 0 ) );
				// ���뼼�ܹ������¼��������ظ�ʱ��ʹ���
				mAttack = getTimer().insert_event(
					bind( &XSkillBase::MarkAttack, this, _1 ),
					GetAttribute( attrSkillRepeat ).toInteger(),
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
	xgc_real32 XSkillBase::ClearCooldown( bool bTrigger, bool bSendMsg )
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
	xgc_void XSkillBase::ClearLifetime( xgc_bool isNormalEnd, bool bTrigger )
	{
		if( bTrigger )
		{
			TriggerSkillOver( isNormalEnd );
		}

		if( mLifetime != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( mLifetime );
			mLifetime = INVALID_TIMER_HANDLE;
		}
		// �����߼���һ������Attack
		if( mAttack != INVALID_TIMER_HANDLE )
		{
			getTimer().remove_event( mAttack );
			mAttack = INVALID_TIMER_HANDLE;
		}
		// ��ʼ������״̬
		mCastingStatus = en_casting_init;
		// �����ϴι������µĺۼ�
		mJsonRoot = Json::Value::null;
		SetUnsignedAttr( attrSkillCurrRepeat, 0 );
		SetRealAttr( attrSkillCurrCoolTs, 0.0f );

		XCharactor* pActor = static_cast<XCharactor*>( GetXObject( GetParent(), TypeXCharactor ) );
		XGC_ASSERT_RETURN( pActor, xgc_void( 0 ) );

		// �����ǰ�ļ����Ǹü���
		if( pActor->GetCurrentSkill() == GetObjectID() )
			pActor->SetCurrentSkill( INVALID_OBJECT_ID, INVALID_OBJECT_ID, XVector3::ZERO );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü�����ȴ״̬  
	//---------------------------------------------------//
	xgc_void XSkillBase::MarkCooldown()
	{
		XGC_ASSERT_MESSAGE( mCooldown == INVALID_TIMER_HANDLE, "����[%u]������ȴ��", GetAttribute( attrSkillIndex ).toUnsigned() );
		xgc_real32 coolts = (xgc_real32) GetAttribute( attrSkillCoolDown ).toReal();
		xgc_real32 currcollts = (xgc_real32) GetAttribute( attrSkillCurrCoolTs ).toReal();
		XCharactor* pxCharactor = ObjectCast<XCharactor>( GetParent() );
		XGC_ASSERT_RETURN( pxCharactor, xgc_void( 0 ) );
		if( FnRealCompareZeroInSkill(coolts) == enMoreThan && !pxCharactor->GetIsIgnoreSkillCD() )
		{
			mCooldown = getTimer().insert_event( bind( &XSkillBase::ResetCooldown, this, _1 ),
				1,
				0.0f,
				FnRealCompareZeroInSkill( currcollts ) == enEquap ? coolts : currcollts );
			GetAttribute( attrSkillCurrCoolTs ) = 0.0f;
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
	xgc_void XSkillBase::ResetCasting()
	{
		// �����涨����attack���¼�
		ClearCasting( true );

		XGC_ASSERT_MESSAGE( mLifetime == INVALID_TIMER_HANDLE, "�����Ѿ����ͷŹ����С�" );
		// ���ü��ܽ������¼�
		mLifetime = getTimer().insert_event(
			bind( &XSkillBase::ResetLifetime, this, _1 ),
			1,
			0.0f,
			(xgc_real32) GetAttribute( attrSkillKeepTime ).toReal() );
	}

	// ------------------------------------------------ //
	// [3/11/2014 jianglei.kinly]
	// �����������ñ�־λ���ȴ��ͻ��˷�������������Ϣ
	// ------------------------------------------------ //
	xgc_void XSkillBase::EndCasting( timer_h handle )
	{
		getTimer().remove_event( mCasting );
		mCasting = INVALID_TIMER_HANDLE;
		// ��ʼ������״̬
		mCastingStatus = en_casting_out;
		TriggerSkillEndCasting( true );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü�����ȴ״̬ 
	//---------------------------------------------------//
	xgc_void XSkillBase::ResetCooldown( timer_h handle )
	{
		ClearCooldown( true );
	}

	//---------------------------------------------------//
	// [12/6/2010 Albert]
	// Description:	���ü����ͷ�״̬ 
	//---------------------------------------------------//
	xgc_void XSkillBase::ResetLifetime( timer_h handle )
	{
		ClearLifetime( true, true );
	}

	// ------------------------------------------------ //
	// [1/24/2014 jianglei.kinly]
	// �趨���ܲ���Ч���е�״̬
	// TriggerCode_SkillCasting ����¼�������Ŀ��ѡȡ
	// ------------------------------------------------ //
	xgc_void XSkillBase::MarkAttack( timer_h handle )
	{
		TriggerSkillAttack();
		auto _event = getTimer().get_event( handle );
		if( _event )
		{
			auto _delay = GetTriggerDelay( GetUnsignedAttr( attrSkillCurrRepeat ) );
			if( _delay )
			{
				_event->SetInterval( _delay );
			}
		}
	}

	xgc_uint32 XSkillBase::GetLastCoolTs()
	{
		return xgc_uint32( getTimer().remaining_seconds( mCooldown ) );
	}

	xgc_bool XSkillBase::IsCastingEndFuzzy() const
	{
		if( IsCasting() )
		{
			if( xgc_int32( getTimer().remaining_seconds( mCasting ) * 1000 ) <= 500 )
				return true;
			return false;
		}
		return true;
	}

	xgc_bool XSkillBase::IsCooldownFuzzy() const
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
