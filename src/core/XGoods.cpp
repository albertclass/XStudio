#include "XHeader.h"
#include "XGoods.h"

namespace xgc
{
	CORE_API xAttrIndex	 attrGoodsIndex;	   // 数据库索引
	CORE_API xAttrIndex	 attrOperaotrCount;	   // 进出包裹次数

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