#include "StdAfx.h"
#include "XQuest.h"
#include "XCharactor.h"

namespace XGC
{
	BEGIN_IMPLEMENT_XCLASS( XQuestObjective, XObject, TypeXQuestObjective )
	END_IMPLEMENT_XCLASS()

	XQuestObjective::XQuestObjective( xgc_string strName, xgc_uint16 nFinishCount )
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
			XQuest *pQuest = static_cast< XQuest* >( GetXObject( GetParent() ) );
			if( pQuest )
			{
				pQuest->ObjectiveNotify( m_nSerial );
			}
			return true;
		}
		return false;
	}

	xgc_int32 XQuestObjective::OnKill( xObject hKiller, xObject hDead )
	{
		XCharactor *pActor = static_cast< XCharactor* >( GetXObject( hDead, TypeXCharactor ) );
		if( pActor )
		{
			xgc_lpcstr name = pActor->GetStrAttr( attrObjectName );
			XGC_ASSERT_POINTER( name );
			if( name && m_strName == name && Increment() )
			{
				XCharactor *pKiller = static_cast< XCharactor* >( GetXObject( hKiller, TypeXCharactor ) );

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
	BEGIN_IMPLEMENT_XCLASS( XQuest, XObject, TypeXQuest )
	END_IMPLEMENT_XCLASS()

	XQuest::XQuest(void)
		: XObject()
		, m_FinishMask( 0 )
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

		XGC_ASSERT( pChild && pChild->IsInheritFrom( TypeXQuestObjective ) );
		XQuestObjective* pObjective = static_cast< XQuestObjective* >( pChild );
		pObjective->m_nSerial = (1<<nChildCount);
		m_FinishMask |= pObjective->m_nSerial;
	}

	xgc_void XQuest::ObjectiveNotify( xgc_uint16 nSerial )
	{
		m_Mask |= nSerial;
		if( m_Mask == m_FinishMask )
			OnQuestFinished();
	}
}
