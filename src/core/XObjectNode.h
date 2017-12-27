#pragma once
#ifndef __XOBJECT_NODE_H__
#define __XOBJECT_NODE_H__
namespace xgc
{
	class CORE_API XObjectNode : public XObject
	{
	public:
		///
		/// \brief 节点构造 
		/// \date 11/13/2017
		/// \author xufeng04
		///
		XObjectNode( xObject hParent = INVALID_OBJECT_ID );

		///
		/// \brief 节点析构
		/// \date 11/13/2017
		/// \author xufeng04
		///
		~XObjectNode();

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
		xgc_void RemoveAll();

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
		xgc_void DeleteAll();

		///
		/// \brief 获取子对象数量
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xgc_size GetChildCount()const override;

		///
		/// \brief 搜索子节点
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		///
		xObject Search( const std::function< xgc_bool( xObject ) > &Filter )const override;

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
		///
		virtual xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) = 0;

		///
		/// \brief 删除子节点后调用
		/// \author albert.xu
		/// \date 8/3/2009
		///
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
		xgc::vector< xObject > mChildren;
	};
}

#endif // __XOBJECT_NODE_H__