// GameCore.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "GameCore.h"

namespace XGC
{
	using namespace common;
	static timer_manager gTimer( xgc_nullptr );
	xgc_bool InitGameCore()
	{
		srand( (xgc_uint32)time( xgc_nullptr ) );
		return true;
	}

	xgc_void FiniGameCore()
	{
	}

	timer_manager& getTimer()
	{
		return gTimer;
	}

	XObject* GetXObject( xObject xObjectID )
	{
		FUNCTION_BEGIN;
		return XObject::handle_exchange( xObjectID );
		FUNCTION_END;
		return xgc_nullptr;
	}

	XObject* GetXObject( xObject xObjectID, xClassType xClassTypeID )
	{
		FUNCTION_BEGIN;
		XObject* pObject = XObject::handle_exchange( xObjectID );
		if( pObject && pObject->IsInheritFrom( xClassTypeID ) )
		{
			return pObject;
		}
		FUNCTION_END;
		return xgc_nullptr;
	}

	//-----------------------------------//
	// [1/8/2014 albert.xu]
	// ��֤�Ƿ���Ч�Ķ���ID
	//-----------------------------------//
	xgc_bool IsValidObject( xObject xObjectID )
	{
		FUNCTION_BEGIN;
		return XObject::handle_exchange( xObjectID ) != xgc_nullptr;
		FUNCTION_END;
		return false;
	}

	///
	/// [1/8/2014 albert.xu]
	/// ��ȡ��������
	///
	xClassType GetObjectType( xObject xObjectID )
	{
		XObject* pObject = XObject::handle_exchange( xObjectID );
		if( pObject )
			return pObject->GetClassType();

		return 0;
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	�ж��Ƿ�Ϊ��Ч�Ķ��� 
	//---------------------------------------------------//
	xgc_bool CheckObjectType( xObject hObject, xClassType xType )
	{
		XObject* pObject = XObject::handle_exchange( hObject );
		if( pObject )
		{
			return pObject->IsInheritFrom( xType );
		}

		return false;
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	��ȡ������ 
	//---------------------------------------------------//
	xObject GetParentObject( xObject hObject )
	{
		XObject* pObject = GetXObject( hObject );
		XGC_ASSERT_RETURN( pObject, INVALID_OBJECT_ID );
		return pObject->GetParent();
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	����Ӷ��� 
	//---------------------------------------------------//
	xgc_bool AddChild( xObject hParent, xObject hChild )
	{
		XObject* pObject = GetXObject( hParent );
		XGC_ASSERT_POINTER( pObject );
		return pObject->AddChild( hChild );
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	ɾ���Ӷ��� 
	//---------------------------------------------------//
	xgc_void RemoveChild( xObject hParent, xObject hChild )
	{
		XObject* pObject = GetXObject( hParent );
		XGC_ASSERT_POINTER( pObject );
		pObject->RemoveChild( hChild );
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	 ��ѯ�Ӷ���
	//---------------------------------------------------//
	xgc_bool QueryChild( xObject hParent, xObject hChild )
	{
		XObject* pObject = GetXObject( hParent );
		XGC_ASSERT_POINTER( pObject );
		return pObject->QueryChild( hChild );
	}

	//---------------------------------------------------//
	// [11/18/2010 Albert]
	// Description:	���ٶ��� 
	//---------------------------------------------------//
	xgc_void DestroyObject( xObject hObject )
	{
		XObject* pObject = GetXObject( hObject );
		XGC_ASSERT_POINTER( pObject );
		pObject->Destroy();
		SAFE_DELETE( pObject );
	}

	//---------------------------------------------------//
	// [12/14/2010 Albert]
	// Description:	���ô�����Reciver 
	//---------------------------------------------------//
	xgc_void RegisteTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, TriggerFunctor &Reciver, xgc_uint16 nCode )
	{
		XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
		XGC_ASSERT_RETURN( pObject, xgc_void( 0 ) );
		pObject->RegisteTrigger( hWhoCare, nEvent, Reciver );
	}

	//---------------------------------------------------//
	// [12/23/2010 Albert]
	// Description:	�����¼� 
	//---------------------------------------------------//
	xgc_void TriggerEvent( xgc_uint16 nEvent, xObject hSource, xgc_intptr wParam, xgc_intptr lParam, xgc_uint16 nCode )
	{
		XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hSource, TypeXGameObject ) );
		XGC_ASSERT_RETURN( pObject, xgc_void( 0 ) );
		pObject->Trigger( (eTriggerEvent)nEvent, wParam, lParam, nCode );
	}

	//-----------------------------------//
	// [3/20/2014 albert.xu]
	// ���������
	//-----------------------------------//
	CORE_API xgc_void DismissTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, xgc_uint16 nCode )
	{
		XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
		XGC_ASSERT_RETURN( pObject, xgc_void( 0 ) );
		pObject->DismissTrigger( nEvent, hWhoCare, nCode );
	}

	//---------------------------------------------------//
	// [12/1/2010 Albert]
	// Description:	��ȡ����ĵ�ǰ���� 
	//---------------------------------------------------//
	const XVector3& GetPosition( xObject hObject )
	{
		XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
		XGC_ASSERT_RETURN( pObject, XVector3::ZERO );
		return pObject->GetPosition();
	}

	//---------------------------------------------------//
	// [1/7/2011 Albert]
	// Description:	��ȡ��ǰ���� 
	//---------------------------------------------------//
	const XVector3& GetDirection( xObject hObject )
	{
		XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
		XGC_ASSERT_RETURN( pObject, XVector3::ZERO );
		return pObject->GetDirection();
	}

	//---------------------------------------------------//
	// [1/7/2011 Albert]
	// Description:	���ý�ɫ״̬ 
	//---------------------------------------------------//
// 	xgc_void SetActorStatus( xObject hActor, xgc_int32 eStatus, xgc_real32 fDelayTime, xgc_int32 nMode )
// 	{
// 		XCharactor* pObject = static_cast<XCharactor*>( GetXObject( hActor, TypeXCharactor ) );
// 		if( pObject )
// 		{
// 			pObject->SetActorStatus( ( XCharactor::ActorStatus )eStatus, fDelayTime, nMode );
// 		}
// 	}

	//---------------------------------------------------//
	// [1/13/2011 Albert]
	// Description:	��ȡ��ɫ״̬ 
	//---------------------------------------------------//
	xgc_int32 GetActorStatus( xObject hActor )
	{
		XCharactor* pObject = static_cast<XCharactor*>( GetXObject( hActor, TypeXCharactor ) );
		if( pObject )
		{
			return pObject->GetActorStatus();
		}

		return -1;
	}

	//---------------------------------------------------//
	// [1/7/2011 Albert]
	// Description:	��̬�ƶ�����ָ��λ��
	//---------------------------------------------------//
	xgc_bool MoveObjectTo( xObject hObject, XVector3& vPosition, xgc_uint32 nCollisionMask, xgc_uintptr lpContext )
	{
		XGameObject* pObject = static_cast<XGameObject*>( GetXObject( hObject, TypeXGameObject ) );
		XGC_ASSERT_RETURN( pObject, false );
		XGameMap* pMap = static_cast<XGameMap*>( GetXObject( pObject->GetParent(), TypeXGameMap ) );
		if( pMap )
			return pMap->DynamicMoveTo( pObject, vPosition, nCollisionMask, lpContext );

		return false;
	}

	//---------------------------------------------------//
	// [11/19/2010 Albert]
	// Description:	��ɫ�ܵ��˺� 
	//---------------------------------------------------//
// 	xgc_void AttackTarget( xObject hAttacker, xObject hInjured, xgc_int32 nDamage )
// 	{
// 		XObject* pObject = GetXObject( hInjured, TypeXCharactor );
// 		if( pObject )
// 		{
// 			XCharactor* pActorBase = static_cast<XCharactor*>( pObject );
// 			pActorBase->UnderAttack( hAttacker, nDamage );
// 		}
// 	}
}
