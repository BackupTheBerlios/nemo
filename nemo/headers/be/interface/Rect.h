//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		Rect.h
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BRect represents a rectangular area.
//------------------------------------------------------------------------------

#ifndef	_RECT_H
#define	_RECT_H

// Standard Includes -----------------------------------------------------------
#include <math.h>

// System Includes -------------------------------------------------------------
#include <SupportDefs.h>
#include <Point.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


// BRect class -----------------------------------------------------------------
class BRect {
public:
	int32	left;
	int32	top;
	int32	right;
	int32	bottom;

	BRect();
	BRect(const BRect &r);
	BRect(int32 l, int32 t, int32 r, int32 b);
	BRect(BPoint lt, BPoint rb);

	BRect	&operator=(const BRect &r);
	void	Set(int32 l, int32 t, int32 r, int32 b);

	void	PrintToStream() const;

	BPoint	LeftTop() const;
	BPoint	RightBottom() const;
	BPoint	LeftBottom() const;
	BPoint	RightTop() const;

	void	SetLeftTop(const BPoint p);
	void	SetRightBottom(const BPoint p);
	void	SetLeftBottom(const BPoint p);
	void	SetRightTop(const BPoint p);

	// transformation
	void	InsetBy(BPoint p);
	void	InsetBy(int32 dx, int32 dy);
	void	OffsetBy(BPoint p);
	void	OffsetBy(int32 dx, int32 dy);
	void	OffsetTo(BPoint p);
	void	OffsetTo(int32 x, int32 y);

	// expression transformations
	BRect &	InsetBySelf(BPoint);
	BRect &	InsetBySelf(int32 dx, int32 dy);
	BRect	InsetByCopy(BPoint);
	BRect	InsetByCopy(int32 dx, int32 dy);
	BRect &	OffsetBySelf(BPoint);
	BRect &	OffsetBySelf(int32 dx, int32 dy);
	BRect	OffsetByCopy(BPoint);
	BRect	OffsetByCopy(int32 dx, int32 dy);
	BRect &	OffsetToSelf(BPoint);
	BRect &	OffsetToSelf(int32 dx, int32 dy);
	BRect	OffsetToCopy(BPoint);
	BRect	OffsetToCopy(int32 dx, int32 dy);

	// comparison
	bool	operator==(BRect r) const;
	bool	operator!=(BRect r) const;

	// intersection and union
	BRect	operator&(BRect r) const;
	BRect	operator|(BRect r) const;

	bool	Intersects(BRect r) const;
	bool	IsValid() const;
	int32		Width() const;
	int32	IntegerWidth() const;
	int32		Height() const;
	int32	IntegerHeight() const;
	bool	Contains(BPoint p) const;
	bool	Contains(BRect r) const;

};
//------------------------------------------------------------------------------

// inline definitions ----------------------------------------------------------

inline BPoint BRect::LeftTop() const
{
	return(*((const BPoint*)&left));
}

inline BPoint BRect::RightBottom() const
{
	return(*((const BPoint*)&right));
}

inline BPoint BRect::LeftBottom() const
{
	return(BPoint(left, bottom));
}

inline BPoint BRect::RightTop() const
{
	return(BPoint(right, top));
}

inline BRect::BRect()
{
	top = left = 0;
	bottom = right = -1;
}

inline BRect::BRect(int32 l, int32 t, int32 r, int32 b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline BRect::BRect(const BRect &r)
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}

inline BRect::BRect(BPoint leftTop, BPoint rightBottom)
{
	left = leftTop.x;
	top = leftTop.y;
	right = rightBottom.x;
	bottom = rightBottom.y;
}

inline BRect &BRect::operator=(const BRect& from)
{
	left = from.left;
	top = from.top;
	right = from.right;
	bottom = from.bottom;
	return *this;
}

inline void BRect::Set(int32 l, int32 t, int32 r, int32 b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline bool BRect::IsValid() const
{
	if (left <= right && top <= bottom)
		return true;
	else
		return false;
}

inline int32 BRect::IntegerWidth() const
{
	return((int32)ceil(right - left));
}

inline int32 BRect::Width() const
{
	return(right - left);
}

inline int32 BRect::IntegerHeight() const
{
	return((int32)ceil(bottom - top));
}

inline int32 BRect::Height() const
{
	return(bottom - top);
}

//------------------------------------------------------------------------------

#endif	// _RECT_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

