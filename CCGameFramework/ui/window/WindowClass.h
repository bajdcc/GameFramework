#ifndef UI_WINDOWCLASS_H
#define UI_WINDOWCLASS_H

class WindowClass
{
public:
    WindowClass(CString _name, bool shadow, bool ownDC, WNDPROC procedure, HINSTANCE hInstance);

    bool IsAvailable() const;
    CString GetName() const;
    ATOM GetClassAtom() const;

protected:
    CString									name;
    WNDCLASSEX								windowClass;
    ATOM									windowAtom;
};

#endif