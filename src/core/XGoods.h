#pragma once
#ifndef __XGOODS_H__
#define __XGOODS_H__

struct stBaseGoodsInfo;

namespace xgc
{
	extern CORE_API xAttrIndex	 attrGoodsIndex;	   // 索引
	extern CORE_API xAttrIndex	 attrOperaotrCount;	   // 进出包裹次数

	class CORE_API XGoods : public XObject
	{
		DECLARE_XCLASS();

	protected:
		XGoods();

		virtual ~XGoods();

	public:
		///
		/// 比较物品,排序时使用，在GS层实现 
		/// [8/4/2014] create by wuhailin.jerry
		///
		virtual xgc_bool Compare( xObject hObject ) = 0;

		///
		/// 物品最大堆叠个数 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xgc_uint32 GetOverlapNum() = 0;

		///
		/// 拷贝函数 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xObject Copy() = 0;

		///
		/// 增加进背包次数 
		/// [9/2/2014] create by wuhailin.jerry
		///
		xgc_void AddOperatorCount()
		{
			incValue<xgc_uint32>( attrOperaotrCount, 1 );
		}

		///
		/// 添加到背包前的对物品的特殊操作 
		/// [9/12/2014] create by wuhailin.jerry
		///
		virtual xgc_void BeforePutIntoBag() { ; }

		///
		///  添加到背包后的对物品的特殊操作
		/// [9/12/2014] create by wuhailin.jerry
		///
		virtual xgc_void AfterPutIntoBag() { ; }

	private:
		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	增加子节点前调用
		// return :	true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		//---------------------------------------------------//
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	加入子节点后调用
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { }

		/////
		/// 增加子节点前调用
		/// [8/3/2009 Albert]
		/// \return true - 确认移除子节点, false - 子节点被否决,移除子节点失败.
		/////
		virtual xgc_bool PreDelChild(XObject* pChild, xgc_bool bRelease) { return true; }

		/////
		/// 删除子节点后调用,此时对象尚未被删除
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnDelChild(XObject* pChild, xgc_bool bRelease) { }

		///
		/// 销毁对象时调用 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	};
}

#endif // __XGOODS_H__