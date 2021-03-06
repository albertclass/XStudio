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
			SYS_ERR( "malloc %u failed", cls.GetAttributeSize() );
			return false;
		}

		memset( mAttributes, 0, memsize( mAttributes ) );

		mAttributeInfo = cls.GetAttributeInfo();
		return true;
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
	/* 事件操作
	/************************************************************************/
	///
	/// \brief 注册事件
	/// \author albert.xu
	/// \date 2017/10/12
	/// \return 该事件的token，可以使用该token删除注册的事件
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
	/// \brief 屏蔽事件
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
	/// \brief 提交事件
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

		EmmitEvent( &evt, xgc_nullptr );
	}

	///
	/// \brief 提交事件
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

		EmmitEvent( &evt, xgc_nullptr );
	}

	///
	/// \brief 提交事件
	/// \author albert.xu
	/// \date 2017/10/12
	///
	xgc_void XObject::EmmitEvent( XObjectEvent* evt, xgc_void( *DeleteIt )( XObjectEvent* ) )
	{
		auto notify_all = [evt]( Observer& o ){
			// 关闭消息通知
			if( o.close == false )
			{
				// 防止在触发器执行期间增删触发器
				++o.count;
				for( auto &pair : o.actions )
				{
					// 取token
					evt->token = pair.first;
					// 取目标对象ID
					evt->target = std::get< 1 >( pair.second );
					// 取触发函数，并执行
					std::get< 0 >( pair.second )( *evt );

					if( evt->over < 0 )
						break;
				}
				--o.count;
			}
		};

		auto it1 = mEventSubject.find( evt->id );
		if( it1 != mEventSubject.end() )
		{
			// 先通知本地的观察者
			notify_all( it1->second );
		}

		// 透传消息， id 为 -1 的观察者监听该对象的所有消息
		auto it2 = mEventSubject.find( -1 );
		if( it2 != mEventSubject.end() )
		{
			// 再通知监视全部消息的观察者
			notify_all( it2->second );
		}

		if( evt->over > 0 && XGC_CHK_FLAGS( evt->over, 1 ) )
		{
			// 对于未决的事件，且允许向上传递的，则向上级传递
			auto xParent = GetParent();
			if( xParent != INVALID_OBJECT_ID )
			{
				// 父对象存在的，则通知父对象
				auto pParent = ObjectCast< XObject >( xParent );
				if( pParent )
				{
					pParent->EmmitEvent( evt );
				}
			}
			//else
			//{
			//	// 这里可以加入全局的事件通知
			//}
		}

		//不再支持向下传递消息 [4/3/2018 albert.xu]
		//if( evt->over > 0 && XGC_CHK_FLAGS( evt->over, 2 ) )
		//{
		//	// 对于未决的事件，且允许向下传递的，则向下级传递给子对象
		//	
		//	// 枚举出所有的子对象，并将事件推送给子对象
		//	Search( [evt]( xObject hObject )->xgc_bool{
		//		auto pChild = ObjectCast< XObject >( hObject );
		//		if( pChild )
		//			pChild->EmmitEvent( evt );

		//		return false;
		//	} );
		//}

		// 有清理需求的，则调用清理的回调。
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
	/// 获取属性
	/// 该函数提供一个不调用属性设置Hook的方法，其他设置和取值函数皆调用Hook。
	///
	XAttribute XObject::getAttr( xAttrIndex nAttr ) const
	{
		XGC_ASSERT_THROW( isAttrValid( nAttr ), std::logic_error( "get attribute, index out of bound." ) );

		// 获取属性信息
		auto info = mAttributeInfo[nAttr];

		// 生成属性操作对象
		return XAttribute( info->impl.type, (xgc_lpstr)mAttributes + info->offset );
	}
}