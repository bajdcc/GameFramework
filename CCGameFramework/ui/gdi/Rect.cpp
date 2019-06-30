#include "stdafx.h"
#include "Gdi.h"


// CRect
CRect::CRect()
{
    left = top = right = bottom = 0;
}
CRect::CRect(int l, int t, int r, int b)
{
    left = l; top = t; right = r; bottom = b;
}
CRect::CRect(const RECT& srXRect)
{
    ::CopyRect(this, &srXRect);
}
CRect::CRect(LPCRECT lpSrcRect)
{
    ::CopyRect(this, lpSrcRect);
}
CRect::CRect(POINT point, SIZE size)
{
    right = (left = point.x) + size.cx; bottom = (top = point.y) + size.cy;
}
CRect::CRect(POINT topLeft, POINT bottomRight)
{
    left = topLeft.x; top = topLeft.y;
    right = bottomRight.x; bottom = bottomRight.y;
}
int CRect::Width() const
{
    return right - left;
}
int CRect::Height() const
{
    return bottom - top;
}
CSize CRect::Size() const
{
    return CSize(right - left, bottom - top);
}
CPoint& CRect::TopLeft()
{
    return *((CPoint*)this);
}
CPoint& CRect::BottomRight()
{
    return *((CPoint*)this + 1);
}
const CPoint& CRect::TopLeft() const
{
    return *((CPoint*)this);
}
const CPoint& CRect::BottomRight() const
{
    return *((CPoint*)this + 1);
}
CPoint CRect::CenterPoint() const
{
    return CPoint((left + right) / 2, (top + bottom) / 2);
}
void CRect::SwapLeftRight()
{
    SwapLeftRight(LPRECT(this));
}
void CRect::SwapLeftRight(LPRECT lpRect)
{
    LONG temp = lpRect->left; lpRect->left = lpRect->right; lpRect->right = temp;
}
CRect::operator LPRECT()
{
    return this;
}
CRect::operator LPCRECT() const
{
    return this;
}
BOOL CRect::IsRectEmpty() const
{
    return ::IsRectEmpty(this);
}
BOOL CRect::IsRectNull() const
{
    return (left == 0 && right == 0 && top == 0 && bottom == 0);
}
BOOL CRect::PtInRect(POINT point) const
{
    return ::PtInRect(this, point);
}
void CRect::SetRect(int x1, int y1, int x2, int y2)
{
    ::SetRect(this, x1, y1, x2, y2);
}
void CRect::SetRect(POINT topLeft, POINT bottomRight)
{
    ::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}
void CRect::SetRectEmpty()
{
    ::SetRectEmpty(this);
}
void CRect::CopyRect(LPCRECT lpSrcRect)
{
    ::CopyRect(this, lpSrcRect);
}
BOOL CRect::EqualRect(LPCRECT lpRect) const
{
    return ::EqualRect(this, lpRect);
}
void CRect::InflateRect(int x, int y)
{
    ::InflateRect(this, x, y);
}
void CRect::InflateRect(SIZE size)
{
    ::InflateRect(this, size.cx, size.cy);
}
void CRect::DeflateRect(int x, int y)
{
    ::InflateRect(this, -x, -y);
}
void CRect::DeflateRect(SIZE size)
{
    ::InflateRect(this, -size.cx, -size.cy);
}
void CRect::OffsetRect(int x, int y)
{
    ::OffsetRect(this, x, y);
}
void CRect::OffsetRect(POINT point)
{
    ::OffsetRect(this, point.x, point.y);
}
CRect CRect::OfRect(const CRect& rt, bool add)
{
    auto r = *this;
    if (add) r += rt.TopLeft();
    r.left = __max(rt.left, __min(r.left, rt.right));
    r.top = __max(rt.top, __min(r.top, rt.bottom));
    r.right = __max(rt.left, __min(r.right, rt.right));
    r.bottom = __max(rt.top, __min(r.bottom, rt.bottom));
    return r;
}
void CRect::OffsetRect(SIZE size)
{
    ::OffsetRect(this, size.cx, size.cy);
}
BOOL CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
{
    return ::IntersectRect(this, lpRect1, lpRect2);
}
BOOL CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
{
    return ::UnionRect(this, lpRect1, lpRect2);
}
void CRect::operator=(const RECT& srXRect)
{
    ::CopyRect(this, &srXRect);
}
BOOL CRect::operator==(const RECT& rect) const
{
    return ::EqualRect(this, &rect);
}
BOOL CRect::operator!=(const RECT& rect) const
{
    return !::EqualRect(this, &rect);
}
void CRect::operator+=(POINT point)
{
    ::OffsetRect(this, point.x, point.y);
}
void CRect::operator+=(SIZE size)
{
    ::OffsetRect(this, size.cx, size.cy);
}
void CRect::operator+=(LPCRECT lpRect)
{
    InflateRect(lpRect);
}
void CRect::operator-=(POINT point)
{
    ::OffsetRect(this, -point.x, -point.y);
}
void CRect::operator-=(SIZE size)
{
    ::OffsetRect(this, -size.cx, -size.cy);
}
void CRect::operator-=(LPCRECT lpRect)
{
    DeflateRect(lpRect);
}
void CRect::operator&=(const RECT& rect)
{
    ::IntersectRect(this, this, &rect);
}
void CRect::operator|=(const RECT& rect)
{
    ::UnionRect(this, this, &rect);
}
CRect CRect::operator+(POINT pt) const
{
    CRect rect(*this); ::OffsetRect(&rect, pt.x, pt.y); return rect;
}
CRect CRect::operator-(POINT pt) const
{
    CRect rect(*this); ::OffsetRect(&rect, -pt.x, -pt.y); return rect;
}
CRect CRect::operator+(SIZE size) const
{
    CRect rect(*this); ::OffsetRect(&rect, size.cx, size.cy); return rect;
}
CRect CRect::operator-(SIZE size) const
{
    CRect rect(*this); ::OffsetRect(&rect, -size.cx, -size.cy); return rect;
}
CRect CRect::operator+(LPCRECT lpRect) const
{
    CRect rect(this); rect.InflateRect(lpRect); return rect;
}
CRect CRect::operator-(LPCRECT lpRect) const
{
    CRect rect(this); rect.DeflateRect(lpRect); return rect;
}
CRect CRect::operator&(const RECT& rect2) const
{
    CRect rect; ::IntersectRect(&rect, this, &rect2);
    return rect;
}
CRect CRect::operator|(const RECT& rect2) const
{
    CRect rect; ::UnionRect(&rect, this, &rect2);
    return rect;
}
BOOL CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
{
    return ::SubtractRect(this, lpRectSrc1, lpRectSrc2);
}

void CRect::InflateRect(LPCRECT lpRect)
{
    left -= lpRect->left;
    top -= lpRect->top;
    right += lpRect->right;
    bottom += lpRect->bottom;
}

void CRect::DeflateRect(LPCRECT lpRect)
{
    left += lpRect->left;
    top += lpRect->top;
    right -= lpRect->right;
    bottom -= lpRect->bottom;
}

void CRect::InsetRect(const RECT& srcRect)
{
    left += srcRect.left;
    top += srcRect.top;
    right -= srcRect.right;
    bottom -= srcRect.bottom;
}

BOOL CRect::RestrictRect(const RECT& srcRect)
{
    CSize offset;

    UINT bFitness = 0;

    if (left < srcRect.left) {
        offset.cx = srcRect.left - left;
    }
    else if (right > srcRect.right) {
        offset.cx = srcRect.right - right;
    }
    else {
        bFitness |= 0x000F;
    }

    if (top < srcRect.top) {
        offset.cy = srcRect.top - top;
    }
    else if (bottom > srcRect.bottom) {
        offset.cy = srcRect.bottom - bottom;
    }
    else {
        bFitness |= 0x00F0;
    }

    OffsetRect(offset);

    return bFitness == 0x00FF ? TRUE : FALSE;
}

CStringA CRect::ToString() const
{
    CStringA str;
    str.Format("Rect(%d, %d, %d, %d)", left, top, right, bottom);
    return str;
}
