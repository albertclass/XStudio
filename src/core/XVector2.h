#pragma once
#ifndef __XVECTOR2_H__
#define __XVECTOR2_H__

#include "XMath.h"

namespace xgc
{
	class CORE_API XVector2
	{
	public:
		//! interal struct
		union{
			struct{
				float	x, y;
			};
			float _m[2];
		};

	public:
		//! construct functions
		XVector2();
		XVector2( float fx, float fy );
		XVector2( float m[2] );
		XVector2( const XVector2& rhs );
		XVector2( const XVector2* const p );

		//! Set component value.
		XVector2&	SetValue( float x, float y );

		//! get component value functions
		float&	operator []	( const int idx );
		float	operator [] ( const int idx ) const;

		//! Convert operators.
				operator float* ();
				operator const float* () const;

		//! assign operators
		XVector2&	operator = ( const XVector2& rhs );

		//! logic operators, compare with epsilon.
		bool 		operator == ( const XVector2& rhs ) const;
		bool 		operator != ( const XVector2& rhs ) const;

		//! arithmetic operators
		XVector2 	operator + ( const XVector2& rhs ) const;
		XVector2 	operator - ( const XVector2& rhs ) const;
		XVector2 	operator * ( const float fScale ) const;
		XVector2 	operator / ( const float fScale ) const;

		XVector2& 	operator -= ( const XVector2& rhs );
		XVector2& 	operator += ( const XVector2& rhs );
		XVector2& 	operator *= ( float fScale );
		XVector2& 	operator /= ( float fScale );

		XVector2 	operator - () const;
		//! Dot pruduct
		float		operator * ( const XVector2& rhs ) const;

		//! Compare numeric, no epsilon.
		bool		NumericCompare( const XVector2& rhs ) const;

		//! get vector length
		float		Length() const;

		//! get vector squared lenth, fast 
		float		SqurLength() const;

		//! normalize
		float 		Normalize();
		float 		NormalizeFast();

		/// Ðý×ªÏòÁ¿
		XVector2	Rotate( float fAngle ) const;

		//! Linear interpolation from v1 to v2. Vectors are expected to be normalized.
		XVector2&	Lerp( const XVector2& v1, const XVector2& v2, float t );

		//! Spherical linear interpolation from v1 to v2. Vectors are expected to be normalized.
		XVector2&	Slerp( const XVector2& v1, const XVector2& v2, float t );

		void		MakeCeil( const XVector2& cmp );
		void		MakeFloor( const XVector2& cmp );

	public:
		static const XVector2		ZERO;
		static const XVector2		UNIT_X;
		static const XVector2		UNIT_Y;

	};

	static XVector2 	Slerp( const XVector2& v1, const XVector2& v2, float t );
	static XVector2&	Lerp( XVector2& r, const XVector2& a, const XVector2& b, float t );
	static XVector2 	Lerp( const XVector2& v1, const XVector2& v2, float t );
	static XVector2 	operator * ( float fScale, const XVector2& rhs );
	#include "XVector2.inl"
}

#endif // __XVECTOR2_H__