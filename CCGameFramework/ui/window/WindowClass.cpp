#include "stdafx.h"
#include "WindowClass.h"


WindowClass::WindowClass(CString _name, bool shadow, bool ownDC, WNDPROC procedure, HINSTANCE hInstance)
{
    name = _name;
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | (shadow ? CS_DROPSHADOW : 0) | (ownDC ? CS_OWNDC : 0);
    windowClass.lpfnWndProc = procedure;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = name.GetBuffer();
    windowClass.hIconSm = NULL;
    windowAtom = ::RegisterClassEx(&windowClass);
}

bool WindowClass::IsAvailable() const
{
    return windowAtom != 0;
}

CString WindowClass::GetName() const
{
    return name;
}

ATOM WindowClass::GetClassAtom() const
{
    return windowAtom;
}