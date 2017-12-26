#include "XHeader.h"
#include "XGoods.h"

namespace xgc
{
	xAttrIndex XGoods::Index;	///< 物品配置索引
	xAttrIndex XGoods::Sequence;///< 物品全局索引
	xAttrIndex XGoods::Alias;	///< 物品别名

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