#include "XHeader.h"
#include "XQuest.h"
#include "XActor.h"
#include "XGoods.h"

namespace xgc
{
	xAttrIndex XQuestObjective::Index;			// 配置ID
	xAttrIndex XQuestObjective::TargetName;		// 对象名字
	xAttrIndex XQuestObjective::Serial;			// 序号
	xAttrIndex XQuestObjective::Count;			// 当前计数
	xAttrIndex XQuestObjective::FinishCount;	// 完成计数

	IMPLEMENT_XCLASS_BEGIN( XQuestObjective, XObject )
		IMPLEMENT_ATTRIBUTE( Index, VT_U32, ATTR_FLAG_SAVE )
		IMPLEMENT_ATTRIBUTE( TargetName, VT_STRING, ATTR_FLAG_SAVE )
		IMPLEMENT_ATTRIBUTE( Serial, VT_U32, ATTR_FLAG_SAVE )
		IMPLEMENT_ATTRIBUTE( Count, VT_U32, ATTR_FLAG_SAVE )
		IMPLEMENT_ATTRIBUTE( FinishCount, VT_U32, ATTR_FLAG_SAVE )
	IMPLEMENT_XCLASS_END();

	XQuestObjective::XQuestObjective()
	{

	}

	XQuestObjective::~XQuestObjective(void)
	{

	}

	bool XQuestObjective::Increment()
	{
		incValue( XQuestObjective::Count, 1 );
		
		if( getValue< xgc_uint32 >( Count ) >= getValue< xgc_uint32 >( FinishCount ) )
		{
			XQuest *pQuest = ObjectCast< XQuest >( GetParent() );
			if( pQuest )
				pQuest->ObjectiveNotify( getValue< xgc_uint32 >( Serial ) );

			return true;
		}

		return false;
	}

	xgc_void XQuestObjective::OnKill( xObject hKiller, xObject hDead )
	{
		XActor *pActor = ObjectCast< XActor >( hDead );
		if( pActor )
		{
			xgc_lpcstr name = pActor->getString( XGameObject::Alias );
			XGC_ASSERT_POINTER( name );
			if( name && getString( TargetName ) == name && Increment() )
			{
				XActor *pKiller = ObjectCast< XActor >( hKiller );

				if( pKiller )
				{
					// 任务完成则将触发器摘除
					// ThisTriggerMgr()->DismissTrigger( hKiller, 0, this, &XQuestObjective::OnKill );
					// pKiller->DismissTrigger( hDead, 0 );
				}
			}
		}
	}

	xgc_void XQuestObjective::OnItem( xObject hSource, xgc_uintptr lParam )
	{
		XGoods *pGoods = ObjectCast< XGoods >( hSource );
		if( pGoods )
		{
			xgc_lpcstr pName = pGoods->getString( XGoods::Alias );
			XGC_ASSERT_POINTER( pName );
			if( pName && getString( TargetName ) == pName && Increment() )
			{
				XActor *pPicker = ObjectCast< XActor >( pGoods->GetParent() );

				if( pPicker )
				{
					// 任务完成则将触发器摘除
					// ThisTriggerMgr()->DismissTrigger( hKiller, 0, this, &XQuestObjective::OnKill );
					// pKiller->DismissTrigger( hDead, 0 );
				}
			}
		}
	}

	xgc_void XQuestObjective::OnTalk( xObject hSource, xgc_uintptr lParam )
	{

	}
	//////////////////////////////////////////////////////////////////////////
	// 任务
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_XCLASS_BEGIN( XQuest, XObject )
	IMPLEMENT_XCLASS_END();

	XQuest::XQuest(void)
		: m_FinishMask( 0 )
		, m_Mask( 0 )
	{
	}

	XQuest::~XQuest(void)
	{
	}

	xgc_void XQuest::OnAddChild( XObject* pChild, xgc_lpcvoid lpContext )
	{
		xgc_size nChildCount = GetChildCount();
		if( nChildCount >= 16 )
		{
			SYS_WARNING( "任务目标已经超过数量限制. %I", nChildCount );
			return;
		}

		XGC_ASSERT( pChild && pChild->IsInheritFrom( &XQuestObjective::GetThisClass() ) );
		XQuestObjective* pObjective = ObjectCast< XQuestObjective >( pChild->GetObjectID() );

		pObjective->setValue( XQuestObjective::Serial, 1 << nChildCount );
		m_FinishMask |= pObjective->getValue< xgc_uint32 >( XQuestObjective::Serial );
	}

	xgc_void XQuest::ObjectiveNotify( xgc_uint16 nSerial )
	{
		m_Mask |= nSerial;
		if( m_Mask == m_FinishMask )
			OnQuestFinished();
	}
}
