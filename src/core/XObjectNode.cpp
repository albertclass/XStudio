#include "XHeader.h"
#include "XObject.h"
#include "XObjectNode.h"

namespace xgc
{
	///
	/// \brief 节点构造 
	/// \date 11/13/2017
	/// \author xufeng04
	///
	XObjectNode::XObjectNode( xObject hParent )
		: XObject()
	{
		SetParent( hParent );
	}

	///
	/// \brief 节点析构
	/// \date 11/13/2017
	/// \author xufeng04
	///

	XObjectNode::~XObjectNode()
	{
	}

	///
	/// \brief 增加子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///
	xgc_bool XObjectNode::Insert( XObject * pObject, xgc_lpvoid pContext )
	{
		if( !InsertChild( pObject->GetObjectID() ) )
			return false;

		auto hParent = pObject->GetParent();
		if( INVALID_OBJECT_ID != hParent )
		{
			auto pParent = ObjectCast< XObjectNode >( hParent );
			if( pParent )
				pParent->RemoveChild( pObject->GetObjectID() );
		}

		pObject->SetParent( GetObjectID() );

		OnInsertChild( pObject, pContext );

		return true;
	}

	///
	/// \brief 移除子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///

	xgc_bool XObjectNode::Remove( XObject * pObject )
	{
		XGC_ASSERT_RETURN( pObject, false );

		if( !RemoveChild( pObject->GetObjectID() ) )
			return false;

		OnRemoveChild( pObject );
		return true;
	}

	///
	/// \brief 移除子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///

	xgc_void XObjectNode::RemoveAll()
	{
		for( auto it = mChildren.rbegin(); it != mChildren.rend(); ++it )
			Remove( ObjectCast< XObject >( *it ) );
	}

	///
	/// \brief 删除子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///

	xgc_bool XObjectNode::Delete( XObject * pObject )
	{
		if( pObject )
		{
			if( pObject->GetParent() != GetObjectID() )
				return false;

			if( !Remove( pObject ) )
				return false;

			pObject->Destroy();
		}

		return true;
	}

	///
	/// \brief 删除所有子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///

	xgc_void XObjectNode::DeleteAll()
	{
		for( auto it = mChildren.rbegin(); it != mChildren.rend(); ++it )
			Delete( ObjectCast< XObject >( *it ) );
	}

	///
	/// \brief 过滤子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///

	xObject XObjectNode::Search( const std::function<xgc_bool( xObject )>& filter ) const
	{
		for( auto hObject : mChildren )
			if( filter( hObject ) )
				return hObject;

		return INVALID_OBJECT_ID;
	}

	///
	/// \brief 获取子对象数量
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///
	xgc_size XObjectNode::GetChildCount() const
	{
		return mChildren.size();
	}

	///
	/// \brief 销毁对象
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///

	xgc_void XObjectNode::OnDestroy()
	{
		auto hParent = GetParent();
		if( hParent != INVALID_OBJECT_ID )
		{
			auto pParent = ObjectCast< XObjectNode >( hParent );
			if( pParent )
				pParent->Remove( this );
		}

		DeleteAll();
	}

	///
	/// \brief 增加子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///
	xgc_bool XObjectNode::InsertChild( xObject hObject )
	{
		auto pObject = ObjectCast<XObject>( hObject );
		if( xgc_nullptr == pObject )
			return false;

		// 此处可以使用堆来优化
		mChildren.push_back( hObject );

		return true;
	}

	///
	/// \brief 删除子节点
	/// \author albert.xu
	/// \date 8/3/2009
	/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
	///
	xgc_bool XObjectNode::RemoveChild( xObject hObject )
	{
		auto it = std::find( mChildren.begin(), mChildren.end(), hObject );
		if( it == mChildren.end() )
			return false;

		mChildren.erase( it );

		auto pObject = ObjectCast<XObject>( hObject );
		if( pObject )
			pObject->SetParent( INVALID_OBJECT_ID );

		return true;
	}
}