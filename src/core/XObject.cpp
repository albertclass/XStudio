#include "XHeader.h"
#include "XObject.h"

namespace xgc
{
	//////////////////////////////////////////////////////////////////////////
	// CXObject
	static XAttributeImpl cls_XObject_Attribute[] =
	{
		{
			{
				VT_VOID,
				xgc_nullptr,
				0,
				0,
				xgc_nullptr,
			},

			{ 0, 0 }
		}
	};

	const XClassInfo& XObject::GetThisClass()
	{
		static XClassInfo cls_XObject( "XObject", cls_XObject_Attribute, xgc_nullptr );
		return cls_XObject;
	}

	const XClassInfo& XObject::GetRuntimeClass() const
	{
		return XObject::GetThisClass();
	}

	XObject::XObject()
		: auto_handle()
		, mIsDestory( false )
		, mParentID( INVALID_OBJECT_ID )
	{
	}

	XObject::~XObject()
	{
		XGC_ASSERT_MESSAGE( mIsDestory, "����ɾ��֮ǰδ���٣�(%p, 0X%08X)", this, handle()._handle );
		free( mAttributes );
	}

	///
	/// \brief �����ʼ��
	/// \author albert.xu
	/// \date 2017/10/09
	///
	bool XObject::InitObject()
	{
		const XClassInfo &cls = GetRuntimeClass();
		mAttributes = malloc( cls.GetAttributeSize() );

		if( !mAttributes )
		{
			SYS_ERROR( "malloc %u failed", cls.GetAttributeSize() );
			return false;
		}

		memset( mAttributes, 0, _msize( mAttributes ) );

		mAttributeInfo = cls.GetAttributeInfo();
		return true;
	}

	// ��ȡ������ID
	xObject XObject::GotParent( const XClassInfo & cls ) const
	{
		XObject *pObject = ObjectCast< XObject >( GetObjectID() );
		while( pObject && cls != pObject->GetRuntimeClass() )
		{
			pObject = ObjectCast< XObject >( pObject->GetParent() );
		}

		return pObject ? pObject->GetObjectID() : INVALID_OBJECT_ID;
	}

	xgc_void XObject::Destroy()
	{
		if( false == mIsDestory )
		{
			mIsDestory = true;

			OnDestroy();
			delete this;
		}
	}

	/************************************************************************/
	/* �¼�����
	/************************************************************************/
	///
	/// \brief ע���¼�
	/// \author albert.xu
	/// \date 2017/10/12
	/// \return ���¼���token������ʹ�ø�tokenɾ��ע����¼�
	///
	xgc_long XObject::RegistEvent( xgc_long id, const xNotify &invoke, xObject hOwner /*= INVALID_OBJECT_ID*/ )
	{
		auto &observer = mEventSubject[id];

		if( observer.count )
			return -1;

		observer.actions[++observer.token] = std::make_tuple( invoke, hOwner );

		return observer.token;
	}

	///
	/// \brief ɾ���¼�
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::RemoveEvent( xgc_long id, xgc_long token )
	{
		auto it1 = mEventSubject.find( id );
		if( it1 == mEventSubject.end() )
			return;

		auto &observer = it1->second;
		// ��ֹ���¼�ִ���ڼ���ɾ�����µ�����ʧЧ
		if( observer.count )
		{
			return;
		}
		else
		{
			auto it2 = observer.actions.find( token );
			if( it2 == observer.actions.end() )
				return;

			observer.actions.erase( it2 );
		}
	}

	xgc_void XObject::RemoveEvent( xObject hOwner )
	{
		auto it1 = mEventSubject.begin();
		while( it1 != mEventSubject.end() )
		{
			auto &observer = it1->second;
			if( 0 == observer.count )
			{
				auto it2 = observer.actions.begin();
				while( it2 != observer.actions.end() )
				{
					if( hOwner == std::get< 1 >( it2->second ) )
						it2 = observer.actions.erase( it2 );
					else
						++it2;
				}
			}
			++it1;
		}
	}

	///
	/// \brief �����¼�
	/// \author albert.xu
	/// \date 2017/11/03
	///

	xgc_void XObject::EnableEvent( xgc_long id, xgc_bool enable )
	{
		auto it = mEventSubject.find( id );
		if( it == mEventSubject.end() )
			return;

		it->second.close = !enable;
	}

	///
	/// \brief �ύ�¼�
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::EmmitEvent( xgc_long id, xgc_long direction )
	{
		XObjectEvent evt;

		evt.id		= id;
		evt.over	= direction;
		evt.result	= 0;
		evt.sender	= GetObjectID();
		evt.target	= INVALID_OBJECT_ID;

		EmmitEvent( &evt, false );
	}

	///
	/// \brief �ύ�¼�
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::EmmitEvent( XObjectEvent& evt, xgc_long id, xgc_long direction )
	{
		evt.id = id;
		evt.over = direction;
		evt.result = 0;
		evt.sender = GetObjectID();
		evt.target = INVALID_OBJECT_ID;

		EmmitEvent( &evt, false );
	}

	///
	/// \brief �ύ�¼�
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::EmmitEvent( XObjectEvent* evt, xgc_void( *DeleteIt )( XObjectEvent* ) )
	{
		auto it1 = mEventSubject.find( evt->id );
		if( it1 != mEventSubject.end() )
		{
			// ��֪ͨ���صĹ۲���
			auto &observer = it1->second;

			// �ر���Ϣ֪ͨ
			if( observer.close == false )
			{
				// ��ֹ�ڴ�����ִ���ڼ���ɾ������
				++observer.count;
				for( auto &pair : observer.actions )
				{
					// ȡtoken
					evt->token = pair.first;
					// ȡĿ�����ID
					evt->target = std::get< 1 >( pair.second );
					// ȡ������������ִ��
					std::get< 0 >( pair.second )( *evt );

					if( evt->over < 0 )
						break;
				}
				--observer.count;
			}
		}

		if( evt->over > 0 )
		{
			// ͸����Ϣ�� id Ϊ -1 �Ĺ۲��߼����ö����������Ϣ
			auto it2 = mEventSubject.find( -1 );
			if( it2 == mEventSubject.end() )
				return;

			auto &observer = it2->second;
			++observer.count;
			for( auto &pair : observer.actions )
			{
				// ȡtoken
				evt->token = pair.first;
				// ȡĿ�����ID
				evt->target = std::get< 1 >( pair.second );
				// ȡ������������ִ��
				std::get< 0 >( pair.second )( *evt );

				if( evt->over < 0 )
					break;
			}
			--observer.count;
		}

		if( evt->over > 0 && XGC_CHK_FLAGS( evt->over, 1 ) )
		{
			// ����δ�����¼�����㼶�ϼ�����
			auto xParent = GetParent();
			if( xParent == INVALID_OBJECT_ID )
			{
				// ������Լ���ȫ�ֵ��¼�֪ͨ
			}
			else
			{
				// ��������ڵģ���֪ͨ������
				auto pParent = ObjectCast< XObject >( xParent );
				if( pParent )
				{
					pParent->EmmitEvent( evt );
				}
			}
		}

		if( evt->over > 0 && XGC_CHK_FLAGS( evt->over, 2 ) )
		{
			// ���ݸ��Ӷ���
			XObjectNode* pNode = ObjectCast< XObjectNode >( GetObjectID() );
			if( pNode )
			{
				// ö�ٳ����е��Ӷ��󣬲����¼����͸��Ӷ���
				pNode->Search( [evt ]( xObject hObject )->xgc_bool{
					auto pChild = ObjectCast< XObject >( hObject );
					if( pChild )
						pChild->EmmitEvent( evt );

					return false;
				} );
			}
		}

		if( DeleteIt ) DeleteIt( evt );
	}

	xgc_bool XObject::LoadObject( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize )
	{
		const XClassInfo &cls = GetRuntimeClass();

		attr_buffer ar( reference_buffer( lpData, uSize ) );

		auto nImplementCount = cls.GetImplementCount();
		auto pImplementInfo  = cls.GetImplementInfo();

		for( xgc_size i = 0, j = 0; i < nImplementCount; ++i )
		{
			auto &version = pImplementInfo[i]->version;
			if( uVersion < version.start ||	uVersion >= version.close )
				continue;

			auto &impl = pImplementInfo[i]->impl;
			if( ( impl.flags & 0x7f ) == ATTR_FLAG_SAVE )
			{
				ar >> getAttr( *impl.attr_ptr );
			}
			++j;
		}
		return true;
	}

	xgc_size XObject::SaveObject( xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize )
	{
		const XClassInfo &cls = GetRuntimeClass();
		auto nAttributeCount = cls.GetAttributeCount();
		auto pAttributeInfo  = cls.GetAttributeInfo();

		attr_buffer ar( reference_buffer( lpData, uSize ) );

		for( xgc_size i = 0, j = 0; i < nAttributeCount; ++i )
		{
			auto &impl = pAttributeInfo[i]->impl;
			if( ( impl.flags & 0x7f ) == ATTR_FLAG_SAVE )
				ar << getAttr( *impl.attr_ptr );

			++j;
		}

		return ar.wd();
	}

	///
	/// [2/11/2014 albert.xu]
	/// ��ȡ����
	/// �ú����ṩһ����������������Hook�ķ������������ú�ȡֵ�����Ե���Hook��
	///
	XAttribute XObject::getAttr( xAttrIndex nAttr ) const
	{
		XGC_ASSERT_THROW( isAttrValid( nAttr ), std::logic_error( "get attribute, index out of bound." ) );

		// ��ȡ������Ϣ
		auto info = mAttributeInfo[nAttr];

		// �������Բ�������
		return XAttribute( info->impl.type, (xgc_lpstr)mAttributes + info->offset );
	}
}