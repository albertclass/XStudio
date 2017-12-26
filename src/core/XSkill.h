#pragma once
#ifndef __XSKILL_H__
#define __XSKILL_H__
#include "XTimeline.h"

namespace xgc
{
	class XSkill : public XTimeline
	{
		DECLARE_XCLASS()
		static xAttrIndex Index;
		static xAttrIndex Level;
		static xAttrIndex StartTime;
		static xAttrIndex CloseTime;
		static xAttrIndex ResetTime;

	public:
		XSkill( timespan interval );

		~XSkill();

		xgc_bool Cast( xgc_uint32 nDuration, xgc_uint32 nReset )
		{
			xgc_time64 nStart = datetime::now().to_milliseconds();

			setValue( StartTime, nStart );
			setValue( CloseTime, nStart + nDuration );
			setValue( ResetTime, nStart + nReset );

			Start( datetime::now() );
		}
	};
}

#endif // __XSKILL_H__