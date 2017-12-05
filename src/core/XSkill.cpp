#include "XHeader.h"
#include "XSkill.h"

namespace xgc
{
	extern CORE_API xAttrIndex attrSkillIndex;
	extern CORE_API xAttrIndex attrSkillId;
	extern CORE_API xAttrIndex attrSkillLevel;
	extern CORE_API xAttrIndex attrSkillStartTime;
	extern CORE_API xAttrIndex attrSkillCloseTime;
	extern CORE_API xAttrIndex attrSkillResetTime;

	IMPLEMENT_XCLASS_BEGIN( XSkill, XTimeline )
		IMPLEMENT_ATTRIBUTE( SkillIndex, VT_U32, ATTR_FLAG_NONE, "20171023" )
		IMPLEMENT_ATTRIBUTE( SkillId, VT_U32, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( SkillLevel, VT_U32, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( SkillStartTime, VT_U64, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( SkillCloseTime, VT_U64, ATTR_FLAG_SAVE, "20171023" )
		IMPLEMENT_ATTRIBUTE( SkillResetTime, VT_U64, ATTR_FLAG_SAVE, "20171023" )
	IMPLEMENT_XCLASS_END()

	XSkill::XSkill( timespan interval )
		: XTimeline( interval )
	{

	}

	XSkill::~XSkill()
	{

	}
}