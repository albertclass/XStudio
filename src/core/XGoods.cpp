#include "XHeader.h"
#include "XGoods.h"

namespace xgc
{
	xAttrIndex XGoods::Index;	///< ��Ʒ��������
	xAttrIndex XGoods::Sequence;///< ��Ʒȫ������
	xAttrIndex XGoods::Alias;	///< ��Ʒ����

	IMPLEMENT_XCLASS_BEGIN(XGoods, XObject )
		IMPLEMENT_ATTRIBUTE( Index, VT_U32, ATTR_FLAG_NONE, "20140912")
		IMPLEMENT_ATTRIBUTE( Sequence,  VT_U64, ATTR_FLAG_NONE, "20171226" )
		IMPLEMENT_ATTRIBUTE( Alias, VT_STRING, ATTR_FLAG_NONE, "20171226" )
	IMPLEMENT_XCLASS_END();

	XGoods::XGoods()
		: XObject()
	{

	}

	XGoods::~XGoods()
	{
	}
}