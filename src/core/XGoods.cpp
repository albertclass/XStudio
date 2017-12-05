#include "XHeader.h"
#include "XGoods.h"

namespace xgc
{
	CORE_API xAttrIndex	 attrGoodsIndex;	   // Êý¾Ý¿âË÷Òý

	IMPLEMENT_XCLASS_BEGIN(XGoods, XObject )
		IMPLEMENT_ATTRIBUTE(GoodsIndex, VT_U32, ATTR_FLAG_NONE, "20140912")
	IMPLEMENT_XCLASS_END();

	XGoods::XGoods()
		: XObject()
	{

	}

	XGoods::~XGoods()
	{
	}
}