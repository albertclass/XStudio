#include "StdAfx.h"
#include "XGameObject.h"

namespace XGC
{
	CORE_API xAttrIndex attrObjectName;		///< 对象名称
	CORE_API xAttrIndex attrObjectAlias;	///< 对象名称
	CORE_API xAttrIndex attrObjectFlags;	///< 对象标记位
	CORE_API xAttrIndex attrObjectRadius;	///< 对象半径
	CORE_API xAttrIndex attrObjectMobID;	///< 出现在场景中

	BEGIN_IMPLEMENT_XCLASS( XGameObject, XObject, TypeXGameObject )
		IMPLEMENT_ATTRIBUTE( ObjectName,	VT_STRING,	ATTR_FLAG_NONE, "20140912" )	// 对象名字
		IMPLEMENT_ATTRIBUTE( ObjectAlias,	VT_STRING,	ATTR_FLAG_NONE, "20140912" )	// 对象名字
		IMPLEMENT_ATTRIBUTE( ObjectFlags,	VT_U32,		ATTR_FLAG_SAVE, "20140912" )	// 是否可见
		IMPLEMENT_ATTRIBUTE( ObjectRadius,	VT_REAL,	ATTR_FLAG_NONE, "20140912" )	// 对象半径
		IMPLEMENT_ATTRIBUTE( ObjectMobID,	VT_U32,		ATTR_FLAG_NONE, "20140912" )	// 刷新索引
	END_IMPLEMENT_XCLASS()
	XGameObject::XGameObject()
		: XObject()
		, mTriggerDeep( 0 )
	{
	}

	XGameObject::~XGameObject()
	{
	}

	xgc_void XGameObject::Release()
	{
		FUNCTION_BEGIN;
		Destroy();
		delete this;
		FUNCTION_END;
	}
	xgc_void XGameObject::Trigger( xgc_uint16 nTriggerEvent, xgc_uintptr wParam, xgc_uintptr lParam, xgc_uint16 nCode )
	{
		FUNCTION_BEGIN;
		// 防止递归调用Trigger引发的迭代器错乱。
		++mTriggerDeep;

		auto iter_event = mTriggerTable.mTriggerEvent.find( nTriggerEvent );
		if( iter_event == mTriggerTable.mTriggerEvent.end() )
		{
			--mTriggerDeep;
			return;
		}

		// 先将事件转给全局观察者
		auto iter_code = iter_event->second.mTriggerCode.find( 0xffff );
		if( iter_code != iter_event->second.mTriggerCode.end() )
		{
			for each( auto iter in iter_code->second.mTriggerList )
				iter.second( GetObjectID(), iter.first, wParam, lParam, nCode );
		}

		// 再将事件转给专职观察者
		iter_code = iter_event->second.mTriggerCode.find( nCode );
		if( iter_code == iter_event->second.mTriggerCode.end() )
		{
			--mTriggerDeep;
			return;
		}

		for each( auto iter in iter_code->second.mTriggerList )
			iter.second( GetObjectID(), iter.first, wParam, lParam, nCode );

		// 回复递归层数
		if( --mTriggerDeep == 0 )
		{
			// 将Trigger过程中新加入的在这里统一处理。
		}
		FUNCTION_END;
	}

	xgc_void XGameObject::RegisteTrigger( xObject hWhoCare, xgc_uint16 nTriggerEvent, const TriggerFunctor &Call, xgc_uint16 nTriggerCode /*= 0xffff */ )
	{
		FUNCTION_BEGIN;
		if( mTriggerDeep )
		{
			// 记录新加入的Trigger
			return;
		}

		// 搜索一个空闲的位置
		auto &lst = mTriggerTable.mTriggerEvent[nTriggerEvent].mTriggerCode[nTriggerCode].mTriggerList;
		auto iter = std::find_if( lst.begin(), lst.end()
			, [hWhoCare]( const TriggerObserver& _Val ){ return hWhoCare == _Val.first; } );
		if( iter == lst.end() )
		{
			lst.push_back( TriggerObserver{ hWhoCare, Call } );
		}
		else
		{
			XGC_DEBUG_MESSAGE( "正常情况下不应该走到这里，除非你反复设置了一个观察者。" );
			iter->second = Call;
		}
		FUNCTION_END;
	}

	xgc_void XGameObject::DismissTrigger( xObject hWhoCare, xgc_uint16 nTriggerEvent, xgc_uint16 nTriggerCode /*= 0xffff*/ )
	{
		FUNCTION_BEGIN;
		if( mTriggerDeep )
		{
			// 记录删除Trigger的操作。
			return;
		}

		for( auto evt = mTriggerTable.mTriggerEvent.begin(); 
			evt != mTriggerTable.mTriggerEvent.end(); )
		{
			if( nTriggerEvent != Trigger_UnknownEvent &&
				nTriggerEvent != evt->first )
			{
				++evt;
				continue;
			}

			for( auto code = evt->second.mTriggerCode.begin();
				code != evt->second.mTriggerCode.end(); )
			{
				if( nTriggerCode != 0xffff &&
					nTriggerCode != code->first )
				{
					++code;
					continue;
				}
				

				for( auto trigger = code->second.mTriggerList.begin();
					trigger != code->second.mTriggerList.end(); )
				{
					if( trigger->first == hWhoCare )
					{
						trigger = code->second.mTriggerList.erase( trigger );
						continue;
					}

					++trigger;
				}

				if( code->second.mTriggerList.empty() )
				{
					code = evt->second.mTriggerCode.erase( code );
					continue;
				}

				++code;
			}

			if( evt->second.mTriggerCode.empty() )
			{
				evt = mTriggerTable.mTriggerEvent.erase( evt );
				continue;
			}

			++evt;
		}

		FUNCTION_END;
	}

	///
	/// 消失在场景中（从别人的视野中消失）
	/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
	/// [10/14/2014] create by albert.xu
	///
	xgc_void XGameObject::Appear( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		FUNCTION_BEGIN;
		if( GetAttribute( attrObjectFlags ).CmpBit( Flag_NotifyEyeshot, true ) )
			return;

		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			xObjectSet Objects = pMap->CaptureObjectByEyeshot( GetPosition(), fnNotifyFilter );

			for each( xObject hObject in Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				OnEnterEyeshot( pObject, VisualMode::Appear );
			}
		}

		GetAttribute( attrObjectFlags ).SetBit( Flag_NotifyEyeshot );
		FUNCTION_END;
	}

	///
	/// 显示角色周围的场景对象
	/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
	/// [12/5/2014] create by albert.xu
	///
	xgc_void XGameObject::Vanish( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		FUNCTION_BEGIN;
		if( GetAttribute( attrObjectFlags ).CmpBit( Flag_NotifyEyeshot, false ) )
			return;

		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			xObjectSet Objects = pMap->CaptureObjectByEyeshot( GetPosition(), fnNotifyFilter );

			for each( xObject hObject in Objects )
			{
				if (hObject == GetObjectID())
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				OnLeaveEyeshot( pObject, VisualMode::Vanish );
			}
		}

		GetAttribute( attrObjectFlags ).ClrBit( Flag_NotifyEyeshot );
		FUNCTION_END;
	}

	///
	/// 显示角色周围的场景对象
	/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
	/// [12/5/2014] create by albert.xu
	///
	xgc_void XGameObject::ShowOther( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			xObjectSet Objects = pMap->CaptureObjectByEyeshot( GetPosition(), fnNotifyFilter );

			for each( xObject hObject in Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				pObject->OnEnterEyeshot( this, VisualMode::ShowOther );
			}
		}
	}

	///
	/// 隐藏角色周围的场景对象
	/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
	/// [12/5/2014] create by albert.xu
	///
	xgc_void XGameObject::HideOther( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			xObjectSet Objects = pMap->CaptureObjectByEyeshot( GetPosition(), fnNotifyFilter );

			for each( xObject hObject in Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				pObject->OnLeaveEyeshot( this, VisualMode::HideOther );
			}
		}
	}

}
