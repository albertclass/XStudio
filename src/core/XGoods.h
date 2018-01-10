#pragma once
#ifndef __XGOODS_H__
#define __XGOODS_H__

struct stBaseGoodsInfo;

namespace xgc
{
	class CORE_API XGoods : public XObject
	{
		DECLARE_XCLASS();
		static xAttrIndex Index;	///< ��Ʒ��������
		static xAttrIndex Sequence;	///< ��Ʒȫ������
		static xAttrIndex Alias;	///< ��Ʒ����

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
	};
}

#endif // __XGOODS_H__