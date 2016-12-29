#ifndef _SERVERCORE_H_
#define _SERVERCORE_H_

// �����������ͣ������������
#include "XMath.h"
#include "XVector2.h"
#include "XVector3.h"
#include "XGeometry.h"
#include "XVariant.h"

// ���������������
#include "XCoreDefine.h"
#include "XClassInfo.h"
#include "XObjectComposition.h"
#include "XObject.h"

// ����������
#include "XAction.h"
#include "XGameObject.h"
#include "XGameMap.h"
#include "XCharactor.h"
#include "XSkillBase.h"
#include "XStateObject.h"
#include "XTeam.h"

#if !defined( _USRDLL ) && !defined( _LIB )
#	pragma message( "Link ServerCore.lib" )
#	pragma comment( lib, "ServerCore.lib" )
#endif

namespace XGC
{
	/////
	/// [11/16/2010 Albert]
	/// Description:	��ʼ���������ں� 
	/////
	CORE_API xgc_bool InitGameCore();

	/////
	/// [11/16/2010 Albert]
	/// Description:	�����������ں� 
	/////
	CORE_API xgc_void FiniGameCore( );

	///
	/// [10/31/2012 Albert.xu]
	/// ��ȡ��ʱ��
	///
	CORE_API common::timer_manager& getTimer();

	//////////////////////////////////////////////////////////////////////////
	/// object pool function
	//////////////////////////////////////////////////////////////////////////
	///
	/// [1/8/2014 albert.xu]
	/// ��֤�Ƿ���Ч�Ķ���ID
	///
	CORE_API xgc_bool IsValidObject( xObject );

	///
	/// [1/8/2014 albert.xu]
	/// ��ȡ��������
	///
	CORE_API xClassType GetObjectType( xObject );

	/////
	/// [11/18/2010 Albert]
	/// �ж��Ƿ�Ϊ��Ч�Ķ��� 
	/////
	CORE_API xgc_bool CheckObjectType( xObject hObject, xClassType TypeId );

	///
	/// [1/7/2014 albert.xu]
	/// ͨ������ID��ȡ����ָ��
	///
	CORE_API XObject* GetXObject( xObject );

	///
	/// [1/7/2014 albert.xu]
	/// ͨ������ID��ȡ����ָ��
	///
	CORE_API XObject* GetXObject( xObject, xClassType );

	//////////////////////////////////////////////////////////////////////////
	// object function
	//////////////////////////////////////////////////////////////////////////
	/////
	/// [11/18/2010 Albert]
	/// Description:	��ȡ������ 
	/////
	CORE_API xObject GetParentObject( xObject hObject );

	/////
	/// [11/18/2010 Albert]
	/// Description:	����Ӷ��� 
	/////
	CORE_API xgc_bool AddChild( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	ɾ���Ӷ��� 
	/////
	CORE_API xgc_void RemoveChild( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	 ��ѯ�Ӷ���
	/////
	CORE_API xgc_bool QueryChild( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	���ٶ��� 
	/////
	CORE_API xgc_void DestroyObject( xObject hObject );

	//////////////////////////////////////////////////////////////////////////
	// charactor function
	//////////////////////////////////////////////////////////////////////////
	/////
	/// [12/14/2010 Albert]
	/// Description:	���ô�����Reciver 
	/////
	CORE_API xgc_void RegisteTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, TriggerFunctor &Reciver, xgc_uint16 nCode = 0xffff );

	/////
	/// [12/23/2010 Albert]
	/// Description:	�����¼� 
	/////
	CORE_API xgc_void TriggerEvent( xgc_uint16 nEvent, xObject hSource, xgc_intptr lParam, xgc_intptr wParam, xgc_uint16 nCode );

	///
	/// [3/20/2014 albert.xu]
	/// ���������
	///
	CORE_API xgc_void DismissTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, xgc_uint16 nCode );

	/////
	/// [12/1/2010 Albert]
	/// Description:	��ȡ����ĵ�ǰ���� 
	/////
	CORE_API const XGC::XVector3& GetPosition( xObject hObject );

	/////
	/// [1/7/2011 Albert]
	/// Description:	��ȡ��ǰ���� 
	/////
	CORE_API const XGC::XVector3& GetDirection( xObject hObject );

	/////
	/// [1/7/2011 Albert]
	/// Description:	���ý�ɫ״̬ 
	/////
	CORE_API xgc_void SetActorStatus( xObject hActor, xgc_int32 eStatus, float fDelayTime = 0.0f, xgc_int32 nMode = 0 );

	/////
	/// [1/13/2011 Albert]
	/// Description:	��ȡ��ɫ״̬ 
	/////
	CORE_API xgc_int32 GetActorStatus( xObject hActor );

	/////
	/// [1/7/2011 Albert]
	/// Description:	��̬�ƶ�����ָ��λ��
	/////
	CORE_API xgc_bool MoveObjectTo( xObject hObject, XGC::XVector3& vPosition, xgc_uint32 nCollisionMask, xgc_uintptr lpContext );

	/////
	/// [11/19/2010 Albert]
	/// Description:	��ɫ�ܵ��˺� 
	/////
	CORE_API xgc_void AttackTarget( xObject hAttacker, xObject hInjured, xgc_int32 nDamage );
}

#endif //_SERVERCORE_H_

