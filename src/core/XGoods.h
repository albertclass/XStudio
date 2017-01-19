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
	extern CORE_API xAttrIndex	 attrDbIndex;	       // DB����
	extern CORE_API xAttrIndex	 attrOperaotrCount;	   // ������������


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
		// �Ƿ�����ͬ����Ʒ
		// ------------------------------------------------ //
		virtual xgc_bool IsSameGoods(xGoodsPtr pShGoods) = 0;

		///
		/// �Ƚ���Ʒ,����ʱʹ�ã���GS��ʵ�� 
		/// [8/4/2014] create by wuhailin.jerry
		///
		virtual xgc_int16 CompareGoods( xGoodsPtr pShGoods ) = 0;

		///
		/// ��Ʒ���ѵ����� 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xgc_uint32 GetOverlapNum() = 0;

		///
		/// �������� 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xGoodsPtr Copy( ) = 0;

		///
		/// ������ 
		/// [1/20/2015] create by wuhailin.jerry
		///
		virtual void SetMakeUser(const xgc_char *pUserName)
		{
			
		}

		///
		/// ���ӽ��������� 
		/// [9/2/2014] create by wuhailin.jerry
		///
		xgc_void AddOperatorCount()
		{
			SetUnsignedAttr( attrOperaotrCount, GetUnsignedAttr( attrOperaotrCount ) + 1 );
		}

		///
		/// ��ӵ�����ǰ�Ķ���Ʒ��������� 
		/// [9/12/2014] create by wuhailin.jerry
		///
		virtual xgc_void BeforePutIntoBag() { ; }

		///
		///  ��ӵ�������Ķ���Ʒ���������
		/// [9/12/2014] create by wuhailin.jerry
		///
		virtual xgc_void AfterPutIntoBag() { ; }

		///
		/// ��ȡ��Ʒ�������� 
		/// [10/14/2014] create by wuhailin.jerry
		///
		const stBaseGoodsInfo *GetBaseGoodsInfo() const { return m_pBaseGoods; }

	public:
		const stBaseGoodsInfo *m_pBaseGoods = xgc_nullptr; // ��Ʒ�Ļ������� ֻ�����õ������Ĳ��仯������

	private:
		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�ǰ����
		// return :	true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		//---------------------------------------------------//
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		//---------------------------------------------------//
		// [8/3/2009 Albert]
		// Description:	�����ӽڵ�����
		//---------------------------------------------------//
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { }

		/////
		/// �����ӽڵ�ǰ����
		/// [8/3/2009 Albert]
		/// \return true - ȷ���Ƴ��ӽڵ�, false - �ӽڵ㱻���,�Ƴ��ӽڵ�ʧ��.
		/////
		virtual xgc_bool PreRemoveChild(XObject* pChild, xgc_bool bRelease) { return true; }

		/////
		/// ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnRemoveChild(XObject* pChild, xgc_bool bRelease) { }

		///
		/// ���ٶ���ʱ���� 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	};
}

#endif

