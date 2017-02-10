#include "stdafx.h"
#include "Gdi.h"


// CPoint
CPoint::CPoint()
{
    x = y = 0;
}
CPoint::CPoint(int initX, int initY)
{
    x = initX; y = initY;
}
#if !defined(_AFX_CORE_IMPL) || !defined(_AFXDLL) || defined(_DEBUG)
CPoint::CPoint(POINT initPt)
{
    *(POINT*)this = initPt;
}
#endif
CPoint::CPoint(SIZE initSize)
{
    *(SIZE*)this = initSize;
}
CPoint::CPoint(DWORD dwPoint)
{
    x = (short)LOWORD(dwPoint);
    y = (short)HIWORD(dwPoint);
}
void CPoint::Offset(int xOffset, int yOffset)
{
    x += xOffset; y += yOffset;
}
void CPoint::Offset(POINT point)
{
    x += point.x; y += point.y;
}
void CPoint::Offset(SIZE size)
{
    x += size.cx; y += size.cy;
}
BOOL CPoint::operator==(POINT point) const
{
    return (x == point.x && y == point.y);
}
BOOL CPoint::operator!=(POINT point) const
{
    return (x != point.x || y != point.y);
}
void CPoint::operator+=(SIZE size)
{
    x += size.cx; y += size.cy;
}
void CPoint::operator-=(SIZE size)
{
    x -= size.cx; y -= size.cy;
}
void CPoint::operator+=(POINT point)
{
    x += point.x; y += point.y;
}
void CPoint::operator-=(POINT point)
{
    x -= point.x; y -= point.y;
}
CPoint CPoint::operator+(SIZE size) const
{
    return CPoint(x + size.cx, y + size.cy);
}
CPoint CPoint::operator-(SIZE size) const
{
    return CPoint(x - size.cx, y - size.cy);
}
CPoint CPoint::operator-() const
{
    return CPoint(-x, -y);
}
CPoint CPoint::operator+(POINT point) const
{
    return CPoint(x + point.x, y + point.y);
}
CSize CPoint::operator-(POINT point) const
{
    return CSize(x - point.x, y - point.y);
}
CRect CPoint::operator+(const RECT* lpRect) const
{
    return CRect(lpRect) + *this;
}
CRect CPoint::operator-(const RECT* lpRect) const
{
    return CRect(lpRect) - *this;
}

CStringA CPoint::ToString()
{
    CStringA str;
    str.Format("Point(%d, %d)", x, y);
    return str;
}