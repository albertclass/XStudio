#pragma once
#ifndef __XOBJECT_NODE_H__
#define __XOBJECT_NODE_H__
namespace xgc
{
	class CORE_API XObjectNode : public XObject
	{
		DECLARE_XCLASS();
	protected:
		XObjectNode()
		{

		}

		~XObjectNode()
		{

		}

	public:
		///
		/// \brief 增加子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_bool Insert( XObject *pObject, xgc_lpvoid pContext );

		///
		/// \brief 移除子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_bool Remove( XObject *pObject );

		///
		/// \brief 移除子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_void RemoveAll( const XClassInfo *pClass = xgc_nullptr );

		///
		/// \brief 删除子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_bool Delete( XObject *pObject );

		///
		/// \brief 删除所有子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_void DeleteAll( const XClassInfo *pClass = xgc_nullptr );

		///
		/// \brief 过滤子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xObject Search( const std::function< xgc_bool( xObject ) > &filter, const XClassInfo *pClass = xgc_nullptr )const;

		///
		/// \brief 获取子对象数量
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_size GetChildrenCount( const XClassInfo *pClass = xgc_nullptr )const;

		///
		/// \brief 获取组件
		/// \author albert.xu
		/// \date 2017/11/01
		///
		xObject GetComposition( const XClassInfo &rClass, xgc_size nIndex = 0 )const
		{
			auto it = mChildren.find( &rClass );
			if( it == mChildren.end() )
				return INVALID_OBJECT_ID;

			if( it->second.size() <= nIndex )
				return INVALID_OBJECT_ID;

			return it->second[nIndex];
		}

		///
		/// \brief 获取组件
		/// \author albert.xu
		/// \date 2017/11/01
		///
		template< template< class, class > class Continer, class _Ax = xgc_allocator >
		typename Continer< xObject, _Ax > GetCompositions( const XClassInfo &rClass )const
		{
			Continer< xObject, _Ax > c;

			auto it = mChildren.find( &rClass );
			if( it != mChildren.end() )
			{
				std::copy( it->second.begin(), it->second.end(), std::back_inserter( c ) );
			}

			return c;
		}

	protected:
		/************************************************************************/
		/* 设置的一些事件响应虚函数。
		/************************************************************************/
		///
		/// \brief 增加子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		virtual xgc_bool InsertChild( xObject hObject );

		///
		/// \brief 删除子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		virtual xgc_bool RemoveChild( xObject hObject );

		///
		/// \brief 增加子节点后调用
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		virtual xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) = 0;

		/////
		/// 删除子节点后调用,此时对象尚未被删除
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild ) = 0;

		///
		/// \brief 销毁对象
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		virtual xgc_void OnDestroy() override;

	private:
		/// @var children
		xgc_unordered_map< xgc_lpcvoid, xgc_vector< xObject > > mChildren;
	};
}

#endif // __XOBJECT_NODE_H__