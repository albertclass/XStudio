// GameCore.cpp : 定义 DLL 应用程序的入口点。
//
#include "XHeader.h"
#include "XCore.h"

namespace xgc
{
	using namespace common;
	static timer gTimer( steady_tickcount );
	static timer gClock( system_tickcount );

	xgc_bool InitGameCore()
	{
		srand( (xgc_uint32)time( xgc_nullptr ) );
		return true;
	}

	xgc_void FiniGameCore()
	{
	}

	timer& getTimer()
	{
		return gTimer;
	}

	timer& getClock()
	{
		return gClock;
	}

	template< class T >
	T* GetXObject( xObject xObjectID )
	{
		XObject* pObject = XObject::handle_exchange( xObjectID );
		if( !pObject )
			return xgc_nullptr;
		
		if( !pObject->IsInheritFrom( T::GetThisClass() ) )
			return xgc_nullptr;
		
		return static_cast< T* >( pObject );
	}

	//-----------------------------------//
	// [1/8/2014 albert.xu]
	// 验证是否有效的对象ID
	//-----------------------------------//
	xgc_bool IsValidObject( xObject xObjectID )
	{
		return XObject::handle_exchange( xObjectID ) != xgc_nullptr;
	}

	///
	/// [1/8/2014 albert.xu]
	/// 获取对象类型
	///
	const XClassInfo* GetClassInfo( xObject xObjectID )
	{
		XObject* pObject = XObject::handle_exchange( xObjectID );
		if( pObject )
			return &pObject->GetRuntimeClass();

		return 0;
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	判断是否为有效的对象 
	//---------------------------------------------------//
	xgc_bool CheckObjectType( xObject hObject, const XClassInfo *cls )
	{
		XObject* pObject = XObject::handle_exchange( hObject );
		if( pObject )
		{
			return pObject->IsInheritFrom( cls );
		}

		return false;
	}

	///
	/// \brief 通过对象ID获取对象指针
	/// \author albert.xu
	/// \date 2017/10/10
	///
	XObject* GetXObject( xObject xObjectID )
	{
		return XObject::handle_exchange( xObjectID );
	}

	///
	/// \brief 通过对象ID获取对象指针，并检查类型
	/// \author albert.xu
	/// \date 2017/10/10
	///
	XObject* GetXObject( xObject xObjectID, const XClassInfo *pClassInfo )
	{
		XObject* pObject = XObject::handle_exchange( xObjectID );
		if( !pObject )
			return xgc_nullptr;

		if( !pObject->IsInheritFrom( pClassInfo ) )
			return xgc_nullptr;

		return pObject;
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	获取父对象 
	//---------------------------------------------------//
	xObject GetParent( xObject hObject )
	{
		XObject* pObject = GetXObject( hObject );
		XGC_ASSERT_RETURN( pObject, INVALID_OBJECT_ID );
		return pObject->GetParent();
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	添加子对象 
	//---------------------------------------------------//
	xgc_bool Insert( xObject hParent, xObject hChild, xgc_lpvoid pContext )
	{
		auto pObject = ObjectCast< XObjectNode >( hParent );
		XGC_ASSERT_POINTER( pObject );
		return pObject->Insert( GetXObject( hChild ), pContext );
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	删除子对象 
	//---------------------------------------------------//
	xgc_void Remove( xObject hParent, xObject hChild )
	{
		auto pObject = ObjectCast< XObjectNode >( hParent );
		XGC_ASSERT_POINTER( pObject );
		pObject->Remove( GetXObject( hChild ) );
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	 查询子对象
	//---------------------------------------------------//
	xgc_bool QueryChild( xObject hParent, const std::function<xgc_bool( xObject )>& filter, const XClassInfo *pClass )
	{
		auto pObject = ObjectCast< XObjectNode >( hParent );
		XGC_ASSERT_POINTER( pObject );
		return pObject->Search( filter, pClass );
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	销毁对象 
	//---------------------------------------------------//
	xgc_void DestroyObject( xObject hObject )
	{
		XObject* pObject = GetXObject( hObject );
		XGC_ASSERT_POINTER( pObject );
		pObject->Destroy();
	}

	////---------------------------------------------------//
	//// [12/14/2010 Albert]
	//// Description:	设置触发器Reciver 
	////---------------------------------------------------//
	//xgc_void RegisteTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, TriggerFunctor &Reciver, xgc_uint16 nCode )
	//{
	//	XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
	//	XGC_ASSERT_RETURN( pObject, xgc_void( 0 ) );
	//	pObject->RegisteTrigger( hWhoCare, nEvent, Reciver );
	//}

	////---------------------------------------------------//
	//// [12/23/2010 Albert]
	//// Description:	触发事件 
	////---------------------------------------------------//
	//xgc_void TriggerEvent( xgc_uint16 nEvent, xObject hSource, xgc_intptr wParam, xgc_intptr lParam, xgc_uint16 nCode )
	//{
	//	XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hSource, TypeXGameObject ) );
	//	XGC_ASSERT_RETURN( pObject, xgc_void( 0 ) );
	//	pObject->Trigger( (eTriggerEvent)nEvent, wParam, lParam, nCode );
	//}

	////-----------------------------------//
	//// [3/20/2014 albert.xu]
	//// 解除触发器
	////-----------------------------------//
	//CORE_API xgc_void DismissTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, xgc_uint16 nCode )
	//{
	//	XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
	//	XGC_ASSERT_RETURN( pObject, xgc_void( 0 ) );
	//	pObject->DismissTrigger( nEvent, hWhoCare, nCode );
	//}

	////---------------------------------------------------//
	//// [12/1/2010 Albert]
	//// Description:	获取对象的当前坐标 
	////---------------------------------------------------//
	//const XVector3& GetPosition( xObject hObject )
	//{
	//	XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
	//	XGC_ASSERT_RETURN( pObject, XVector3::ZERO );
	//	return pObject->GetPosition();
	//}

	////---------------------------------------------------//
	//// [1/7/2011 Albert]
	//// Description:	获取当前方向 
	////---------------------------------------------------//
	//const XVector3& GetDirection( xObject hObject )
	//{
	//	XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
	//	XGC_ASSERT_RETURN( pObject, XVector3::ZERO );
	//	return pObject->GetDirection();
	//}

	//---------------------------------------------------//
	// [1/7/2011 Albert]
	// Description:	设置角色状态 
	//---------------------------------------------------//
	// xgc_void SetState( xObject hActor, xgc_int32 eStatus, xgc_real32 fDelayTime, xgc_int32 nMode )
	// {
	// 	XActor* pObject = static_cast<XActor*>( GetXObject( hActor, TypeXCharactor ) );
	// 	if( pObject )
	// 	{
	// 		pObject->SetState( ( XActor::enActorState )eStatus, fDelayTime, nMode );
	// 	}
	// }

	////---------------------------------------------------//
	//// [1/13/2011 Albert]
	//// Description:	获取角色状态 
	////---------------------------------------------------//
	//xgc_int32 getStatus( xObject hActor )
	//{
	//	XActor* pObject = static_cast<XActor*>( GetXObject( hActor, TypeXCharactor ) );
	//	if( pObject )
	//	{
	//		return pObject->getStatus();
	//	}

	//	return -1;
	//}

	////---------------------------------------------------//
	//// [1/7/2011 Albert]
	//// Description:	动态移动对象到指定位置
	////---------------------------------------------------//
	//xgc_bool MoveObjectTo( xObject hObject, XVector3& vPosition, xgc_uint32 nCollisionMask, xgc_uintptr lpContext )
	//{
	//	XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
	//	XGC_ASSERT_RETURN( pObject, false );
	//	XGameMap* pMap = static_cast<XGameMap*>( GetXObject( pObject->GetParent(), TypeXGameMap ) );
	//	if( pMap )
	//		return pMap->DynamicMoveTo( pObject, vPosition, nCollisionMask, lpContext );

	//	return false;
	//}

	//---------------------------------------------------//
	// [11/19/2010 Albert]
	// Description:	角色受到伤害 
	//---------------------------------------------------//
	// 	xgc_void AttackTarget( xObject hAttacker, xObject hInjured, xgc_int32 nDamage )
	// 	{
	// 		XObject* pObject = GetXObject( hInjured, TypeXCharactor );
	// 		if( pObject )
	// 		{
	// 			XActor* pActorBase = static_cast<XActor*>( pObject );
	// 			pActorBase->UnderAttack( hAttacker, nDamage );
	// 		}
	// 	}
}
