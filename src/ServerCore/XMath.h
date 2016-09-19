#pragma once
#include <math.h>
#include <xmmintrin.h>

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS

namespace XGC
{
	#define FLOATSIGNBITSET(f)		((*(const unsigned long *)&(f)) >> 31)
	#define DEG2RAD(a)				( (a) * XMath::M_DEG2RAD )
	#define RAD2DEG(a)				( (a) * XMath::M_RAD2DEG )

	namespace XMath
	{
		// common constants
		CORE_API  extern const float			ONE_PI;						// pi
		CORE_API  extern const float			TWO_PI;						// pi * 2
		CORE_API  extern const float			HALF_PI;					// pi / 2
		CORE_API  extern const float 			INV_PI;
		CORE_API  extern const float 			INV_TWO_PI;
		CORE_API  extern const float			ONEFOURTH_PI;				// pi / 4
		CORE_API  extern const float			E;							// e
		CORE_API  extern const float			SQRT_TWO;					// sqrt( 2 )
		CORE_API  extern const float			SQRT_THREE;					// sqrt( 3 )
		CORE_API  extern const float			SQRT_1OVER2;				// sqrt( 1 / 2 )
		CORE_API  extern const float			SQRT_1OVER3;				// sqrt( 1 / 3 )
		CORE_API  extern const float			M_DEG2RAD;					// degrees to radians multiplier
		CORE_API  extern const float			M_RAD2DEG;					// radians to degrees multiplier
		CORE_API  extern const float			M_SEC2MS;					// seconds to milliseconds multiplier
		CORE_API  extern const float			M_MS2SEC;					// milliseconds to seconds multiplier

		CORE_API  extern const float			F_EPSILON;
		CORE_API  extern const double			D_EPSILON;
		CORE_API  extern const float 			ZERO_TOLERANCE;
		CORE_API  extern const float 			MAX_REAL;
		CORE_API  extern const double			MAX_DOUBLE;
		CORE_API  extern const xgc_int32			MAX_INTEGER;
		CORE_API  extern const xgc_int32			MIN_INTEGER;

		// Wrappers to hide implementations of functions.  The ACos and ASin
		// functions clamp the input argument to [-1,1] to avoid NaN issues
		// when the input is slightly larger than 1 or slightly smaller than -1.
		// Other functions have the potential for using a fast and approximate
		// algorithm rather than calling the standard math library functions.
		CORE_API __inline float 	ACos (float fValue);
		CORE_API __inline float 	ASin (float fValue);
		CORE_API __inline float 	ATan (float fValue);
		CORE_API __inline float 	ATan2 (float fY, float fX);
		CORE_API __inline float 	Ceil (float fValue);
		CORE_API __inline float 	Cos (float fValue);
		CORE_API __inline float 	Exp (float fValue);
		CORE_API __inline float 	FAbs (float fValue);
		CORE_API __inline double 	FAbs (double fValue);
		CORE_API __inline float 	Floor (float fValue);
		CORE_API __inline float 	FMod (float fX, float fY);
		CORE_API __inline float 	InvSqrt (float fValue);
		CORE_API __inline float 	Log (float fValue);
		CORE_API __inline float 	Pow (float fBase, float fExponent);
		CORE_API __inline float 	Sin (float fValue);
		CORE_API __inline float 	Sqr (float fValue);
		CORE_API __inline float 	Sqrt (float fValue);
		CORE_API __inline float 	Tan (float fValue);
		CORE_API __inline float	Random(float min, float max);

		// Return -1 if the input is negative, 0 if the input is zero, and +1
		// if the input is positive.
		CORE_API __inline xgc_int32	Sign (xgc_int32 iValue);
		CORE_API __inline float	Sign (float fValue);

		// Generate a random number in [0,1).  The random number generator may
		// be seeded by a first call to UnitRandom with a positive seed.
		CORE_API __inline float	UnitRandom (xgc_uint32 uiSeed = 0);

		// Generate a random number in [-1,1).  The random number generator may
		// be seeded by a first call to SymmetricRandom with a positive seed.
		CORE_API __inline float	SymmetricRandom (xgc_uint32 uiSeed = 0);

		// Generate a random number in [min,max).  The random number generator may
		// be seeded by a first call to IntervalRandom with a positive seed.
		CORE_API __inline float	IntervalRandom (float fMin, float fMax,	xgc_uint32 uiSeed = 0);

		// Fast evaluation of trigonometric and inverse trigonometric functions
		// using polynomial approximations.  The speed ups were measured on an
		// AMD 2800 (2.08 GHz) processor using Visual Studion .NET 2003 with a
		// release build.

		// The input must be in [0,pi/2].
		// max error sin0 = 1.7e-04, speed up = 4.0
		// max error sin1 = 1.9e-08, speed up = 2.8
		CORE_API __inline float 	FastSin0 (float fAngle);
		CORE_API __inline float 	FastSin1 (float fAngle);

		// The input must be in [0,pi/2]
		// max error cos0 = 1.2e-03, speed up = 4.5
		// max error cos1 = 6.5e-09, speed up = 2.8
		CORE_API __inline float 	FastCos0 (float fAngle);
		CORE_API __inline float 	FastCos1 (float fAngle);

		// The input must be in [0,pi/4].
		// max error tan0 = 8.1e-04, speed up = 5.6
		// max error tan1 = 1.9e-08, speed up = 3.4
		CORE_API __inline float 	FastTan0 (float fAngle);
		CORE_API __inline float 	FastTan1 (float fAngle);

		// The input must be in [0,1].
		// max error invsin0 = 6.8e-05, speed up = 7.5
		// max error invsin1 = 1.4e-07, speed up = 5.5
		CORE_API __inline float	FastInvSin0 (float fValue);
		CORE_API __inline float	FastInvSin1 (float fValue);

		// The input must be in [0,1].
		// max error invcos0 = 6.8e-05, speed up = 7.5
		// max error invcos1 = 1.4e-07, speed up = 5.7
		CORE_API __inline float 	FastInvCos0 (float fValue);
		CORE_API __inline float 	FastInvCos1 (float fValue);

		// The input must be in [-1,1]. 
		// max error invtan0 = 1.2e-05, speed up = 2.8
		// max error invtan1 = 2.3e-08, speed up = 1.8
		CORE_API __inline float 	FastInvTan0 (float fValue);
		CORE_API __inline float 	FastInvTan1 (float fValue);

		// A fast approximation to 1/sqrt.
		CORE_API __inline float	FastInvSqrt (float fValue);
		CORE_API __inline double FastInvSqrt (double dValue);

		//! sine and cosine with 32 bits precision
		CORE_API __inline void SinCos( float a, float &s, float &c );

		//! arc tangent with 16 bits precision, maximum absolute error is 1.3593e-08
		CORE_API __inline float ATan16( float y, float x );

		//! sine with 16 bits precision, maximum absolute error is 2.3082e-09
		CORE_API __inline float Sin16( float a );

		template < typename T >
		T Clamp( const T& t, const T& min, const T& max )
		{
			return __max( min, __min( t, max ) );
		}
	};

    CORE_API __inline float		Lerp( float a, float b, float t );
	CORE_API __inline float&	Lerp( float& r, float a, float b, float t );

	// Fast conversion from a IEEE 32-bit floating point number F in [0,1] to a
	// a 32-bit integer I in [0,2^L-1].
	//
	//   fFloat = F
	//   iLog = L
	//   iInt = I
	#define DE_SCALED_FLOAT_TO_INT(fFloat,iLog,iInt)\
	{ \
		xgc_int32 iShift = 150 - iLog - ((*(xgc_int32*)(&fFloat) >> 23) & 0xFF); \
		if (iShift < 24) \
		{ \
			iInt = ((*(xgc_int32*)(&fFloat) & 0x007FFFFF) | \
				0x00800000) >> iShift; \
			if (iInt == (1 << iLog)) \
			{ \
				iInt--; \
			} \
		} \
		else \
		{ \
			iInt = 0; \
		} \
	}
#include "XMath.inl"
}