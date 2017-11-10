#ifndef __CORE_H__
#define __CORE_H__

// 服务器对象
//#include "XAction.h"
//#include "XGameObject.h"
//#include "XGameMap.h"
//#include "XActor.h"
//#include "XSkillBase.h"
//#include "XStateObject.h"
//#include "XTeam.h"

namespace xgc
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
	CORE_API timer& getTimer();

	///
	/// [10/31/2012 Albert.xu]
	/// 获取计时器
	///
	CORE_API timer& getClock();

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
	CORE_API const XClassInfo* GetObjectType( xObject );

	/////
	/// [11/18/2010 Albert]
	/// 判断是否为有效的对象 
	/////
	CORE_API xgc_bool CheckObjectType( xObject hObject, const XClassInfo* pClassInfo );

	///
	/// \brief 通过对象ID获取对象指针
	/// \author albert.xu
	/// \date 2017/10/10
	///
	CORE_API XObject* GetXObject( xObject xObjectID );

	///
	/// \brief 通过对象ID获取对象指针，并检查类型
	/// \author albert.xu
	/// \date 2017/10/10
	///
	CORE_API XObject* GetXObject( xObject xObjectID, const XClassInfo *pClassInfo );

	//////////////////////////////////////////////////////////////////////////
	// object function
	//////////////////////////////////////////////////////////////////////////
	/////
	/// [11/18/2010 Albert]
	/// Description:	获取父对象 
	/////
	CORE_API xObject GetParent( xObject hObject );

	/////
	/// [11/18/2010 Albert]
	/// Description:	添加子对象 
	/////
	CORE_API xgc_bool Insert( xObject hParent, xObject hChild, xgc_lpvoid pContext );

	/////
	/// [11/18/2010 Albert]
	/// Description:	删除子对象 
	/////
	CORE_API xgc_void Remove( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	 查询子对象
	/////
	CORE_API xgc_bool QueryChild( xObject hParent, const std::function<xgc_bool( xObject )>& filter, const XClassInfo * pClass );

	/////
	/// [11/18/2010 Albert]
	/// Description:	销毁对象 
	/////
	CORE_API xgc_void DestroyObject( xObject hObject );
}

#endif // __CORE_H__

