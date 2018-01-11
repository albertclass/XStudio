#pragma once
#include "XMath.h"
#include "XVector2.h"

namespace xgc
{
	class CORE_API XVector3 
	{
	public:
		//! interal struct
		union
		{
			struct
			{
				float	x, y, z;
			};
			float _m[3];
		};

	public:
		//! construct functions
		XVector3();
		XVector3( const XVector2& v );
		XVector3( float fx, float fy, float fz );
		XVector3( const float m[3] );
		XVector3( const XVector3* const p );

		//! get component value functions
		float&	operator []	( const int idx );
		float	operator [] ( const int idx ) const;

		//! Convert operators.
					operator float* ();
					operator const float* () const;

					operator XVector2();
					operator const XVector2()const;
		//! logic operators, compare with epsilon.
		bool 		operator == ( const XVector3& rhs ) const;
		bool 		operator != ( const XVector3& rhs ) const;

		//! arithmetic operators
		XVector3 	operator + ( const XVector3& rhs ) const;
		XVector3 	operator - ( const XVector3& rhs ) const;
		XVector3 	operator * ( const float fScale ) const;
		XVector3 	operator / ( const float fScale ) const;

		XVector3& 	operator -= ( const XVector3& rhs );
		XVector3& 	operator += ( const XVector3& rhs );
		XVector3& 	operator *= ( float fScale );
		XVector3& 	operator /= ( float fScale );

		XVector3 	operator - () const;
		//! Dot pruduct
		float		operator * ( const XVector3& rhs ) const;

		//! Compare numeric, no epsilon.
		bool		NumericCompare( const XVector3& rhs ) const;

		//! Rotate by axis
		XVector3	RotateX( float fAngle ) const;
		XVector3	RotateY( float fAngle ) const;
		XVector3	RotateZ( float fAngle ) const;

		//! get vector length
		float		Length() const;

		//! get vector squared lenth, fast 
		float		SqurLength() const;

		//! CrossProduct
		XVector3	Cross( const XVector3& rhs ) const;

		//! normalize
		void 	Normalize();
		void 	NormalizeFast();

		//! Linear interpolation from v1 to v2. Vectors are expected to be normalized.
		XVector3&	Lerp( const XVector3& v1, const XVector3& v2, float t );

		//! Spherical linear interpolation from v1 to v2. Vectors are expected to be normalized.
		XVector3&	Slerp( const XVector3& v1, const XVector3& v2, float t );
		//! Set component value.
		XVector3&	SetValue( float x, float y, float z );

		void		MakeCeil( const XVector3& cmp );
		void		MakeFloor( const XVector3& cmp );

		float		DotProduct(const XVector3& other) const;

	public:
		static const XVector3	ZERO;
		static const XVector3	UNIT_X;
		static const XVector3	UNIT_Y;
		static const XVector3	UNIT_Z;
	};

	static XVector3 	Slerp( const XVector3& v1, const XVector3& v2, float t );
	static XVector3&	Lerp( XVector3& r, const XVector3& a, const XVector3& b, float t );
	static XVector3 	Lerp( const XVector3& v1, const XVector3& v2, float t );
	static XVector3 	operator * ( float fScale, const XVector3& rhs );
	static XVector3&	Vector3_Accumulate( XVector3& vOut, const XVector3& vIn, float fDelta );

	/*static XVector3&	operator=( float lsh[3], const XVector3& rsh );*/
	#include "XVector3.inl"
}