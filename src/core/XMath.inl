
	//----------------------------------------------------------------------------
	__inline float XMath::ACos (float fValue)
	{
		if ( -1.f < fValue )
		{
			if ( fValue < 1.f )
				return acos(fValue);
			else
				return 0.f;
		}
		else
		{
			return ONE_PI;
		}
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::ASin (float fValue)
	{
		if ( -1.f < fValue )
		{
			if ( fValue < 1.f )
				return asin(fValue);
			else
				return HALF_PI;
		}
		else
		{
			return -HALF_PI;
		}
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::ATan (float fValue)
	{
		return atan(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::ATan2 (float fY, float fX)
	{
		return atan2(fY,fX);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Ceil (float fValue)
	{
		return ceil(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Cos (float fValue)
	{
		return cos(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Exp (float fValue)
	{
		return exp(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FAbs (float fValue)
	{
		int tmp = *reinterpret_cast<int *>( &fValue );
		tmp &= 0x7FFFFFFF;
		return *reinterpret_cast<float *>( &tmp );
	}
	//----------------------------------------------------------------------------
	__inline 	double XMath::FAbs (double fValue)
	{
		return fabs(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Floor (float fValue)
	{
		return floor(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FMod (float fX, float fY)
	{
		return fmod(fX,fY);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::InvSqrt (float fValue)
	{
		if ( fValue == 1.f )
			return 1.f;

		else if ( fValue <= 1.e-8 )
			return 0.f;

		static const __m128 _05 = _mm_set1_ps( 0.5f );
		static const __m128 _3 = _mm_set1_ps( 3.f );

		__m128 a = _mm_load_ss( &fValue );
		__m128 rsqrt = _mm_rsqrt_ss( a );

		rsqrt = _mm_mul_ss( 
			_mm_mul_ss( _05, rsqrt ),
			_mm_sub_ss( _3, _mm_mul_ss( a, _mm_mul_ss( rsqrt, rsqrt ) ) ) );

		return rsqrt.m128_f32[0];
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Log (float fValue)
	{
		return log(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Pow (float fBase, float fExponent)
	{
		return pow(fBase,fExponent);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Sin (float fValue)
	{
		return sin(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Sqr (float fValue)
	{
		return fValue*fValue;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Sqrt (float fValue)
	{
		if ( fValue == 1.f )
			return 1.f;

		else if ( fValue <= 1.e-8 )
			return 0.f;

		//return _mm_sqrt_ss( _mm_load_ss( &fValue ) ).m128_f32[0];
		return sqrt(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Tan (float fValue)
	{
		return tan(fValue);
	}
	//----------------------------------------------------------------------------
	__inline 	int XMath::Sign (int iValue)
	{
		if ( iValue > 0 )
			return 1;

		if ( iValue < 0 )
			return -1;

		return 0;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::Sign (float fValue)
	{
		if ( fValue > 0.0 )
			return 1.0;

		if ( fValue < 0.0 )
			return -1.0;

		return 0.0;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::UnitRandom (unsigned int uiSeed )
	{
		if ( uiSeed > 0 )
			srand(uiSeed);

		float dRatio = rand()*1.0f/RAND_MAX;
		return dRatio;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::SymmetricRandom (unsigned int uiSeed)
	{
		if ( uiSeed > 0.0 )
			srand(uiSeed);

		float dRatio = rand()*1.0f/RAND_MAX;
		return (2.0f*dRatio - 1.0f);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::IntervalRandom (float fMin, float fMax, unsigned int uiSeed)
	{
		if ( uiSeed > 0 )
			srand(uiSeed);

		float dRatio = rand()*1.0f/RAND_MAX;
		return fMin+(fMax-fMin)*(dRatio);
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastSin0 (float fAngle)
	{
		float fASqr = fAngle*fAngle;
		float fResult = (float)7.61e-03;
		fResult *= fASqr;
		fResult -= (float)1.6605e-01;
		fResult *= fASqr;
		fResult += 1.0;
		fResult *= fAngle;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastSin1 (float fAngle)
	{
		float fASqr = fAngle*fAngle;
		float fResult = (float)-2.39e-08;
		fResult *= fASqr;
		fResult += (float)2.7526e-06;
		fResult *= fASqr;
		fResult -= (float)1.98409e-04;
		fResult *= fASqr;
		fResult += (float)8.3333315e-03;
		fResult *= fASqr;
		fResult -= (float)1.666666664e-01;
		fResult *= fASqr;
		fResult += 1.0;
		fResult *= fAngle;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastCos0 (float fAngle)
	{
		float fASqr = fAngle*fAngle;
		float fResult = (float)3.705e-02;
		fResult *= fASqr;
		fResult -= (float)4.967e-01;
		fResult *= fASqr;
		fResult += 1.0;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastCos1 (float fAngle)
	{
		float fASqr = fAngle*fAngle;
		float fResult = (float)-2.605e-07;
		fResult *= fASqr;
		fResult += (float)2.47609e-05;
		fResult *= fASqr;
		fResult -= (float)1.3888397e-03;
		fResult *= fASqr;
		fResult += (float)4.16666418e-02;
		fResult *= fASqr;
		fResult -= (float)4.999999963e-01;
		fResult *= fASqr;
		fResult += 1.0;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastTan0 (float fAngle)
	{
		float fASqr = fAngle*fAngle;
		float fResult = (float)2.033e-01;
		fResult *= fASqr;
		fResult += (float)3.1755e-01;
		fResult *= fASqr;
		fResult += 1.0;
		fResult *= fAngle;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastTan1 (float fAngle)
	{
		float fASqr = fAngle*fAngle;
		float fResult = (float)9.5168091e-03;
		fResult *= fASqr;
		fResult += (float)2.900525e-03;
		fResult *= fASqr;
		fResult += (float)2.45650893e-02;
		fResult *= fASqr;
		fResult += (float)5.33740603e-02;
		fResult *= fASqr;
		fResult += (float)1.333923995e-01;
		fResult *= fASqr;
		fResult += (float)3.333314036e-01;
		fResult *= fASqr;
		fResult += 1.0;
		fResult *= fAngle;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvSin0 (float fValue)
	{
		float fRoot = XMath::Sqrt((1.0f)-fValue);
		float fResult = -0.0187293f;
		fResult *= fValue;
		fResult += 0.0742610f;
		fResult *= fValue;
		fResult -= 0.2121144f;
		fResult *= fValue;
		fResult += 1.5707288f;
		fResult = HALF_PI - fRoot*fResult;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvSin1 (float fValue)
	{
		float fRoot = XMath::Sqrt(FAbs((1.0f)-fValue));
		float fResult = -0.0012624911f;
		fResult *= fValue;
		fResult += 0.0066700901f;
		fResult *= fValue;
		fResult -= 0.0170881256f;
		fResult *= fValue;
		fResult += 0.0308918810f;
		fResult *= fValue;
		fResult -= 0.0501743046f;
		fResult *= fValue;
		fResult += 0.0889789874f;
		fResult *= fValue;
		fResult -= 0.2145988016f;
		fResult *= fValue;
		fResult += 1.5707963050f;
		fResult = HALF_PI - fRoot*fResult;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvCos0 (float fValue)
	{
		float fRoot = XMath::Sqrt((1.0f)-fValue);
		float fResult = -0.0187293f;
		fResult *= fValue;
		fResult += 0.0742610f;
		fResult *= fValue;
		fResult -= 0.2121144f;
		fResult *= fValue;
		fResult += 1.5707288f;
		fResult *= fRoot;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvCos1 (float fValue)
	{
		float fRoot = XMath::Sqrt(FAbs((1.0f)-fValue));
		float fResult = -0.0012624911f;
		fResult *= fValue;
		fResult += 0.0066700901f;
		fResult *= fValue;
		fResult -= 0.0170881256f;
		fResult *= fValue;
		fResult += 0.0308918810f;
		fResult *= fValue;
		fResult -= 0.0501743046f;
		fResult *= fValue;
		fResult += 0.0889789874f;
		fResult *= fValue;
		fResult -= 0.2145988016f;
		fResult *= fValue;
		fResult += 1.5707963050f;
		fResult *= fRoot;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvTan0 (float fValue)
	{
		float fVSqr = fValue*fValue;
		float fResult = 0.0208351f;
		fResult *= fVSqr;
		fResult -= 0.085133f;
		fResult *= fVSqr;
		fResult += 0.180141f;
		fResult *= fVSqr;
		fResult -= 0.3302995f;
		fResult *= fVSqr;
		fResult += 0.999866f;
		fResult *= fValue;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvTan1 (float fValue)
	{
		float fVSqr = fValue*fValue;
		float fResult = 0.0028662257f;
		fResult *= fVSqr;
		fResult -= 0.0161657367f;
		fResult *= fVSqr;
		fResult += 0.0429096138f;
		fResult *= fVSqr;
		fResult -= 0.0752896400f;
		fResult *= fVSqr;
		fResult += 0.1065626393f;
		fResult *= fVSqr;
		fResult -= 0.1420889944f;
		fResult *= fVSqr;
		fResult += 0.1999355085f;
		fResult *= fVSqr;
		fResult -= 0.3333314528f;
		fResult *= fVSqr;
		fResult += 1.0;
		fResult *= fValue;
		return fResult;
	}
	//----------------------------------------------------------------------------
	__inline 	float XMath::FastInvSqrt (float fValue)
	{
		float fHalf = 0.5f*fValue;
		int i  = *(int*)&fValue;
		i = 0x5f3759df - (i >> 1);
		fValue = *(float*)&i;
		fValue = fValue*(1.5f - fHalf*fValue*fValue);
		return fValue;
	}
	//----------------------------------------------------------------------------
	__inline double XMath::FastInvSqrt (double dValue)
	{
		double dHalf = 0.5*dValue;
		_int64 i  = *(_int64*)&dValue;
#if defined(DE_USING_VC70) || defined(DE_USING_VC6)
		i = 0x5fe6ec85e7de30da - (i >> 1);
#else
		i = 0x5fe6ec85e7de30daLL - (i >> 1);
#endif
		dValue = *(double*)&i;
		dValue = dValue*(1.5 - dHalf*dValue*dValue);
		return dValue;
	}
	//----------------------------------------------------------------------------
	__inline void XMath::SinCos( float a, float &s, float &c )
	{
#ifdef _M_IX86
		_asm {
			fld	a
			fsincos
			mov		ecx, c
			mov		edx, s
			fstp	dword ptr [ecx]
			fstp	dword ptr [edx]
		}
#else
		s = sinf( a );
		c = cosf( a );
#endif
	}
	//----------------------------------------------------------------------------
	__inline float XMath::ATan16( float y, float x )
	{
		float a, s;

		if ( fabs( y ) > fabs( x ) ) {
			a = x / y;
			s = a * a;
			s = - ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
				* s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
			if ( FLOATSIGNBITSET( a ) ) {
				return s - HALF_PI;
			} else {
				return s + HALF_PI;
			}
		} else {
			a = y / x;
			s = a * a;
			return ( ( ( ( ( ( ( ( ( 0.0028662257f * s - 0.0161657367f ) * s + 0.0429096138f ) * s - 0.0752896400f )
				* s + 0.1065626393f ) * s - 0.1420889944f ) * s + 0.1999355085f ) * s - 0.3333314528f ) * s ) + 1.0f ) * a;
		}
	}
	//----------------------------------------------------------------------------
	__inline float XMath::Sin16( float a )
	{
		float s;

		if ( ( a < 0.0f ) || ( a >= TWO_PI ) ) {
			a -= floorf( a / TWO_PI ) * TWO_PI;
		}
		if ( a < ONE_PI ) {
			if ( a > HALF_PI ) {
				a = ONE_PI - a;
			}
		} else {
			if ( a > ONE_PI + HALF_PI ) {
				a = a - TWO_PI;
			} else {
				a = ONE_PI - a;
			}
		}
		s = a * a;
		return a * ( ( ( ( ( -2.39e-08f * s + 2.7526e-06f ) * s - 1.98409e-04f ) * s + 8.3333315e-03f ) * s - 1.666666664e-01f ) * s + 1.0f );
	}
	//----------------------------------------------------------------------------
	__inline float	XMath::Random(float min, float max)
	{
		if(min >= max)
			return min;
		return ((float)rand()/(float)RAND_MAX)*(max - min) + min;
	}
	//----------------------------------------------------------------------------
	__inline float	Lerp( float a, float b, float t )
	{
		if ( t <= 0.0f ) 
		{
			return a;
		} 
		else if ( t >= 1.0f ) 
		{
			return b;
		}

		float scale0 = 1.0f - t;
		float scale1 = t;

		return a * scale0 + b * scale1;
	}
	//----------------------------------------------------------------------------
	__inline float&	Lerp( float& r, float a, float b, float t )
	{
		if ( t <= 0.0f ) 
		{
			r = a;
			return r;
		} 
		else if ( t >= 1.0f ) 
		{
			r = b;
			return r;
		}

		float scale0 = 1.0f - t;
		float scale1 = t;

		r = a * scale0 + b * scale1;
		return r;
	}
