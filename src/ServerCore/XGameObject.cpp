#include "StdAfx.h"
#include "XGameObject.h"

namespace XGC
{
	CORE_API xAttrIndex attrObjectName;		///< ��������
	CORE_API xAttrIndex attrObjectAlias;	///< ��������
	CORE_API xAttrIndex attrObjectFlags;	///< ������λ
	CORE_API xAttrIndex attrObjectRadius;	///< ����뾶
	CORE_API xAttrIndex attrObjectMobID;	///< �����ڳ�����

	BEGIN_IMPLEMENT_XCLASS( XGameObject, XObject, TypeXGameObject )
		IMPLEMENT_ATTRIBUTE( ObjectName,	VT_STRING,	ATTR_FLAG_NONE, "20140912" )	// ��������
		IMPLEMENT_ATTRIBUTE( ObjectAlias,	VT_STRING,	ATTR_FLAG_NONE, "20140912" )	// ��������
		IMPLEMENT_ATTRIBUTE( ObjectFlags,	VT_U32,		ATTR_FLAG_SAVE, "20140912" )	// �Ƿ�ɼ�
		IMPLEMENT_ATTRIBUTE( ObjectRadius,	VT_REAL,	ATTR_FLAG_NONE, "20140912" )	// ����뾶
		IMPLEMENT_ATTRIBUTE( ObjectMobID,	VT_U32,		ATTR_FLAG_NONE, "20140912" )	// ˢ������
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
		// ��ֹ�ݹ����Trigger�����ĵ��������ҡ�
		++mTriggerDeep;

		auto iter_event = mTriggerTable.mTriggerEvent.find( nTriggerEvent );
		if( iter_event == mTriggerTable.mTriggerEvent.end() )
		{
			--mTriggerDeep;
			return;
		}

		// �Ƚ��¼�ת��ȫ�ֹ۲���
		auto iter_code = iter_event->second.mTriggerCode.find( 0xffff );
		if( iter_code != iter_event->second.mTriggerCode.end() )
		{
			for each( auto iter in iter_code->second.mTriggerList )
				iter.second( GetObjectID(), iter.first, wParam, lParam, nCode );
		}

		// �ٽ��¼�ת��רְ�۲���
		iter_code = iter_event->second.mTriggerCode.find( nCode );
		if( iter_code == iter_event->second.mTriggerCode.end() )
		{
			--mTriggerDeep;
			return;
		}

		for each( auto iter in iter_code->second.mTriggerList )
			iter.second( GetObjectID(), iter.first, wParam, lParam, nCode );

		// �ظ��ݹ����
		if( --mTriggerDeep == 0 )
		{
			// ��Trigger�������¼����������ͳһ����
		}
		FUNCTION_END;
	}

	xgc_void XGameObject::RegisteTrigger( xObject hWhoCare, xgc_uint16 nTriggerEvent, const TriggerFunctor &Call, xgc_uint16 nTriggerCode /*= 0xffff */ )
	{
		FUNCTION_BEGIN;
		if( mTriggerDeep )
		{
			// ��¼�¼����Trigger
			return;
		}

		// ����һ�����е�λ��
		auto &lst = mTriggerTable.mTriggerEvent[nTriggerEvent].mTriggerCode[nTriggerCode].mTriggerList;
		auto iter = std::find_if( lst.begin(), lst.end()
			, [hWhoCare]( const TriggerObserver& _Val ){ return hWhoCare == _Val.first; } );
		if( iter == lst.end() )
		{
			lst.push_back( TriggerObserver{ hWhoCare, Call } );
		}
		else
		{
			XGC_DEBUG_MESSAGE( "��������²�Ӧ���ߵ���������㷴��������һ���۲��ߡ�" );
			iter->second = Call;
		}
		FUNCTION_END;
	}

	xgc_void XGameObject::DismissTrigger( xObject hWhoCare, xgc_uint16 nTriggerEvent, xgc_uint16 nTriggerCode /*= 0xffff*/ )
	{
		FUNCTION_BEGIN;
		if( mTriggerDeep )
		{
			// ��¼ɾ��Trigger�Ĳ�����
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
	/// ��ʧ�ڳ����У��ӱ��˵���Ұ����ʧ��
	/// @param fnNotifyFilter ���ھ����뿪��Ұ��Ϣ���͵�����
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
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				OnEnterEyeshot( pObject, VisualMode::Appear );
			}
		}

		GetAttribute( attrObjectFlags ).SetBit( Flag_NotifyEyeshot );
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
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				OnLeaveEyeshot( pObject, VisualMode::Vanish );
			}
		}

		GetAttribute( attrObjectFlags ).ClrBit( Flag_NotifyEyeshot );
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

			for each( xObject hObject in Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				pObject->OnEnterEyeshot( this, VisualMode::ShowOther );
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

			for each( xObject hObject in Objects )
			{
				if( hObject == GetObjectID() )
					continue;

				XGameObject* pObject = ObjectCast< XGameObject >( hObject );
				XGC_ASSERT_CONTINUE( pObject, "��Ч�Ķ�����%x", hObject );

				pObject->OnLeaveEyeshot( this, VisualMode::HideOther );
			}
		}
	}

}
