#include "stdafx.h"
#include "XObject.h"
#include "XObjectComposition.h"

namespace XGC
{
	//////////////////////////////////////////////////////////////////////////
	// CXObject
	static XImplementInfo cls_XObject_Attribute[] = { xgc_nullptr, 0, VT_VOID, 0, { 0, 0 }, xgc_nullptr, xgc_nullptr, xgc_nullptr };
	const XClassInfo& XObject::GetThisClass()
	{
		static XClassInfo cls_XObject( TypeXObject, "XObject", cls_XObject_Attribute, xgc_nullptr );
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
		memset( mComposition, 0, sizeof( mComposition ) );
	}

	XObject::~XObject()
	{
		XGC_ASSERT_MESSAGE( mIsDestory, "对象被删除之前未销毁！(%p, 0X%08X)", this, handle()._handle );
		// 清理组件
		for( auto i = 0; i < XGC_COUNTOF( mComposition ); ++i )
		{
			SAFE_RELEASE( mComposition[i] );
		}
	}

	xgc_bool XObject::AddChild( xObject nID, xgc_lpcvoid lpContext /*= 0*/ )
	{
		XObject *pObj = GetXObject( nID );
		return AddChild( pObj, lpContext );
	}

	xgc_bool XObject::AddChild( XObject *pObj, xgc_lpcvoid lpContext /*= 0*/ )
	{
		XGC_ASSERT_RETURN( pObj, false );

		if( PreAddChild( pObj, lpContext ) )
		{
			XObject* pParent = GetXObject( pObj->GetParent() );
			if( pParent )
			{
				pParent->RemoveChild( pObj );
			}
			pObj->SetParent( handle()._handle );

			if( std::find( mChildList.begin(), mChildList.end(), pObj->GetObjectID() ) == mChildList.end() )
			{
				mChildList.push_back( pObj->GetObjectID() );
			}

			OnAddChild( pObj, lpContext );
			return true;
		}
		return false;
	}

	// 删除子对象ID
	xgc_void XObject::RemoveChild( xObject nID, xgc_bool bDestroy )
	{
		XObject* pObj = GetXObject( nID );
		return RemoveChild( pObj, bDestroy );
	}

	xgc_void XObject::RemoveChild( XObject* pObj, xgc_bool bDestroy /* = false */ )
	{
		XGC_ASSERT_RETURN( pObj, void( 0 ) );

		xObject nID = pObj->GetObjectID();
		xObjectList::iterator iter = std::find( mChildList.begin(), mChildList.end(), nID );
		if( iter != mChildList.end() && PreRemoveChild( pObj, bDestroy ) )
		{
			mChildList.erase( iter );

			OnRemoveChild( pObj, bDestroy );
			// 确认是自己的子物体，根据指示销毁对象
			if( pObj->GetParent() == GetObjectID() )
				pObj->SetParent( INVALID_OBJECT_ID );

			if( bDestroy )
				pObj->Destroy();
		}
	}

	xgc_bool XObject::QueryChild( xObject nID )const
	{
		return std::find( mChildList.begin(), mChildList.end(), nID ) != mChildList.end();
	}

	xgc_bool XObject::QueryChild( XObject* pObj )const
	{
		if( pObj )
			return QueryChild( pObj->GetObjectID() );

		return false;
	}

	xgc_bool XObject::QueryChild( const std::function< xgc_bool( xObject ) >& fnFilter )const
	{
		for( auto it : mChildList )
			if( fnFilter( it ) )
				return true;

		return false;
	}

	xgc_void XObject::Destroy()
	{
		FUNCTION_BEGIN;
		mIsDestory = true;
		if( GetXObject( handle()._handle ) != this )
			return;

		if( GetParent() != INVALID_OBJECT_ID )
		{
			XObject* pParent = GetXObject( GetParent() );
			if( pParent )
			{
				pParent->RemoveChild( this );
			}
		}
		DestroyAllChild();
		OnDestroy();
		FUNCTION_END;
	}

	void XObject::DestroyAllChild()
	{
		FUNCTION_BEGIN;
		while( !mChildList.empty() )
		{
			auto it = mChildList.begin();
			XObject* pObject = GetXObject( *it );

			pObject->Destroy();
			SAFE_DELETE( pObject );
		}
		FUNCTION_END;
	}

	xgc_bool XObject::SetComposition( xgc_uint16 eType, XObjectComposition* pComp )
	{
		FUNCTION_BEGIN;
		if( eType < XGC_COUNTOF( mComposition ) )
		{
			XGC_ASSERT_MESSAGE( xgc_nullptr == mComposition[eType], "重复设置组件 Type = %u", eType );
			mComposition[eType] = pComp;
			return true;
		}
		FUNCTION_END;

		return false;
	}

	XObjectComposition* XObject::GetComposition( xgc_uint16 eType )const
	{
		FUNCTION_BEGIN;
		XGC_ASSERT_RETURN( XGC_CHECK_ARRAY_INDEX( mComposition, eType ) && mComposition[eType], xgc_nullptr );
		return mComposition[eType];
		FUNCTION_END;
		return xgc_nullptr;
	}
}