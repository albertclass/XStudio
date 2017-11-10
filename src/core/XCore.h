#ifndef __CORE_H__
#define __CORE_H__

// ����������
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
	CORE_API timer& getTimer();

	///
	/// [10/31/2012 Albert.xu]
	/// ��ȡ��ʱ��
	///
	CORE_API timer& getClock();

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
	CORE_API const XClassInfo* GetObjectType( xObject );

	/////
	/// [11/18/2010 Albert]
	/// �ж��Ƿ�Ϊ��Ч�Ķ��� 
	/////
	CORE_API xgc_bool CheckObjectType( xObject hObject, const XClassInfo* pClassInfo );

	///
	/// \brief ͨ������ID��ȡ����ָ��
	/// \author albert.xu
	/// \date 2017/10/10
	///
	CORE_API XObject* GetXObject( xObject xObjectID );

	///
	/// \brief ͨ������ID��ȡ����ָ�룬���������
	/// \author albert.xu
	/// \date 2017/10/10
	///
	CORE_API XObject* GetXObject( xObject xObjectID, const XClassInfo *pClassInfo );

	//////////////////////////////////////////////////////////////////////////
	// object function
	//////////////////////////////////////////////////////////////////////////
	/////
	/// [11/18/2010 Albert]
	/// Description:	��ȡ������ 
	/////
	CORE_API xObject GetParent( xObject hObject );

	/////
	/// [11/18/2010 Albert]
	/// Description:	����Ӷ��� 
	/////
	CORE_API xgc_bool Insert( xObject hParent, xObject hChild, xgc_lpvoid pContext );

	/////
	/// [11/18/2010 Albert]
	/// Description:	ɾ���Ӷ��� 
	/////
	CORE_API xgc_void Remove( xObject hParent, xObject hChild );

	/////
	/// [11/18/2010 Albert]
	/// Description:	 ��ѯ�Ӷ���
	/////
	CORE_API xgc_bool QueryChild( xObject hParent, const std::function<xgc_bool( xObject )>& filter, const XClassInfo * pClass );

	/////
	/// [11/18/2010 Albert]
	/// Description:	���ٶ��� 
	/////
	CORE_API xgc_void DestroyObject( xObject hObject );
}

#endif // __CORE_H__

