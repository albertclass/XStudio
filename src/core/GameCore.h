#ifndef _SERVERCORE_H_
#define _SERVERCORE_H_

// 基本数据类型，及辅助计算库
#include "XMath.h"
#include "XVector2.h"
#include "XVector3.h"
#include "XGeometry.h"
#include "XVariant.h"

// 服务器对象基础类
#include "XCoreDefine.h"
#include "XClassInfo.h"
#include "XObjectComposition.h"
#include "XObject.h"

// 服务器对象
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
	/// Description:	初始化服务器内核 
	/////
	CORE_API xgc_bool InitGameCore();

	/////
	/// [11/16/2010 Albert]
	/// Description:	结束服务器内核 
	/////
	CORE_API xgc_void FiniGameCore( );

	///
	/// [10/31/2012 Albert.xu]
	/// 获取定时器
	///
	CORE_API common::timer_manager& getTimer();

	//////////////////////////////////////////////////////////////////////////
	/// object pool function
	//////////////////////////////////////////////////////////////////////////
	///
	/// [1/8/2014 albert.xu]
	/// 验证是否有效的对象ID
	///
	CORE_API xgc_bool IsValidObject( xObject );

	///
	/// [1/8/2014 albert.xu]
	/// 获取对象类型
	///
	CORE_API xClassType GetObjectType( xObject );

	/////
	/// [11/18/2010 Albert]
	/// 判断是否为有效的对象 
	/////
	CORE_API xgc_bool CheckObjectType( xObject hObject, xClassType TypeId );

	///
	/// [1/7/2014 albert.xu]
	/// 通过对象ID获取对象指针
	///
	CORE_API XObject* GetXObject( xObject );

	///
	/// [1/7/2014 albert.xu]
	/// 通过对象ID获取对象指针
	///
	CORE_API XObject* GetXObject( xObject, xClassType );

	//////////////////////////////////////////////////////////////////////////
	// object function
	//////////////////////////////////////////////////////////////////////////
	/////
	/// [11/18/2010 Albert]
	/// Description:	获取父对象 
	/////
	CORE_API xObject GetParentObject( xObject hObject );

	/////
	/// [11/18/2010 Albert]
	/// Description:	添加子对象 
	/////
	CORE_API xgc_bool AddChild( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	删除子对象 
	/////
	CORE_API xgc_void RemoveChild( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	 查询子对象
	/////
	CORE_API xgc_bool QueryChild( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	销毁对象 
	/////
	CORE_API xgc_void DestroyObject( xObject hObject );

	//////////////////////////////////////////////////////////////////////////
	// charactor function
	//////////////////////////////////////////////////////////////////////////
	/////
	/// [12/14/2010 Albert]
	/// Description:	设置触发器Reciver 
	/////
	CORE_API xgc_void RegisteTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, TriggerFunctor &Reciver, xgc_uint16 nCode = 0xffff );

	/////
	/// [12/23/2010 Albert]
	/// Description:	触发事件 
	/////
	CORE_API xgc_void TriggerEvent( xgc_uint16 nEvent, xObject hSource, xgc_intptr lParam, xgc_intptr wParam, xgc_uint16 nCode );

	///
	/// [3/20/2014 albert.xu]
	/// 解除触发器
	///
	CORE_API xgc_void DismissTrigger( xObject hObject, xgc_uint16 nEvent, xObject hWhoCare, xgc_uint16 nCode );

	/////
	/// [12/1/2010 Albert]
	/// Description:	获取对象的当前坐标 
	/////
	CORE_API const XGC::XVector3& GetPosition( xObject hObject );

	/////
	/// [1/7/2011 Albert]
	/// Description:	获取当前方向 
	/////
	CORE_API const XGC::XVector3& GetDirection( xObject hObject );

	/////
	/// [1/7/2011 Albert]
	/// Description:	设置角色状态 
	/////
	CORE_API xgc_void SetActorStatus( xObject hActor, xgc_int32 eStatus, float fDelayTime = 0.0f, xgc_int32 nMode = 0 );

	/////
	/// [1/13/2011 Albert]
	/// Description:	获取角色状态 
	/////
	CORE_API xgc_int32 GetActorStatus( xObject hActor );

	/////
	/// [1/7/2011 Albert]
	/// Description:	动态移动对象到指定位置
	/////
	CORE_API xgc_bool MoveObjectTo( xObject hObject, XGC::XVector3& vPosition, xgc_uint32 nCollisionMask, xgc_uintptr lpContext );

	/////
	/// [11/19/2010 Albert]
	/// Description:	角色受到伤害 
	/////
	CORE_API xgc_void AttackTarget( xObject hAttacker, xObject hInjured, xgc_int32 nDamage );
}

#endif //_SERVERCORE_H_

