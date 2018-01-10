#include "XHeader.h"
#include "XSkill.h"

namespace xgc
{
	xAttrIndex XSkill::Index;
	xAttrIndex XSkill::Level;
	xAttrIndex XSkill::StartTime;
	xAttrIndex XSkill::CloseTime;
	xAttrIndex XSkill::ResetTime;

	IMPLEMENT_XCLASS_BEGIN( XSkill, XTimeline )
		IMPLEMENT_ATTRIBUTE( Index, VT_U32, ATTR_FLAG_NONE, "20171023" )
		IMPLEMENT_ATTRIBUTE( Level, VT_U32, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( StartTime, VT_U64, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( CloseTime, VT_U64, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( ResetTime, VT_U64, ATTR_FLAG_SAVE, "20171023" )
	IMPLEMENT_XCLASS_END();

	XSkill::XSkill( timespan interval )
		: XTimeline( interval )
	{

	}

	XSkill::~XSkill()
	{

	}
}