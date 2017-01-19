#include "StdAfx.h"
#include <float.h>
#include "XVector2.h"
#include "XVector3.h"
namespace XGC
{
	namespace XMath
	{
		const float		ONE_PI			= 3.14159265358979323846f;
		const float		TWO_PI			= 2.0f * ONE_PI;
		const float		HALF_PI			= 0.5f * ONE_PI;
		const float		ONEFOURTH_PI	= 0.25f * ONE_PI;
		const float 	INV_PI			= 1.0f / ONE_PI;
		const float 	INV_TWO_PI		= 1.0f / TWO_PI;
		const float		E				= 2.71828182845904523536f;
		const float		SQRT_TWO		= 1.41421356237309504880f;
		const float		SQRT_THREE		= 1.73205080756887729352f;
		const float		SQRT_1OVER2		= 0.70710678118654752440f;
		const float		SQRT_1OVER3		= 0.57735026918962576450f;
		const float		M_DEG2RAD		= ONE_PI / 180.0f;
		const float		M_RAD2DEG		= 180.0f / ONE_PI;
		const float		M_SEC2MS		= 1000.0f;
		const float		M_MS2SEC		= 0.001f;

		const float		F_EPSILON		= FLT_EPSILON;
		const double	D_EPSILON		= DBL_EPSILON;
		const float		ZERO_TOLERANCE	= 1e-06f;
		const float		MAX_REAL		= FLT_MAX;
		const double	MAX_DOUBLE		= DBL_MAX;

		const xgc_int32		MAX_INTEGER		= INT_MAX;
		const xgc_int32		MIN_INTEGER		= INT_MIN;

		//const DeMatrix2 DeMatrix2::ZERO( 0.f, 0.f, 0.f, 0.f );
		//const DeMatrix2 DeMatrix2::IDENTITY( 1.f, 0.f, 1.f, 0.f );

		//const DeMatrix3 DeMatrix3::ZERO( 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f );
		//const DeMatrix3 DeMatrix3::IDENTITY( 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f );

		//const DeMatrix4 DeMatrix4::ZERO( 0.f, 0.f, 0.f, 0.f,
		//	0.f, 0.f, 0.f, 0.f,
		//	0.f, 0.f, 0.f, 0.f,
		//	0.f, 0.f, 0.f, 0.f );

		//const DeMatrix4 DeMatrix4::IDENTITY( 1.f, 0.f, 0.f, 0.f,
		//	0.f, 1.f, 0.f, 0.f,
		//	0.f, 0.f, 1.f, 0.f,
		//	0.f, 0.f, 0.f, 1.f );

		//const DeMatrix4x3 DeMatrix4x3::ZERO( 0.f, 0.f, 0.f, 0.f,
		//	0.f, 0.f, 0.f, 0.f,
		//	0.f, 0.f, 0.f, 0.f );

		//const DeMatrix4x3 DeMatrix4x3::IDENTITY( 1.f, 0.f, 0.f, 0.f,
		//	0.f, 1.f, 0.f, 0.f,
		//	0.f, 0.f, 1.f, 0.f );

		//const DeQuat DeQuat::IDENTITY( 0.f, 0.f, 0.f, 1.f );

	};
	const XVector3 XVector3::ZERO	( 0.0f, 0.0f, 0.0f );
	const XVector3 XVector3::UNIT_X ( 1.0f, 0.0f, 0.0f );
	const XVector3 XVector3::UNIT_Y	( 0.0f, 1.0f, 0.0f );
	const XVector3 XVector3::UNIT_Z	( 0.0f, 0.0f, 1.0f );

	//const DeVec4 DeVec4::ZERO	( 0.0f, 0.0f, 0.0f, 0.0f );
	//const DeVec4 DeVec4::UNIT_X ( 1.0f, 0.0f, 0.0f, 0.0f );
	//const DeVec4 DeVec4::UNIT_Y	( 0.0f, 1.0f, 0.0f, 0.0f );
	//const DeVec4 DeVec4::UNIT_Z	( 0.0f, 0.0f, 1.0f, 0.0f );
	//const DeVec4 DeVec4::UNIT_W	( 0.0f, 0.0f, 0.0f, 1.0f );

	const XVector2 XVector2::ZERO	( 0.0f, 0.0f );
	const XVector2 XVector2::UNIT_X ( 1.0f, 0.0f );
	const XVector2 XVector2::UNIT_Y	( 0.0f, 1.0f );
};
