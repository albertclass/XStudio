
#define LERP_DELTA 1e-6

	//----------------------------------------------------------------------------
	__inline XVector2::XVector2()
		: x(0.f)
		, y(0.f)
	{
	}
	//----------------------------------------------------------------------------
	__inline XVector2::XVector2( float fx, float fy )
		: x(fx)
		, y(fy)
	{
	}
	//----------------------------------------------------------------------------
	__inline XVector2::XVector2( float val[2] )
		: x(val[0])
		, y(val[1])
	{
	}
	//----------------------------------------------------------------------------
	__inline XVector2::XVector2( const XVector2& rhs )
		: x(rhs._m[0])
		, y(rhs._m[1])
	{
	}
	//----------------------------------------------------------------------------
	__inline XVector2::XVector2( const XVector2* const p )
		: x(p->_m[0])
		, y(p->_m[1])
	{
	}

	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::SetValue( float x, float y )
	{
		this->x = x; this->y = y;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline float XVector2::operator [] ( int idx ) const
	{
		XGC_ASSERT( idx < 2 );
		return _m[idx];
	}
	//----------------------------------------------------------------------------
	__inline float& XVector2::operator []( const int idx )
	{
		XGC_ASSERT( idx < 2 );
		return _m[idx];
	}
	//----------------------------------------------------------------------------
	__inline XVector2::operator float* ()
	{
		return _m;
	}
	//----------------------------------------------------------------------------
	__inline XVector2::operator const float* () const 
	{
		return _m;
	}
	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::operator = ( const XVector2& rhs )
	{
		x = rhs.x;	y = rhs.y;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline bool XVector2::operator == ( const XVector2& rhs ) const
	{
		if ( XMath::FAbs( x - rhs.x ) > XMath::F_EPSILON ) return false;
		if ( XMath::FAbs( y - rhs.y ) > XMath::F_EPSILON ) return false;
		return true;
	}
	//----------------------------------------------------------------------------
	__inline bool XVector2::operator != ( const XVector2& rhs ) const
	{
		return !( *this == rhs );
	}
	//----------------------------------------------------------------------------
	__inline XVector2 XVector2::operator + ( const XVector2& rhs ) const
	{
		return XVector2( x + rhs.x, y + rhs.y );
	}
	//----------------------------------------------------------------------------
	__inline XVector2 XVector2::operator - ( const XVector2& rhs ) const
	{
		return XVector2( x - rhs.x, y - rhs.y );
	}
	//----------------------------------------------------------------------------
	__inline float XVector2::operator * ( const XVector2& rhs ) const
	{
		return x * rhs.x + y * rhs.y;
	}
	//----------------------------------------------------------------------------
	__inline XVector2 XVector2::operator * ( const float fScale ) const
	{
		return XVector2( x * fScale, y * fScale );
	}
	//----------------------------------------------------------------------------
	__inline XVector2 XVector2::operator / ( const float fScale ) const
	{
		return XVector2( x / fScale, y / fScale );
	}
	//----------------------------------------------------------------------------
	__inline XVector2 XVector2::operator - () const
	{
		return XVector2( -x, -y );
	}
	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::operator -= ( const XVector2& rhs )
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::operator += ( const XVector2& rhs )
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::operator *= ( float fScale )
	{
		x *= fScale;
		y *= fScale;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::operator /= ( float fScale )
	{
		x /= fScale;
		y /= fScale;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline bool XVector2::NumericCompare( const XVector2& rhs ) const
	{
		return ( x == rhs.x && y == rhs.y );
	}
	//----------------------------------------------------------------------------
	__inline float XVector2::Length() const
	{
		return XMath::Sqrt( x*x + y*y );
	}
	//----------------------------------------------------------------------------
	__inline float XVector2::SqurLength() const
	{
		return ( x*x + y*y );
	}
	//----------------------------------------------------------------------------
	__inline float XVector2::Normalize()
	{
		float fSqrLen = x*x + y*y;
		float fInvSqrt = XMath::InvSqrt( fSqrLen );
		x *= fInvSqrt;
		y *= fInvSqrt;
		return fInvSqrt * fSqrLen;
	}
	//----------------------------------------------------------------------------
	__inline float XVector2::NormalizeFast()
	{
		float fSqrLen = x*x + y*y;
		float fInvSqrt = XMath::FastInvSqrt( fSqrLen );
		x *= fInvSqrt;
		y *= fInvSqrt;
		return fInvSqrt * fSqrLen;
	}
	//----------------------------------------------------------------------------
	__inline XVector2 XVector2::Rotate( xgc_real32 fAngle ) const
	{
		float _x = x * XMath::Cos( fAngle ) - y * XMath::Sin( fAngle );
		float _y = x * XMath::Sin( fAngle ) + y * XMath::Cos( fAngle );
		return XVector2( _x, _y );
	}

	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::Lerp( const XVector2& v1, const XVector2& v2, float t )
	{
		if ( t <= 0.0f ) {
			return *this = v1;
		} else if ( t >= 1.0f ) {
			return *this = v2;
		}

		return *this = v1 + ( v2 - v1 ) * t;
	}
	//----------------------------------------------------------------------------
	__inline XVector2& XVector2::Slerp( const XVector2& v1, const XVector2& v2, float t )
	{
		// From Doom3 SDK source code.
		float omega, cosom, sinom, scale0, scale1;

		if ( t <= 0.0f ) {
			return *this = v1;
		} else if ( t >= 1.0f ) {
			return *this = v2;
		}

		cosom = v1 * v2;
		if ( ( 1.0f - cosom ) > LERP_DELTA ) {
			omega = acos( cosom );
			sinom = sin( omega );
			scale0 = sin( ( 1.0f - t ) * omega ) / sinom;
			scale1 = sin( t * omega ) / sinom;
		} else {
			scale0 = 1.0f - t;
			scale1 = t;
		}

		*this = ( v1 * scale0 + v2 * scale1 );
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline void XVector2::MakeCeil( const XVector2& cmp )
	{
		if( cmp.x > x ) x = cmp.x;
		if( cmp.y > y ) y = cmp.y;
	}
	//----------------------------------------------------------------------------
	__inline void XVector2::MakeFloor( const XVector2& cmp )
	{
		if( cmp.x < x ) x = cmp.x;
		if( cmp.y < y ) y = cmp.y;
	}
	//----------------------------------------------------------------------------
	__inline XVector2 Lerp( const XVector2& v1, const XVector2& v2, float t )
	{
		if ( t <= 0.0f ) {
			return XVector2( v1 );
		} else if ( t >= 1.0f ) {
			return XVector2( v2 );
		}

		return XVector2( v1 + ( v2 - v1 ) * t );
	}
	//----------------------------------------------------------------------------
	__inline XVector2& Lerp( XVector2& r, const XVector2& a, const XVector2& b, float t )
	{
		return r.Lerp( a, b, t );
	}
	//----------------------------------------------------------------------------
	__inline XVector2 Slerp( const XVector2& v1, const XVector2& v2, float t )
	{
		// From Doom3 SDK source code.
		float omega, cosom, sinom, scale0, scale1;

		if ( t <= 0.0f ) {
			return XVector2( v1 );
		} else if ( t >= 1.0f ) {
			return XVector2( v2 );
		}

		cosom = v1 * v2;
		if ( ( 1.0f - cosom ) > LERP_DELTA ) {
			omega = acos( cosom );
			sinom = sin( omega );
			scale0 = sin( ( 1.0f - t ) * omega ) / sinom;
			scale1 = sin( t * omega ) / sinom;
		} else {
			scale0 = 1.0f - t;
			scale1 = t;
		}

		return XVector2( ( v1 * scale0 + v2 * scale1 ) );
	}
	//----------------------------------------------------------------------------
	__inline XVector2 operator * ( float fScale, const XVector2& rhs )
	{
		return rhs * fScale;
	}