#include "stdafx.h"
#include "Gdi.h"


// CSize
CSize::CSize()
{
    cx = cy = 0;
}
CSize::CSize(int initCX, int initCY)
{
    cx = initCX; cy = initCY;
}
CSize::CSize(SIZE initSize)
{
    *(SIZE*)this = initSize;
}
CSize::CSize(POINT initPt)
{
    *(POINT*)this = initPt;
}
CSize::CSize(DWORD dwSize)
{
    cx = (short)LOWORD(dwSize);
    cy = (short)HIWORD(dwSize);
}
BOOL CSize::operator==(SIZE size) const
{
    return (cx == size.cx && cy == size.cy);
}
BOOL CSize::operator!=(SIZE size) const
{
    return (cx != size.cx || cy != size.cy);
}
void CSize::operator+=(SIZE size)
{
    cx += size.cx; cy += size.cy;
}
void CSize::operator-=(SIZE size)
{
    cx -= size.cx; cy -= size.cy;
}
CSize CSize::operator+(SIZE size) const
{
    return CSize(cx + size.cx, cy + size.cy);
}
CSize CSize::operator-(SIZE size) const
{
    return CSize(cx - size.cx, cy - size.cy);
}
CSize CSize::operator-() const
{
    return CSize(-cx, -cy);
}

CSize CSize::operator*(int n) const
{
    return CSize(cx * n, cy * n);
}

CSize CSize::operator/(int n) const
{
    return CSize(cx / n, cy / n);
}

CPoint CSize::operator+(POINT point) const
{
    return CPoint(cx + point.x, cy + point.y);
}
CPoint CSize::operator-(POINT point) const
{
    return CPoint(cx - point.x, cy - point.y);
}
CRect CSize::operator+(const RECT* lpRect) const
{
    return CRect(lpRect) + *this;
}
CRect CSize::operator-(const RECT* lpRect) const
{
    return CRect(lpRect) - *this;
}

CSize CSize::Max(CSize size)
{
    return CSize(__max(cx, size.cx), __max(cy, size.cy));
}

CStringA CSize::ToString() const
{
    CStringA str;
    str.Format("Size(%d, %d)", cx, cy);
    return str;
}
