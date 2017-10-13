#include "XHeader.h"
#include "XGameObject.h"
#include "XGameMap.h"

namespace xgc
{
	CORE_API xAttrIndex attrObjectName;		///< ��������
	CORE_API xAttrIndex attrObjectAlias;	///< ��������
	CORE_API xAttrIndex attrObjectFlags;	///< ������λ
	CORE_API xAttrIndex attrObjectRadius;	///< ����뾶
	CORE_API xAttrIndex attrObjectMobID;	///< �����ڳ�����

	IMPLEMENT_XCLASS_BEGIN( XGameObject, XObject )
		IMPLEMENT_ATTRIBUTE( ObjectName,	VT_STRING,	ATTR_FLAG_NONE, "20140912" )	// ��������
		IMPLEMENT_ATTRIBUTE( ObjectAlias,	VT_STRING,	ATTR_FLAG_NONE, "20140912" )	// ��������
		IMPLEMENT_ATTRIBUTE( ObjectFlags,	VT_U32,		ATTR_FLAG_SAVE, "20140912" )	// �Ƿ�ɼ�
		IMPLEMENT_ATTRIBUTE( ObjectRadius,	VT_REAL32,	ATTR_FLAG_NONE, "20140912" )	// ����뾶
		IMPLEMENT_ATTRIBUTE( ObjectMobID,	VT_U32,		ATTR_FLAG_NONE, "20140912" )	// ˢ������
	IMPLEMENT_XCLASS_END()
	XGameObject::XGameObject()
		: XObject()
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

	///
	/// ��ʧ�ڳ����У��ӱ��˵���Ұ����ʧ��
	/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
	/// [10/14/2014] create by albert.xu
	///
	xgc_void XGameObject::Appear( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		FUNCTION_BEGIN;
		if( getAttr( attrObjectFlags ).CmpBit( Flag_NotifyEyeshot, true ) )
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
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				OnEnterEyeshot( pObject, VisualMode::eAppear );
			}
		}

		getAttr( attrObjectFlags ).SetBit( Flag_NotifyEyeshot );
		FUNCTION_END;
	}

	///
	/// ��ʾ��ɫ��Χ�ĳ�������
	/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
	/// [12/5/2014] create by albert.xu
	///
	xgc_void XGameObject::Vanish( const std::function< xgc_bool( xObject ) > &fnNotifyFilter )
	{
		FUNCTION_BEGIN;
		if( getAttr( attrObjectFlags ).CmpBit( Flag_NotifyEyeshot, false ) )
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
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				OnLeaveEyeshot( pObject, VisualMode::eVanish );
			}
		}

		getAttr( attrObjectFlags ).ClrBit( Flag_NotifyEyeshot );
		FUNCTION_END;
	}

	///
	/// ��ʾ��ɫ��Χ�ĳ�������
	/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
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
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				pObject->OnEnterEyeshot( this, VisualMode::eShowOther );
			}
		}
	}

	///
	/// ���ؽ�ɫ��Χ�ĳ�������
	/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
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
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				pObject->OnLeaveEyeshot( this, VisualMode::eHideOther );
			}
		}
	}

}
