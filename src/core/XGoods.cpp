#include "stdafx.h"
#include "XGoods.h"

namespace XGC
{
	extern "C"
	{
		CORE_API xAttrIndex	 attrDbIndex;	   // ���ݿ�����
		CORE_API xAttrIndex	 attrOperaotrCount;	   // ������������
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