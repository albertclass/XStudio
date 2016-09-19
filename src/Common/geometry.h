#pragma once
#ifndef _XGEOMETRY_H_
#define _XGEOMETRY_H_
#include "defines.h"

namespace xgc
{
	template< class type > class xSize;
	template< class type > class xPoint;
	template< class type > class xRect;

	/////////////////////////////////////////////////////////////////////////////
	// xgcSize<type> - An extent, similar to Windows xgcSize<type> structure.
	template< class type >
	class xSize
	{
	public:
		type cx;
		type cy;

		// Constructors
		// construct an uninitialized size
		xSize<type>() throw();
		// create from two integers
		xSize<type>(type initCX, type initCY) throw();
		// create from another size
		xSize<type>(const xSize<type> &initSize) throw();
		// create from a point
		xSize<type>(const xPoint<type> &initPt) throw();

		// Operations
		bool operator==(xSize<type> size) const throw();
		bool operator!=(xSize<type> size) const throw();
		void operator+=(xSize<type> size) throw();
		void operator-=(xSize<type> size) throw();
		void SetSize(type cx, type cy) throw();

		// Operators returning xgcSize<type> values
		xSize<type> operator+(xSize<type> size) const throw();
		xSize<type> operator-(xSize<type> size) const throw();
		xSize<type> operator-() const throw();

		// Operators returning xgcPoint<type> values
		xPoint<type> operator+(xPoint<type> point) const throw();
		xPoint<type> operator-(xPoint<type> point) const throw();

		// Operators returning xgcRect<type> values
		xRect<type> operator+(const xRect<type>* lpRect) const throw();
		xRect<type> operator-(const xRect<type>* lpRect) const throw();
	};

	/////////////////////////////////////////////////////////////////////////////
	// xgcPoint<type> - A 2-D point, similar to Windows xgcPoint<type> structure.
	template< class type >
	class xPoint
	{
	public:
		// Constructors
		type x, y;
		// create an uninitialized point
		xPoint<type>() throw();
		// create from two integers
		xPoint<type>(type initX, type initY) throw();
		// create from another point
		xPoint<type>(const xPoint<type> &initPt) throw();
		// create from a size
		xPoint<type>(const xSize<type> &initSize) throw();

		// Operations

		// translate the point
		void Offset(type xOffset, type yOffset) throw();
		void Offset(xPoint<type> point) throw();
		void Offset(xSize<type> size) throw();
		void SetPoint(type X, type Y) throw();
		type GetX() const throw();
		type GetY() const throw();
		bool operator==(xPoint<type> point) const throw();
		bool operator!=(xPoint<type> point) const throw();
		void operator+=(xSize<type> size) throw();
		void operator-=(xSize<type> size) throw();
		void operator+=(xPoint<type> point) throw();
		void operator-=(xPoint<type> point) throw();

		// Operators returning xgcPoint<type> values
		xPoint<type> operator+(xSize<type> size) const throw();
		xPoint<type> operator-(xSize<type> size) const throw();
		xPoint<type> operator-() const throw();
		xPoint<type> operator+(xPoint<type> point) const throw();

		// Operators returning xgcSize<type> values
		xSize<type> operator-(xPoint<type> point) const throw();

		// Operators returning xgcRect<type> values
		xRect<type> operator+(const xRect<type>* lpRect) const throw();
		xRect<type> operator-(const xRect<type>* lpRect) const throw();
	};

	/////////////////////////////////////////////////////////////////////////////
	// xgcRect<type> - A 2-D rectangle, similar to Windows xgcRect<type> structure.
	template< class type > 
	class xRect
	{
		// Constructors
	public:
		type    left;
		type    top;
		type    right;
		type    bottom;

		// uninitialized rectangle
		xRect<type>() throw();
		// from left, top, right, and bottom
		xRect<type>(type l, type t, type r, type b) throw();
		// copy constructor
		xRect<type>(const xRect<type>& srcRect) throw();
		// from a pointer to another rect
		xRect<type>(const xRect<type>* lpSrcRect) throw();
		// from a point and size
		xRect<type>(xPoint<type> point, xSize<type> size) throw();
		// from two points
		xRect<type>(xPoint<type> topLeft, xPoint<type> bottomRight) throw();

		// Attributes (in addition to xgcRect<type> members)

		// retrieves the width
		type Width() const throw();
		// returns the height
		type Height() const throw();
		// returns the size
		xSize<type> Size() const throw();
		// reference to the top-left point
		xPoint<type>& TopLeft() throw();
		// reference to the bottom-right point
		xPoint<type>& BottomRight() throw();
		// const reference to the top-left point
		const xPoint<type>& TopLeft() const throw();
		// const reference to the bottom-right point
		const xPoint<type>& BottomRight() const throw();
		// the geometric center point of the rectangle
		xPoint<type> CenterPoint() const throw();
		// swap the left and right
		void SwapLeftRight() throw();

		// returns true if rectangle has no area
		bool IsRectEmpty() const throw();
		// returns true if rectangle is at (0,0) and has no area
		bool IsRectNull() const throw();
		// returns true if point is within rectangle
		bool PtInRect(xPoint<type> point) const throw();

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(type x1, type y1, type x2, type y2) throw();
		void SetRect(xPoint<type> topLeft, xPoint<type> bottomRight) throw();
		// empty the rectangle
		void SetRectEmpty() throw();
		// copy from another rectangle
		void CopyRect(const xRect<type>* lpSrcRect) throw();
		// true if exactly the same as another rectangle
		bool EqualRect(const xRect<type>* lpRect) const throw();

		// Inflate rectangle's width and height by
		// x units to the left and right ends of the rectangle
		// and y units to the top and bottom.
		void InflateRect(type x, type y) throw();
		// Inflate rectangle's width and height by
		// size.cx units to the left and right ends of the rectangle
		// and size.cy units to the top and bottom.
		void InflateRect(xSize<type> size) throw();
		// Inflate rectangle's width and height by moving individual sides.
		// Left side is moved to the left, right side is moved to the right,
		// top is moved up and bottom is moved down.
		void InflateRect(const xRect<type>* lpRect) throw();
		void InflateRect(type l, type t, type r, type b) throw();

		// deflate the rectangle's width and height without
		// moving its top or left
		void DeflateRect(type x, type y) throw();
		void DeflateRect(xSize<type> size) throw();
		void DeflateRect(const xRect<type> *lpRect) throw();
		void DeflateRect(type l, type t, type r, type b) throw();

		// translate the rectangle by moving its top and left
		void OffsetRect(type x, type y) throw();
		void OffsetRect(xSize<type> size) throw();
		void OffsetRect(xPoint<type> point) throw();
		void NormalizeRect() throw();

		// absolute position of rectangle
		void MoveToY(type y) throw();
		void MoveToX(type x) throw();
		void MoveToXY(type x, type y) throw();
		void MoveToXY(xPoint<type> point) throw();

		// set this rectangle to intersection of two others
		bool IntersectRect(const xRect<type>* lpRect1, const xRect<type>* lpRect2) throw();

		// set this rectangle to bounding union of two others
		bool UnionRect(const xRect<type>* lpRect1, const xRect<type>* lpRect2) throw();

		//// set this rectangle to minimum of two others
		//bool SubtractRect(xgcRect<type>* lpRectSrc1, xgcRect<type>* lpRectSrc2) throw();

		// Additional Operations
		void operator=(const xRect<type>& srcRect) throw();
		bool operator==(const xRect<type>& rect) const throw();
		bool operator!=(const xRect<type>& rect) const throw();
		void operator+=(xPoint<type> point) throw();
		void operator+=(xSize<type> size) throw();
		void operator+=(xRect<type>* lpRect) throw();
		void operator-=(xPoint<type> point) throw();
		void operator-=(xSize<type> size) throw();
		void operator-=(xRect<type>* lpRect) throw();
		void operator&=(const xRect<type>& rect) throw();
		void operator|=(const xRect<type>& rect) throw();

		// Operators returning xgcRect<type> values
		xRect<type> operator+(xPoint<type> point) const throw();
		xRect<type> operator-(xPoint<type> point) const throw();
		xRect<type> operator+(xRect<type>* lpRect) const throw();
		xRect<type> operator+(xSize<type> size) const throw();
		xRect<type> operator-(xSize<type> size) const throw();
		xRect<type> operator-(xRect<type>* lpRect) const throw();
		xRect<type> operator&(const xRect<type>& rect2) const throw();
		xRect<type> operator|(const xRect<type>& rect2) const throw();
		xRect<type> MulDiv(type nMultiplier, type nDivisor) const throw();
	};
	template< class type > 
	inline xSize<type>::xSize() throw()
	{ /* random filled */ }

	template< class type > 
	inline xSize<type>::xSize(type initCX, type initCY) throw()
	{ cx = initCX; cy = initCY; }

	template< class type > 
	inline xSize<type>::xSize(const xSize<type> &initSize) throw()
	{ *(xSize<type>*)this = initSize; }

	template< class type > 
	inline xSize<type>::xSize(const xPoint<type> &initPt) throw()
	{ *(xPoint<type>*)this = initPt; }

	template< class type > 
	inline bool xSize<type>::operator==(xSize<type> size) const throw()
	{ return (cx == size.cx && cy == size.cy); }

	template< class type > 
	inline bool xSize<type>::operator!=(xSize<type> size) const throw()
	{ return (cx != size.cx || cy != size.cy); }

	template< class type > 
	inline void xSize<type>::operator+=(xSize<type> size) throw()
	{ cx += size.cx; cy += size.cy; }

	template< class type > 
	inline void xSize<type>::operator-=(xSize<type> size) throw()
	{ cx -= size.cx; cy -= size.cy; }

	template< class type > 
	inline void xSize<type>::SetSize(type CX, type CY) throw()
	{ cx = CX; cy = CY; }	

	template< class type > 
	inline xSize<type> xSize<type>::operator+(xSize<type> size) const throw()
	{ return xSize<type>(cx + size.cx, cy + size.cy); }

	template< class type > 
	inline xSize<type> xSize<type>::operator-(xSize<type> size) const throw()
	{ return xSize<type>(cx - size.cx, cy - size.cy); }

	template< class type > 
	inline xSize<type> xSize<type>::operator-() const throw()
	{ return xSize<type>(-cx, -cy); }

	template< class type > 
	inline xPoint<type> xSize<type>::operator+(xPoint<type> point) const throw()
	{ return xPoint<type>(cx + point.x, cy + point.y); }

	template< class type > 
	inline xPoint<type> xSize<type>::operator-(xPoint<type> point) const throw()
	{ return xPoint<type>(cx - point.x, cy - point.y); }

	template< class type > 
	inline xRect<type> xSize<type>::operator+(const xRect<type>* lpRect) const throw()
	{ return xRect<type>(lpRect) + *this; }

	template< class type > 
	inline xRect<type> xSize<type>::operator-(const xRect<type>* lpRect) const throw()
	{ return xRect<type>(lpRect) - *this; }


	// xgcPoint<type>
	template< class type > 
	inline xPoint<type>::xPoint() throw()
	{ /* random filled */ }

	template< class type > 
	inline xPoint<type>::xPoint(type initX, type initY) throw()
	{ x = initX; y = initY; }

	template< class type > 
	inline xPoint<type>::xPoint(const xPoint<type> &initPt) throw()
	{ *(xPoint<type>*)this = initPt; }

	template< class type > 
	inline xPoint<type>::xPoint(const xSize<type> &initSize) throw()
	{ *(xSize<type>*)this = initSize; }

	template< class type > 
	inline void xPoint<type>::Offset(type xOffset, type yOffset) throw()
	{ x += xOffset; y += yOffset; }

	template< class type > 
	inline void xPoint<type>::Offset(xPoint<type> point) throw()
	{ x += point.x; y += point.y; }

	template< class type > 
	inline void xPoint<type>::Offset(xSize<type> size) throw()
	{ x += size.cx; y += size.cy; }

	template< class type > 
	inline void xPoint<type>::SetPoint(type X, type Y) throw()
	{ x = X; y = Y; }

	template< class type >
	inline type xPoint<type>::GetX() const throw()
	{
		return x;
	}

	template< class type >
	inline type xPoint<type>::GetY()const throw()
	{
		return y;
	}

	

	template< class type > 
	inline bool xPoint<type>::operator==(xPoint<type> point) const throw()
	{ return (x == point.x && y == point.y); }

	template< class type > 
	inline bool xPoint<type>::operator!=(xPoint<type> point) const throw()
	{ return (x != point.x || y != point.y); }

	template< class type > 
	inline void xPoint<type>::operator+=(xSize<type> size) throw()
	{ x += size.cx; y += size.cy; }

	template< class type > 
	inline void xPoint<type>::operator-=(xSize<type> size) throw()
	{ x -= size.cx; y -= size.cy; }

	template< class type > 
	inline void xPoint<type>::operator+=(xPoint<type> point) throw()
	{ x += point.x; y += point.y; }

	template< class type > 
	inline void xPoint<type>::operator-=(xPoint<type> point) throw()
	{ x -= point.x; y -= point.y; }

	template< class type > 
	inline xPoint<type> xPoint<type>::operator+(xSize<type> size) const throw()
	{ return xPoint<type>(x + size.cx, y + size.cy); }

	template< class type > 
	inline xPoint<type> xPoint<type>::operator-(xSize<type> size) const throw()
	{ return xPoint<type>(x - size.cx, y - size.cy); }

	template< class type > 
	inline xPoint<type> xPoint<type>::operator-() const throw()
	{ return xPoint<type>(-x, -y); }

	template< class type > 
	inline xPoint<type> xPoint<type>::operator+(xPoint<type> point) const throw()
	{ return xPoint<type>(x + point.x, y + point.y); }

	template< class type > 
	inline xSize<type> xPoint<type>::operator-(xPoint<type> point) const throw()
	{ return xSize<type>(x - point.x, y - point.y); }

	template< class type > 
	inline xRect<type> xPoint<type>::operator+(const xRect<type>* lpRect) const throw()
	{ return xRect<type>(lpRect) + *this; }

	template< class type > 
	inline xRect<type> xPoint<type>::operator-(const xRect<type>* lpRect) const throw()
	{ return xRect<type>(lpRect) - *this; }

	// xgcRect<type>
	template< class type > 
	inline xRect<type>::xRect() throw()
	{ /* random filled */ }

	template< class type > 
	inline xRect<type>::xRect(type l, type t, type r, type b) throw()
	{ left = l; top = t; right = r; bottom = b; }

	template< class type > 
	inline xRect<type>::xRect(const xRect<type>& srcRect) throw()
	{ CopyRect( &srcRect ); }

	template< class type > 
	inline xRect<type>::xRect(const xRect<type>* lpSrcRect) throw()
	{ CopyRect( lpSrcRect ); }

	template< class type > 
	inline xRect<type>::xRect(xPoint<type> point, xSize<type> size) throw()
	{ right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy; }

	template< class type > 
	inline xRect<type>::xRect(xPoint<type> topLeft, xPoint<type> bottomRight) throw()
	{ left = topLeft.x; top = topLeft.y;
	right = bottomRight.x; bottom = bottomRight.y; }

	template< class type > 
	inline type xRect<type>::Width() const throw()
	{ return right - left; }

	template< class type > 
	inline type xRect<type>::Height() const throw()
	{ return bottom - top; }

	template< class type > 
	inline xSize<type> xRect<type>::Size() const throw()
	{ return xSize<type>(right - left, bottom - top); }

	template< class type > 
	inline xPoint<type>& xRect<type>::TopLeft() throw()
	{ return *((xPoint<type>*)this); }

	template< class type > 
	inline xPoint<type>& xRect<type>::BottomRight() throw()
	{ return *((xPoint<type>*)this+1); }

	template< class type > 
	inline const xPoint<type>& xRect<type>::TopLeft() const throw()
	{ return *((xPoint<type>*)this); }

	template< class type > 
	inline const xPoint<type>& xRect<type>::BottomRight() const throw()
	{ return *((xPoint<type>*)this+1); }

	template< class type > 
	inline xPoint<type> xRect<type>::CenterPoint() const throw()
	{ return xPoint<type>((left+right)/2, (top+bottom)/2); }

	template< class type > 
	inline void xRect<type>::SwapLeftRight() throw()
	{ type temp = left; left = right; right = temp; }

	template< class type > 
	inline bool xRect<type>::IsRectEmpty() const throw()
	{ return (left == right && top == bottom); }

	template< class type > 
	inline bool xRect<type>::IsRectNull() const throw()
	{ return (left == 0 && right == 0 && top == 0 && bottom == 0); }

	template< class type > 
	inline bool xRect<type>::PtInRect(xPoint<type> point) const throw()
	{ return (point.x > left && point.x < right && point.y > top && point.y < bottom); }

	template< class type > 
	inline void xRect<type>::SetRect(type x1, type y1, type x2, type y2) throw()
	{ 
		left	= x1;
		top		= y1;
		right	= x2;
		bottom	= y2;
	}

	template< class type > 
	inline void xRect<type>::SetRect(xPoint<type> topLeft, xPoint<type> bottomRight) throw()
	{ 
		left	= topLeft.x;
		top		= topLeft.y;
		right	= bottomRight.x;
		bottom	= bottomRight.y;
 	}

	template< class type > 
	inline void xRect<type>::SetRectEmpty() throw()
	{ left = right = top = bottom = 0; }

	template< class type > 
	inline void xRect<type>::CopyRect(const xRect<type>* lpSrcRect) throw()
	{ 
		left	= lpSrcRect->left; 
		top		= lpSrcRect->top;
		right	= lpSrcRect->right;
		bottom	= lpSrcRect->bottom;
	}

	template< class type > 
	inline bool xRect<type>::EqualRect(const xRect<type>* lpRect) const throw()
	{ return left == lpRect->left && right == lpRect->right && top == lpRect->top && bottom == lpRect->bottom; }

	template< class type > 
	inline void xRect<type>::InflateRect(type x, type y) throw()
	{ left -= x;right += x; top -= y; bottom += y;}

	template< class type > 
	inline void xRect<type>::InflateRect(xSize<type> size) throw()
	{ InflateRect(size.cx, size.cy); }

	template< class type > 
	inline void xRect<type>::DeflateRect(type x, type y) throw()
	{ InflateRect(-x, -y); }

	template< class type > 
	inline void xRect<type>::DeflateRect(xSize<type> size) throw()
	{ InflateRect(-size.cx, -size.cy); }

	template< class type > 
	inline void xRect<type>::OffsetRect(type x, type y) throw()
	{ left += x;right += x;top += y;bottom += y; }

	template< class type > 
	inline void xRect<type>::OffsetRect(xPoint<type> point) throw()
	{ OffsetRect(point.x, point.y); }

	template< class type > 
	inline void xRect<type>::OffsetRect(xSize<type> size) throw()
	{ OffsetRect(size.cx, size.cy); }

	template< class type > 
	inline void xRect<type>::MoveToY(type y) throw()
	{ bottom = Height() + y; top = y; }

	template< class type > 
	inline void xRect<type>::MoveToX(type x) throw()
	{ right = Width() + x; left = x; }

	template< class type > 
	inline void xRect<type>::MoveToXY(type x, type y) throw()
	{ MoveToX(x); MoveToY(y); }

	template< class type > 
	inline void xRect<type>::MoveToXY(xPoint<type> pt) throw()
	{ MoveToX(pt.x); MoveToY(pt.y); }

	template< class type > 
	inline bool xRect<type>::IntersectRect(const xRect<type>* lpRect1, const xRect<type>* lpRect2) throw()
	{ 
		left	= __max( lpRect1->left, lpRect2->left );
		right	= __min( lpRect1->right, lpRect2->right );
		top		= __max( lpRect1->top, lpRect2->top );
		bottom	= __min( lpRect1->bottom, lpRect2->bottom );

		return ( left > right || top > bottom );
	}

	template< class type > 
	inline bool xRect<type>::UnionRect(const xRect<type>* lpRect1, const xRect<type>* lpRect2) throw()
	{ 
		left	= __min( lpRect1->left, lpRect2->left );
		right	= __max( lpRect1->right, lpRect2->right );
		top		= __min( lpRect1->top, lpRect2->top );
		bottom	= __max( lpRect1->bottom, lpRect2->bottom );

		return ( left < right && top < bottom );
	}

	template< class type > 
	inline void xRect<type>::operator=(const xRect<type>& srcRect) throw()
	{ CopyRect(&srcRect); }

	template< class type > 
	inline bool xRect<type>::operator==(const xRect<type>& rect) const throw()
	{ return EqualRect(&rect); }

	template< class type > 
	inline bool xRect<type>::operator!=(const xRect<type>& rect) const throw()
	{ return !EqualRect(&rect); }

	template< class type > 
	inline void xRect<type>::operator+=(xPoint<type> point) throw()
	{ OffsetRect(point.x, point.y); }

	template< class type > 
	inline void xRect<type>::operator+=(xSize<type> size) throw()
	{ OffsetRect(size.cx, size.cy); }

	template< class type > 
	inline void xRect<type>::operator+=(xRect<type>* lpRect) throw()
	{ InflateRect(lpRect); }

	template< class type > 
	inline void xRect<type>::operator-=(xPoint<type> point) throw()
	{ OffsetRect(-point.x, -point.y); }

	template< class type > 
	inline void xRect<type>::operator-=(xSize<type> size) throw()
	{ OffsetRect(-size.cx, -size.cy); }

	template< class type > 
	inline void xRect<type>::operator-=(xRect<type>* lpRect) throw()
	{ DeflateRect(lpRect); }

	template< class type > 
	inline void xRect<type>::operator&=(const xRect<type>& rect) throw()
	{ IntersectRect(this, &rect); }

	template< class type > 
	inline void xRect<type>::operator|=(const xRect<type>& rect) throw()
	{ UnionRect(this, &rect); }

	template< class type > 
	inline xRect<type> xRect<type>::operator+(xPoint<type> pt) const throw()
	{ xRect<type> rect(*this); rect.OffsetRect(pt.x, pt.y); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator-(xPoint<type> pt) const throw()
	{ xRect<type> rect(*this); rect.OffsetRect(-pt.x, -pt.y); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator+(xSize<type> size) const throw()
	{ xRect<type> rect(*this); rect.OffsetRect(size.cx, size.cy); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator-(xSize<type> size) const throw()
	{ xRect<type> rect(*this); rect.OffsetRect(-size.cx, -size.cy); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator+(xRect<type>* lpRect) const throw()
	{ xRect<type> rect(this); rect.InflateRect(lpRect); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator-(xRect<type>* lpRect) const throw()
	{ xRect<type> rect(this); rect.DeflateRect(lpRect); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator&(const xRect<type>& rect2) const throw()
	{ xRect<type> rect; rect.IntersectRect(this, &rect2); return rect; }

	template< class type > 
	inline xRect<type> xRect<type>::operator|(const xRect<type>& rect2) const throw()
	{ xRect<type> rect; rect.UnionRect(this, &rect2);return rect; }

	template< class type > 
	inline void xRect<type>::NormalizeRect() throw()
	{
		type nTemp;
		if (left > right)
		{
			nTemp = left;
			left = right;
			right = nTemp;
		}
		if (top > bottom)
		{
			nTemp = top;
			top = bottom;
			bottom = nTemp;
		}
	}

	template< class type > 
	inline void xRect<type>::InflateRect(const xRect<type>* lpRect) throw()
	{
		left -= lpRect->left;		top -= lpRect->top;
		right += lpRect->right;		bottom += lpRect->bottom;
	}

	template< class type > 
	inline void xRect<type>::InflateRect(type l, type t, type r, type b) throw()
	{
		left -= l;			top -= t;
		right += r;			bottom += b;
	}

	template< class type > 
	inline void xRect<type>::DeflateRect(const xRect<type>* lpRect) throw()
	{
		left += lpRect->left;	top += lpRect->top;
		right -= lpRect->right;	bottom -= lpRect->bottom;
	}

	template< class type > 
	inline void xRect<type>::DeflateRect(type l, type t, type r, type b) throw()
	{
		left += l;		top += t;
		right -= r;		bottom -= b;
	}

	typedef xRect	< xgc_int32 >	iRect;
	typedef xSize	< xgc_int32 >	iSize;
	typedef xPoint	< xgc_int32 >	iPoint;

	typedef xRect	< xgc_real32 >	fRect;
	typedef xSize	< xgc_real32 >	fSize;
	typedef xPoint	< xgc_real32 >	fPoint;

	///
	/// 碰撞分组
	/// [11/21/2014] create by albert.xu
	///
	union CollisionFilter64
	{
		/// @var 值
		xgc_uint64 value;

		struct
		{
			/// @var 被撞掩码
			xgc_uint16	category;
			/// @var 碰撞掩码
			xgc_uint16	mask;
			/// @var 组编号
			xgc_int32	group;
		};
	};

	///
	/// 碰撞分组
	/// [11/21/2014] create by albert.xu
	///
	union CollisionFilter32
	{
		/// @var 值
		xgc_uint32 value;

		struct
		{
			/// @var 被撞掩码
			xgc_uint8	category;
			/// @var 碰撞掩码
			xgc_uint8	mask;
			/// @var 组编号
			xgc_int16	group;
		};
	};

	///
	/// 碰撞检测函数
	/// A,B的组ID相等且大于0时，两物体一定碰撞
	/// A,B的组ID相等且小于0时，两物体一定不撞
	/// A,B的组ID不等则
	/// 判定A的mask是否和B的category有相同的位
	/// 并且
	///	判定B的mask是否和A的category有相同的位
	///
	template< class T >
	xgc_bool CollisionTest( const T &A, const T &B )
	{
		if( A.group == B.group && A.group != 0 )
			return A.group > 0;

		return ( A.mask & B.category ) != 0 && ( A.category & B.mask ) != 0;
	}
}
#endif // _XGEOMETRY_H_
