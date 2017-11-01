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
		XGC_ASSERT_MESSAGE( mIsDestory, "对象被删除之前未销毁！(%p, 0X%08X)", this, handle()._handle );
		free( mAttributes );
	}

	///
	/// \brief 对象初始化
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
		mImplementInfo = cls.GetImplementInfo();
		return true;
	}

	xgc_void XObject::Destroy()
	{
		if( false == mIsDestory )
		{
			mIsDestory = true;

			OnDestory();
			delete this;
		}
	}

	/************************************************************************/
	/* 事件操作
	/************************************************************************/
	///
	/// \brief 注册事件
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_long XObject::RegistEvent( xgc_long id, const xAction &invoke, xObject hOwner /*= INVALID_OBJECT_ID*/ )
	{
		auto &observer = mEventSubject[id];

		if( observer.count )
			return -1;

		observer.actions[++observer.token] = std::make_tuple( invoke, hOwner );

		return observer.token;
	}

	///
	/// \brief 删除事件
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::RemoveEvent( xgc_long id, xgc_long token )
	{
		auto it1 = mEventSubject.find( id );
		if( it1 == mEventSubject.end() )
			return;

		auto &observer = it1->second;
		// 防止在事件执行期间增删，导致迭代器失效
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
	/// \brief 提交事件
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::EmmitEvent( xgc_long id, XObjectEvent & evt )
	{
		evt.id = id;
		evt.over = false;
		evt.result = 0;
		evt.sender = GetObjectID();
		evt.target = INVALID_OBJECT_ID;

		EmmitEvent( evt );
	}

	///
	/// \brief 提交事件
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::EmmitEvent( XObjectEvent& evt )
	{
		auto it1 = mEventSubject.find( evt.id );
		if( it1 == mEventSubject.end() )
			return;

		auto &observer = it1->second;

		// 防止在触发器执行期间增删触发器
		++observer.count;
		for( auto &pair : observer.actions )
		{
			// 取token
			evt.token = pair.first;
			// 取目标对象ID
			evt.target = std::get< 1 >( pair.second );
			// 取触发函数，并执行
			std::get< 0 >( pair.second )( evt );

			if( evt.over )
				break;
		}

		// 对于未决的事件，向层级上级传递
		if( evt.result == 0 )
		{
			auto xParent = GetParent();
			if( xParent == INVALID_OBJECT_ID )
			{
				// 这里可以加入全局的事件通知
			}
			else
			{
				// 父对象存在的，则通知父对象
				auto pParent = ObjectCast< XObject >( xParent );
				if( pParent )
				{
					pParent->EmmitEvent( evt );
				}
			}
		}
		--observer.count;
	}

	xgc_bool LoadObject( XObject* pObj, xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize )
	{
		const XClassInfo &cls = pObj->GetRuntimeClass();

		attr_buffer ar( reference_buffer( lpData, uSize ) );

		auto nImplementCount = cls.GetImplementCount();
		auto pImplementInfo  = cls.GetImplementInfo();

		for( xgc_size i = 0, j = 0; i < nImplementCount; ++i )
		{
			if( uVersion < pImplementInfo[i].version.start ||
				uVersion >= pImplementInfo[i].version.close )
				continue;

			if( pImplementInfo[i].impl.flags == ATTR_FLAG_SAVE )
			{
				for( xgc_size n = 0; n < pImplementInfo[i].impl.count; ++n )
					ar >> pObj->getAttr( *pImplementInfo[i].impl.attr_ptr, n );
			}
			++j;
		}
		return true;
	}

	xgc_size SaveObject( XObject* pObj, xgc_uint32 uVersion, xgc_lpvoid lpData, xgc_size uSize )
	{
		const XClassInfo &cls = pObj->GetRuntimeClass();
		auto nAttributeCount = cls.GetAttributeCount();
		auto pAttributeInfo  = cls.GetAttributeInfo();

		attr_buffer ar( reference_buffer( lpData, uSize ) );

		for( xgc_size i = 0, j = 0; i < nAttributeCount; ++i )
		{
			if( pAttributeInfo[i].impl.flags == ATTR_FLAG_SAVE )
			{
				for( xgc_size n = 0; n < pAttributeInfo[i].impl.count; ++n )
					ar << pObj->getAttr( *pAttributeInfo[i].impl.attr_ptr, n );
			}
			++j;
		}

		return ar.wd();
	}
}