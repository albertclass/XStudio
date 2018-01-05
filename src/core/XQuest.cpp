#include "XHeader.h"
#include "XQuest.h"
#include "XActor.h"
#include "XGoods.h"
#include "XBag.h"

namespace xgc
{
	xAttrIndex XQuestObjective::Index;		///< 配置ID
	xAttrIndex XQuestObjective::TargetName;	///< 对象名字
	xAttrIndex XQuestObjective::Count;		///< 当前计数
	xAttrIndex XQuestObjective::Total;		///< 完成计数
	xAttrIndex XQuestObjective::Finished;	///< 完成标志

	IMPLEMENT_XCLASS_BEGIN( XQuestObjective, XObject )
		IMPLEMENT_ATTRIBUTE( Index, VT_U32, ATTR_FLAG_SAVE, "20171227" )
		IMPLEMENT_ATTRIBUTE( TargetName, VT_STRING, ATTR_FLAG_SAVE, "20171227" )
		IMPLEMENT_ATTRIBUTE( Count, VT_U32, ATTR_FLAG_SAVE, "20171227" )
		IMPLEMENT_ATTRIBUTE( Total, VT_U32, ATTR_FLAG_SAVE, "20171227" )
		IMPLEMENT_ATTRIBUTE( Finished, VT_BOOL, ATTR_FLAG_SAVE, "20171227" )
	IMPLEMENT_XCLASS_END();

	REGIST_ATTR_LISTENER( XQuestObjective, Count, OnValueChanged );

	XQuestObjective::XQuestObjective()
	{
	}

	XQuestObjective::~XQuestObjective(void)
	{
	}

	xgc_void XQuestObjective::incCount( xgc_long nInc )
	{
		if( false == getValue< xgc_bool >( Finished ) )
		{
			incValue( XQuestObjective::Count, nInc );

			if( getValue< xgc_uint32 >( Count ) >= getValue< xgc_uint32 >( Total ) )
			{
				setValue( Finished, true );
				EmmitEvent( evt_objective_finished, eEventTrans::toParent );
			}
		}
	}

	xgc_void XQuestObjective::onKill( XObjectEvent &evt )
	{
		auto pSender = ObjectCast< XActor >( evt.sender );

		if( pSender && evt.id == evt_actor_killed )
		{
			xgc_lpcstr name = pSender->getString( XGameObject::Alias );
			XGC_ASSERT_POINTER( name );
			if( name && getString( TargetName ) == name )
				incCount();
		}
	}

	xgc_void XQuestObjective::onPick( XObjectEvent &evt )
	{
		auto pSender = ObjectCast< XBag >( evt.sender );
		XGC_ASSERT_RETURN( pSender, XGC_NONE );

		// 不是放在自己背包，直接返回
		if( pSender->GotParent( XActor::GetThisClass() ) != GotParent( XActor::GetThisClass() ) )
			return;

		// 将对象转为实际的消息对象
		XBag::TransEvent *e = XGC_CONTAINER_OF( &evt, XBag::TransEvent, cast );

		XGoods *pGoods = ObjectCast< XGoods >( e->hGoods );
		if( pGoods )
		{
			// 查看物品是否满足计数条件
			xgc_lpcstr pName = pGoods->getString( XGoods::Alias );
			XGC_ASSERT_POINTER( pName );
			if( pName && getString( TargetName ) == pName )
				incCount( e->nCount );
		}
	}

	xgc_void XQuestObjective::onTalk( XObjectEvent &evt )
	{

	}

	xgc_void XQuestObjective::OnValueChanged( xAttrIndex nAttr )
	{
		if( nAttr == Count )
		{
			auto nCount = getValue< xgc_uint32 >( Count );
			if( nCount > getValue< xgc_uint32 >( Total ) )
				setValue( Count, nCount );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 任务
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_XCLASS_BEGIN( XQuest, XObjectNode )
	IMPLEMENT_XCLASS_END();

	XQuest::XQuest(void)
		: m_FinishMask( 0 )
		, m_Mask( 0 )
	{
	}

	XQuest::~XQuest(void)
	{
	}

	xgc_void XQuest::OnInsertChild( XObject* pChild, xgc_lpvoid lpContext )
	{
		if( false == pChild->IsInheritFrom( XQuestObjective::GetThisClass() ) )
			return;

		// 注册任务目标完成的事件，该事件被触发后，任务会收到回调。
		pChild->RegistEvent( evt_objective_finished, std::bind( &XQuest::OnFinish, this, _1 ), GetObjectID() );
	}

	xgc_void XQuest::OnRemoveChild( XObject * pChild )
	{
		// 解除自身观察的事件引用
		pChild->RemoveEvent( GetObjectID() );
	}

	xgc_void XQuest::OnFinish( XObjectEvent &evt )
	{
		// 搜索子目标中，未完成的目标
		xObject hSearch = Search( []( xObject hObject ){
			auto pObjective = ObjectCast< XQuestObjective >( hObject );
			if( pObjective )
				return pObjective->isFinished() == false;

			return false;
		} );

		// 没有未完成的目标，则该任务已完成。
		if( hSearch == INVALID_OBJECT_ID )
		{
			// 提交任务完成事件
			EmmitEvent( evt_quest_finished );
		}
	}
}
