#pragma once
#ifndef __XGOODS_H__
#define __XGOODS_H__

struct stBaseGoodsInfo;

namespace xgc
{
	extern CORE_API xAttrIndex	 attrGoodsIndex;	   // ����
	extern CORE_API xAttrIndex	 attrOperaotrCount;	   // ������������

	class CORE_API XGoods : public XObject
	{
		DECLARE_XCLASS();

	protected:
		XGoods();

		virtual ~XGoods();

	public:
		///
		/// �Ƚ���Ʒ,����ʱʹ�ã���GS��ʵ�� 
		/// [8/4/2014] create by wuhailin.jerry
		///
		virtual xgc_bool Compare( xObject hObject ) = 0;

		///
		/// ��Ʒ���ѵ����� 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xgc_uint32 GetOverlapNum() = 0;

		///
		/// �������� 
		/// [9/2/2014] create by wuhailin.jerry
		///
		virtual xObject Copy() = 0;

		///
		/// ���ӽ��������� 
		/// [9/2/2014] create by wuhailin.jerry
		///
		xgc_void AddOperatorCount()
		{
			incValue<xgc_uint32>( attrOperaotrCount, 1 );
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
		virtual xgc_bool PreDelChild(XObject* pChild, xgc_bool bRelease) { return true; }

		/////
		/// ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnDelChild(XObject* pChild, xgc_bool bRelease) { }

		///
		/// ���ٶ���ʱ���� 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	};
}

#endif // __XGOODS_H__