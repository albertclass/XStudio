#include "stdafx.h"
#include "XGoods.h"

namespace XGC
{
	extern "C"
	{
		CORE_API xAttrIndex	 attrDbIndex;	   // 数据库索引
		CORE_API xAttrIndex	 attrOperaotrCount;	   // 进出包裹次数
	}

	BEGIN_IMPLEMENT_XCLASS(XGoods, XObject, TypeXGoods)
		IMPLEMENT_ATTRIBUTE(DbIndex, VT_U32, ATTR_FLAG_NONE, "20140912")
		IMPLEMENT_ATTRIBUTE(OperaotrCount, VT_U32, ATTR_FLAG_NONE, "20140912")
	END_IMPLEMENT_XCLASS();

	XGoods::~XGoods()
	{
		m_pBaseGoods = xgc_nullptr;
	}

	/*xgc_bool XGoods::IsInheritFrom(xClassType clsType)
	{
		return true;
	}*/
}