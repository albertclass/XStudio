#pragma once
#ifndef _XGOODS_H_
#define _XGOODS_H_

#include "XObject.h"

#ifdef CORE_EXPORTS
#	define CORE_API __declspec(dllexport)
#else
#	define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS


struct stBaseGoodsInfo;

namespace XGC
{
	extern CORE_API xAttrIndex	 attrDbIndex;	       // DB索引
	extern CORE_API xAttrIndex	 attrOperaotrCount;	   // 进出包裹次数


	class CORE_API XGoods;
	typedef std::shared_ptr<XGoods> xGoodsPtr;
	class CORE_API XGoods : public XObject, public std::enable_shared_from_this< XGoods >
	{
		DECLARE_XCLASS();

	protected:
		XGoods() : XObject()
		{

		}

		virtual ~XGoods();

	public:
		// ------------------------------------------------ //
		// [10/3/2014 wuhailin.jerry]
		// 是否是相同的物品
		// ------------------------------------------------ //
		virtual xgc_bool IsSameGoods(xGoodsPtr pShGoods) = 0;

		///
		/// 比较物品,排序时使用，在GS层实现 
		/// [8/4/2014] create by wuhailin.jerry
		///
		virtual xgc_int16 CompareGoods( xGoodsPtr pShGoods ) = 0;

		///
		/// 物品最大堆叠个数 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xgc_uint32 GetOverlapNum() = 0;

		///
		/// 拷贝函数 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xGoodsPtr Copy( ) = 0;

		///
		/// 制造者 
		/// [1/20/2015] create by wuhailin.jerry
		///
		virtual void SetMakeUser(const xgc_char *pUserName)
		{
			
		}

		///
		/// 增加进背包次数 
		/// [9/2/2014] create by wuhailin.jerry
		///
		xgc_void AddOperatorCount()
		{
			SetUnsignedAttr( attrOperaotrCount, GetUnsignedAttr( attrOperaotrCount ) + 1 );
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

		///
		/// 获取物品额外属性 
		/// [10/14/2014] create by wuhailin.jerry
		///
		const stBaseGoodsInfo *GetBaseGoodsInfo() const { return m_pBaseGoods; }

	public:
		const stBaseGoodsInfo *m_pBaseGoods = xgc_nullptr; // 物品的基础属性 只可以拿到基础的不变化的属性

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
		virtual xgc_bool PreRemoveChild(XObject* pChild, xgc_bool bRelease) { return true; }

		/////
		/// 删除子节点后调用,此时对象尚未被删除
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnRemoveChild(XObject* pChild, xgc_bool bRelease) { }

		///
		/// 销毁对象时调用 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	};
}

#endif

