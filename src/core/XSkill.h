#pragma once
#ifndef __XSKILL_H__
#define __XSKILL_H__
#include "XTimeline.h"

namespace xgc
{
	extern CORE_API xAttrIndex attrSkillIndex;
	extern CORE_API xAttrIndex attrSkillId;
	extern CORE_API xAttrIndex attrSkillLevel;
	extern CORE_API xAttrIndex attrSkillStartTime;
	extern CORE_API xAttrIndex attrSkillCloseTime;
	extern CORE_API xAttrIndex attrSkillResetTime;
	
	class XSkill : public XTimeline
	{
		DECLARE_XCLASS()

	public:
		XSkill();

		~XSkill();

		xgc_bool Cast( xgc_uint32 nDuration, xgc_uint32 nReset )
		{
			xgc_time64 nStart = datetime::now().to_milliseconds();

			setValue( attrSkillStartTime, nStart );
			setValue( attrSkillCloseTime, nStart + nDuration );
			setValue( attrSkillResetTime, nStart + nReset );

			Start();
		}
	};
}

#endif // __XSKILL_H__