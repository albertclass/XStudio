#include "XHeader.h"
#include "XGoods.h"

namespace xgc
{
	CORE_API xAttrIndex	 attrGoodsIndex;	   // ���ݿ�����
	CORE_API xAttrIndex	 attrOperaotrCount;	   // ������������

	IMPLEMENT_XCLASS_BEGIN(XGoods, XObject, TypeXGoods)
		IMPLEMENT_ATTRIBUTE(GoodsIndex, VT_U32, ATTR_FLAG_NONE, "20140912")
		IMPLEMENT_ATTRIBUTE(OperaotrCount, VT_U32, ATTR_FLAG_NONE, "20140912")
	IMPLEMENT_XCLASS_END();

	XGoods::XGoods()
		: XObject()
	{

	}

	XGoods::~XGoods()
	{
		m_pBaseGoods = xgc_nullptr;
	}
}