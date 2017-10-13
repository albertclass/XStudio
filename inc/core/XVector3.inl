
	//----------------------------------------------------------------------------
	__inline XVector3::XVector3()
	{
		x = y = z = 0.f;
	}
	//----------------------------------------------------------------------------
	__inline XVector3::XVector3( const XVector2& v )
	{
		x = v.x;
		y = v.y;
		z = 0.f;
	}
	//----------------------------------------------------------------------------
	__inline XVector3::XVector3( float fx, float fy, float fz )
	{
		x = fx; y = fy; z = fz; 
	}

	//----------------------------------------------------------------------------
	__inline XVector3::XVector3( const float m[3] )
	{
		_m[0] = m[0];
		_m[1] = m[1];
		_m[2] = m[2];
	}

	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::SetValue( float x, float y, float z )
	{
		this->x = x; this->y = y; this->z = z;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline float XVector3::operator [] ( const int idx ) const
	{
		return _m[idx];
	}
	//----------------------------------------------------------------------------
	__inline float& XVector3::operator []( const int idx )
	{
		return _m[idx];
	}
	//----------------------------------------------------------------------------
	__inline XVector3::operator float* ()
	{
		return _m;
	}
	//----------------------------------------------------------------------------
	__inline XVector3::operator const float* () const 
	{
		return _m;
	}
	__inline XVector3::operator XVector2()
	{
		return XVector2( _m[0], _m[1] );
	}
	__inline XVector3::operator const XVector2()const
	{
		return XVector2( _m[0], _m[1] );
	}

	//----------------------------------------------------------------------------
	__inline bool XVector3::operator == ( const XVector3& rhs ) const
	{
		if ( XMath::FAbs( x - rhs.x ) > XMath::F_EPSILON ) return false;
		if ( XMath::FAbs( y - rhs.y ) > XMath::F_EPSILON ) return false;
		if ( XMath::FAbs( z - rhs.z ) > XMath::F_EPSILON ) return false;
		return true;
	}
	//----------------------------------------------------------------------------
	__inline bool XVector3::operator != ( const XVector3& rhs ) const
	{
		return !( *this == rhs );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::operator + ( const XVector3& rhs ) const
	{
		return XVector3( x + rhs.x, y + rhs.y, z + rhs.z );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::operator - ( const XVector3& rhs ) const
	{
		return XVector3( x - rhs.x, y - rhs.y, z - rhs.z );
	}
	//----------------------------------------------------------------------------
	__inline float XVector3::operator * ( const XVector3& rhs ) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::operator * ( const float fScale ) const
	{
		return XVector3( x * fScale, y * fScale, z * fScale );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::operator / ( const float fScale ) const
	{
		return *this * ( 1.f / fScale );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::operator - () const
	{
		return XVector3( -x, -y, -z );
	}
	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::operator -= ( const XVector3& rhs )
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::operator += ( const XVector3& rhs )
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::operator *= ( float fScale )
	{
		x *= fScale;
		y *= fScale;
		z *= fScale;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::operator /= ( float fScale )
	{
		return *this *= ( 1.f / fScale );
	}
	//----------------------------------------------------------------------------
	__inline bool XVector3::NumericCompare( const XVector3& rhs ) const
	{
		return !( x != rhs.x || y != rhs.y || z != rhs.z );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::RotateX( float fAngle ) const
	{
		float _y = y * XMath::Cos( fAngle ) - z * XMath::Sin( fAngle );
		float _z = y * XMath::Sin( fAngle ) + z * XMath::Cos( fAngle );

		return XVector3( x, _y, _z );
	}

	__inline XVector3 XVector3::RotateY( float fAngle ) const
	{
		float _z = z * XMath::Cos( fAngle ) - x * XMath::Sin( fAngle );
		float _x = z * XMath::Sin( fAngle ) + x * XMath::Cos( fAngle );

		return XVector3( _x, y, _z );
	}

	__inline XVector3 XVector3::RotateZ( float fAngle ) const
	{
		float _x = x * XMath::Cos( fAngle ) - y * XMath::Sin( fAngle );
		float _y = x * XMath::Sin( fAngle ) + y * XMath::Cos( fAngle );

		return XVector3( _x, _y, z );
	}
	//----------------------------------------------------------------------------
	__inline float XVector3::Length() const
	{
		return XMath::Sqrt( x*x + y*y + z*z );
	}
	//----------------------------------------------------------------------------
	__inline float XVector3::SqurLength() const
	{
		return ( x*x + y*y + z*z );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 XVector3::Cross( const XVector3& rhs ) const
	{
		return XVector3( y * rhs.z - z * rhs.y,
					   z * rhs.x - x * rhs.z,
					   x * rhs.y - y * rhs.x );
	}
	//----------------------------------------------------------------------------
	__inline void XVector3::Normalize()
	{
		float fSqrLen = x*x + y*y + z*z;
		float fInvSqrt = XMath::InvSqrt( fSqrLen );
		x *= fInvSqrt;
		y *= fInvSqrt;
		z *= fInvSqrt;
	}
	//----------------------------------------------------------------------------
	__inline void XVector3::NormalizeFast()
	{
		float fSqrLen = x*x + y*y + z*z;
		float fInvSqrt = XMath::FastInvSqrt( fSqrLen );
		x *= fInvSqrt;
		y *= fInvSqrt;
		z *= fInvSqrt;
	}
	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::Lerp( const XVector3& v1, const XVector3& v2, float t )
	{
		if ( t <= 0.0f ) {
			return *this = v1;
		} else if ( t >= 1.0f ) {
			return *this = v2;
		}

		x = v1.x + ( v2.x - v1.x ) * t;
		y = v1.y + ( v2.y - v1.y ) * t;
		z = v1.z + ( v2.z - v1.z ) * t;
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline XVector3& XVector3::Slerp( const XVector3& v1, const XVector3& v2, float t )
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
			sinom = 1.0f/sin( omega );
			scale0 = sin( ( 1.0f - t ) * omega ) * sinom;
			scale1 = sin( t * omega ) * sinom;
		} else {
			scale0 = 1.0f - t;
			scale1 = t;
		}

		*this = ( v1 * scale0 + v2 * scale1 );
		return *this;
	}
	//----------------------------------------------------------------------------
	__inline void XVector3::MakeCeil( const XVector3& cmp )
	{
		if( cmp.x > x ) x = cmp.x;
		if( cmp.y > y ) y = cmp.y;
		if( cmp.z > z ) z = cmp.z;
	}
	//----------------------------------------------------------------------------
	__inline void XVector3::MakeFloor( const XVector3& cmp )
	{
		if( cmp.x < x ) x = cmp.x;
		if( cmp.y < y ) y = cmp.y;
		if( cmp.z < z ) z = cmp.z;
	}

	__inline float XVector3::DotProduct(const XVector3& other) const
	{
		return x*other.x + y*other.y + z*other.z;
	}

	//----------------------------------------------------------------------------
	__inline XVector3&	Lerp( XVector3& r, const XVector3& a, const XVector3& b, float t )
	{
		return r.Lerp( a, b, t );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 Lerp( const XVector3& v1, const XVector3& v2, float t )
	{
		if ( t <= 0.0f ) {
			return v1;
		} else if ( t >= 1.0f ) {
			return v2;
		}

		return XVector3( v1 + ( v2 - v1 ) * t );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 Slerp( const XVector3& v1, const XVector3& v2, float t )
	{
		// From Doom3 SDK source code.
		float omega, cosom, sinom, scale0, scale1;

		if ( t <= 0.0f ) {
			return XVector3( v1 );
		} else if ( t >= 1.0f ) {
			return XVector3( v2 );
		}

		cosom = v1 * v2;
		if ( ( 1.0f - cosom ) > LERP_DELTA ) {
			omega = acos( cosom );
			sinom = 1.0f/sin( omega );
			scale0 = sin( ( 1.0f - t ) * omega ) * sinom;
			scale1 = sin( t * omega ) * sinom;
		} else {
			scale0 = 1.0f - t;
			scale1 = t;
		}

		return XVector3( ( v1 * scale0 + v2 * scale1 ) );
	}
	//----------------------------------------------------------------------------
	__inline XVector3 operator * ( float fScale, const XVector3& rhs )
	{
		return rhs * fScale;
	}
	//----------------------------------------------------------------------------
	__inline XVector3& Vector3_Accumulate( XVector3& vOut, const XVector3& vIn, float fDelta )
	{
		vOut.x += vIn.x * fDelta;
		vOut.y += vIn.y * fDelta;
		vOut.z += vIn.z * fDelta;
		return vOut;
	}

	//__inline XVector3&	operator=( float lsh[3], const XVector3& rsh )
	//{
	//	lsh[0] = rsh[0];
	//	lsh[1] = rsh[1];
	//	lsh[2] = rsh[2];
	//}