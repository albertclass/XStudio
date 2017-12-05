#pragma once
#ifndef __XGOODS_H__
#define __XGOODS_H__

struct stBaseGoodsInfo;

namespace xgc
{
	extern CORE_API xAttrIndex	 attrGoodsIndex;	   // ����

	class CORE_API XGoods : public XObject
	{
		DECLARE_XCLASS();

	protected:
		XGoods();

		~XGoods();

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
		/// ��ӵ�����ǰ�Ķ���Ʒ��������� 
		/// [9/12/2014] create by wuhailin.jerry
		///
		virtual xgc_void BeforePutIntoBag() {}

		///
		///  ��ӵ�������Ķ���Ʒ���������
		/// [9/12/2014] create by wuhailin.jerry
		///
		virtual xgc_void AfterPutIntoBag() {}

		///
		/// ���ٶ���ʱ���� 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	};
}

#endif // __XGOODS_H__