#include "XHeader.h"
#include "XGameObject.h"
#include "XGameMap.h"

namespace xgc
{
	xAttrIndex XGameObject::Name;	///< 对象名称
	xAttrIndex XGameObject::Alias;	///< 对象别名
	xAttrIndex XGameObject::Flags;	///< 对象标记位

	IMPLEMENT_XCLASS_BEGIN( XGameObject, XObject )
		IMPLEMENT_ATTRIBUTE( Name, VT_STRING, ATTR_FLAG_NONE, "20140912" )	// 对象名字
		IMPLEMENT_ATTRIBUTE( Alias, VT_STRING, ATTR_FLAG_NONE, "20140912" )	// 对象别名
		IMPLEMENT_ATTRIBUTE( Flags, VT_U32, ATTR_FLAG_SAVE, "20140912" )	// 是否可见
	IMPLEMENT_XCLASS_END();

	XGameObject::XGameObject()
	{
	}

	XGameObject::~XGameObject()
	{
	}

	///
	/// 消失在场景中（从别人的视野中消失）
	/// @param fnNotifyFilter 用于决定离开视野消息发送的条件
	/// [10/14/2014] create by albert.xu
	///
	xgc_void XGameObject::Appear( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		FUNCTION_BEGIN;
		if( ChkBit( Flags, Flag_NotifyEyeshot, true ) )
			return;

		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			xObjectSet Objects = pMap->CaptureObjectByEyeshot( GetPosition(), fnNotifyFilter );

			for( xObject hObject : Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				OnEnterEyeshot( pObject, eVisualMode::eAppear );
			}
		}

		SetBit( Flags, Flag_NotifyEyeshot );
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
		if( ChkBit( XGameObject::Flags, Flag_NotifyEyeshot, false ) )
			return;

		XGameMap* pMap = ObjectCast< XGameMap >( GetParent() );
		if( pMap )
		{
			xObjectSet Objects = pMap->CaptureObjectByEyeshot( GetPosition(), fnNotifyFilter );

			for( xObject hObject : Objects )
			{
				if (hObject == GetObjectID())
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				OnLeaveEyeshot( pObject, eVisualMode::eVanish );
			}
		}

		ClrBit( XGameObject::Flags, Flag_NotifyEyeshot );
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

			for( xObject hObject : Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				pObject->OnEnterEyeshot( this, eVisualMode::eShowOther );
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

			for( xObject hObject : Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "无效的对象句柄%x", hObject );

				pObject->OnLeaveEyeshot( this, eVisualMode::eHideOther );
			}
		}
	}

}
