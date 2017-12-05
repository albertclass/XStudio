#include "XHeader.h"
#include "XQuest.h"
#include "XActor.h"

namespace xgc
{
	IMPLEMENT_XCLASS_BEGIN( XQuestObjective, XObject )
	IMPLEMENT_XCLASS_END()

	XQuestObjective::XQuestObjective( xgc::string strName, xgc_uint16 nFinishCount )
		: XObject()
		, m_strName( strName )
		, m_nFinishCount( nFinishCount )
		, m_nCount( 0 )
		, m_nSerial( 0 )
	{

	}

	XQuestObjective::~XQuestObjective(void)
	{

	}

	bool XQuestObjective::Increment()
	{
		++m_nCount;
		if( m_nCount == m_nFinishCount )
		{
			XQuest *pQuest = ObjectCast< XQuest >( GetParent() );
			if( pQuest )
				pQuest->ObjectiveNotify( m_nSerial );

			return true;
		}
		return false;
	}

	xgc_int32 XQuestObjective::OnKill( xObject hKiller, xObject hDead )
	{
		XActor *pActor = ObjectCast< XActor >( hDead );
		if( pActor )
		{
			xgc_lpcstr name = pActor->getString( attrObjectName );
			XGC_ASSERT_POINTER( name );
			if( name && m_strName == name && Increment() )
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
		return false;
	}

	xgc_int32 XQuestObjective::OnItem( xObject hSource, xgc_uintptr lParam )
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// 任务
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_XCLASS_BEGIN( XQuest, XObject )
	IMPLEMENT_XCLASS_END()

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
		xgc_size nChildCount = GetChildrenCount();
		if( nChildCount >= 16 )
		{
			SYS_WARNING( "任务目标已经超过数量限制. %I", nChildCount );
			return;
		}

		XGC_ASSERT( pChild && pChild->IsInheritFrom( &XQuestObjective::GetThisClass() ) );
		XQuestObjective* pObjective = ObjectCast< XQuestObjective >( pChild->GetObjectID() );

		pObjective->setSerial(1<<nChildCount);
		m_FinishMask |= pObjective->getSerial();
	}

	xgc_void XQuest::ObjectiveNotify( xgc_uint16 nSerial )
	{
		m_Mask |= nSerial;
		if( m_Mask == m_FinishMask )
			OnQuestFinished();
	}
}
