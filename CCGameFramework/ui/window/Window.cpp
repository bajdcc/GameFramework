#include "stdafx.h"
#include "Window.h"
#include "WindowMsgLoop.h"
#include <lua_ext/ext.h>

static bool IsKeyPressing(cint code)
{
    return (GetKeyState((int)code) & 0xF0) != 0;
}

static bool IsKeyToggled(cint code)
{
    return (GetKeyState((int)code) & 0x0F) != 0;
}

static CString GetLastErrorStr() // Error Notification
{
    LPSTR format_string = NULL; // format string

    DWORD dwRet = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED),
        (LPSTR)&format_string,
        0,
        NULL);

    CString buf(format_string);

    if (dwRet)
    {
        LocalFree(format_string);
    }

    return buf;
}

Window *window;

extern int ui_clear_scene(lua_State *L)
{
    window->layers.clear();
    return 0;
}

extern int ui_add_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checknumber(L, -1)));
    lua_pop(L, 1);
    switch (type)
    {
    case SolidBackground:
    {
        RefPtr<SolidBackgroundElement> obj = SolidBackgroundElement::Create();
        window->layers.push_back(obj);
        lua_pushnumber(L, window->layers.size());
    }
    break;
    case SolidLabel:
    {
        RefPtr<SolidLabelElement> obj = SolidLabelElement::Create();
        window->layers.push_back(obj);
        lua_pushnumber(L, window->layers.size());
    }
    break;
    case GradientBackground:
    {
        RefPtr<GradientBackgroundElement> obj = GradientBackgroundElement::Create();
        window->layers.push_back(obj);
        lua_pushnumber(L, window->layers.size());
    }
    break;
    default:
        return luaL_argerror(L, 1, "Invalid obj id");
    }
    return 1;
}

extern int ui_update_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checknumber(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checknumber(L, -1)); lua_pop(L, 1);
    if (handle <= 0 || handle > cint(window->layers.size()))
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->layers[handle - 1];
    if (o->GetTypeId() != type)
        return luaL_argerror(L, 1, "Invalid obj type");
    {
        CRect rt;
        lua_getfield(L, -1, "left");
        rt.left = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "top");
        rt.top = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "right");
        rt.right = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "bottom");
        rt.bottom = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        o->SetRenderRect(rt);
    }
    switch (type)
    {
    case SolidBackground:
    {
        auto obj = static_cast<SolidBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(CString(color)));
        }
    }
    break;
    case SolidLabel:
    {
        auto obj = static_cast<SolidLabelElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(CString(color)));
        }
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checknumber(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checklstring(L, -1, NULL)); lua_pop(L, 1);
            obj->SetFont(font);
        }
        {
            lua_getfield(L, -1, "text");
            obj->SetText(CString(luaL_checklstring(L, -1, NULL))); lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "align");
            obj->SetHorizontalAlignment(Alignment(cint(luaL_checknumber(L, -1)))); lua_pop(L, 1);
            lua_getfield(L, -1, "valign");
            obj->SetVerticalAlignment(Alignment(cint(luaL_checknumber(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case GradientBackground:
    {
        auto obj = static_cast<GradientBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color1");
            auto color1 = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor1(CColor::Parse(CString(color1)));
            lua_getfield(L, -1, "color2");
            auto color2 = luaL_checklstring(L, -1, NULL); lua_pop(L, 1);
            obj->SetColor2(CColor::Parse(CString(color2)));
        }
        {
            lua_getfield(L, -1, "direction");
            obj->SetDirection(GradientBackgroundElement::Direction(cint(luaL_checknumber(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    default:
        break;
    }
    return 0;
}

extern int ui_info(lua_State *L)
{
    lua_newtable(L);
    auto size = window->GetClientWindowSize();
    lua_pushstring(L, "width");
    lua_pushnumber(L, size.cx);
    lua_settable(L, -3);
    lua_pushstring(L, "height");
    lua_pushnumber(L, size.cy);
    lua_settable(L, -3);
    return 1;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    if (window->HandleMessage(hwnd, uMsg, wParam, lParam, result))
    {
        return result;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window::Window(HWND parent, CString className, CString windowTitle, HINSTANCE hInstance)
    : wndClass(className, false, false, WndProc, hInstance)
    , d2dRenderTarget(adoptRef(new Direct2DRenderTarget(this)))
    , L(luaL_newstate())
{
    window = this;
    DWORD exStyle = WS_EX_APPWINDOW | WS_EX_CONTROLPARENT;
    DWORD style = WS_VISIBLE | WS_BORDER | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    CreateWindowEx(exStyle, className, windowTitle, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        parent, NULL, hInstance, NULL);
}


Window::~Window()
{
    Destroyed();
    DestroyWindow(handle);
    lua_close(L);
}

void Window::Run()
{
    Center();
    Show();
    winMsgLoop.Run();
}

void Window::Center()
{
    CSize screenSize(GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));
    CRect windowBounds = GetBounds();
    CPoint topLeft = (screenSize - windowBounds.Size()) / 2;
    SetWindowPos(handle, HWND_TOPMOST, topLeft.x, topLeft.y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

bool Window::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
#ifdef _DEBUG
    if (uMsg != WM_PAINT &&
        uMsg != WM_SETCURSOR &&
        uMsg != WM_NCHITTEST &&
        uMsg != WM_MOUSEMOVE &&
        uMsg != WM_NCMOUSEMOVE)
    {
        ATLTRACE(atlTraceHosting, 0, "hwnd: 0x%08x message: %-30S[0x%04x] {W:0x%08X,L:0x%08X}\n", hwnd,
            winMsgLoop.DebugGetMessageName(uMsg), uMsg, wParam, lParam);
    }
#endif // _DEBUG
    switch (uMsg)
    {
    case WM_DESTROY:
        Destroying();
        PostQuitMessage(0);
        break;
    }
    return HandleMessageInternal(hwnd, uMsg, wParam, lParam, result);
}

HWND Window::GetWindowHandle() const
{
    return handle;
}

PassRefPtr<Direct2DRenderTarget> Window::GetD2DRenderTarget()
{
    return d2dRenderTarget;
}

CRect Window::GetBounds()
{
    CRect rect;
    GetWindowRect(handle, &rect);
    return rect;
}

void Window::SetBounds(const CRect& bounds)
{
    CRect newBounds = bounds;
    MoveWindow(handle, (int)newBounds.left, (int)newBounds.top, (int)newBounds.Width(), (int)newBounds.Height(), FALSE);
}

CSize Window::GetClientSize()
{
    return GetClientBoundsInScreen().Size();
}

void Window::SetClientSize(CSize size)
{
    CRect required(CPoint(), size);
    CRect bounds;
    GetWindowRect(handle, &bounds);
    AdjustWindowRect(&required, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE);
    SetBounds(CRect(bounds.TopLeft(), required.Size()));
}

CRect Window::GetClientBoundsInScreen()
{
    CRect required;
    CRect bounds;
    GetWindowRect(handle, &bounds);
    AdjustWindowRect(&required, (DWORD)GetWindowLongPtr(handle, GWL_STYLE), FALSE);
    return CRect(bounds.TopLeft() + (-required.TopLeft()),
        bounds.Size() - required.Size());
}

CSize Window::GetClientWindowSize()
{
    CRect rect;
    GetClientRect(handle, &rect);
    return rect.Size();
}

CString Window::GetTitle()
{
    return title;
}

void Window::SetTitle(CString _title)
{
    title = _title;
    SetWindowText(handle, title);
}

Window::WindowSizeState Window::GetSizeState()
{
    return IsIconic(handle) ? Minimized :
        IsZoomed(handle) ? Maximized : Restored;
}

void Window::Show()
{
    ShowWindow(handle, SW_SHOWNORMAL);
}

void Window::Show(int nCmdShow)
{
    ShowWindow(handle, nCmdShow);
}

void Window::ShowDeactivated()
{
    ShowWindow(handle, SW_SHOWNA);
}

void Window::ShowRestored()
{
    ShowWindow(handle, SW_RESTORE);
}

void Window::ShowMaximized()
{
    ShowWindow(handle, SW_SHOWMAXIMIZED);
}

void Window::ShowMinimized()
{
    ShowWindow(handle, SW_SHOWMINIMIZED);
}

void Window::Hide()
{
    PostMessage(handle, WM_CLOSE, NULL, NULL);
}

bool Window::IsVisible()
{
    return IsWindowVisible(handle) != 0;
}

void Window::Enable()
{
    EnableWindow(handle, TRUE);
}

void Window::Disable()
{
    EnableWindow(handle, FALSE);
}

bool Window::IsEnabled()
{
    return IsWindowEnabled(handle) != 0;
}

void Window::SetFocus()
{
    ::SetFocus(handle);
}

bool Window::IsFocused()
{
    return GetFocus() == handle;
}

void Window::SetActivate()
{
    SetActiveWindow(handle);
}

bool Window::IsActivated()
{
    return GetActiveWindow() == handle;
}

void Window::ShowInTaskBar()
{
    SetExStyle(WS_EX_APPWINDOW, true);
}

void Window::HideInTaskBar()
{
    SetExStyle(WS_EX_APPWINDOW, false);
}

bool Window::IsAppearedInTaskBar()
{
    return GetExStyle(WS_EX_APPWINDOW);
}

void Window::EnableActivate()
{
    SetExStyle(WS_EX_NOACTIVATE, false);
}

void Window::DisableActivate()
{
    SetExStyle(WS_EX_NOACTIVATE, true);
}

bool Window::IsEnabledActivate()
{
    return !GetExStyle(WS_EX_NOACTIVATE);
}

bool Window::RequireCapture()
{
    SetCapture(handle);
    return true;
}

bool Window::ReleaseCapture()
{
    ::ReleaseCapture();
    return true;
}

bool Window::IsCapturing()
{
    return GetCapture() == handle;
}

bool Window::GetMaximizedBox()
{
    return GetStyle(WS_MAXIMIZEBOX);
}

void Window::SetMaximizedBox(bool visible)
{
    SetStyle(WS_MAXIMIZEBOX, visible);
}

bool Window::GetMinimizedBox()
{
    return GetStyle(WS_MINIMIZEBOX);
}

void Window::SetMinimizedBox(bool visible)
{
    SetStyle(WS_MINIMIZEBOX, visible);
}

bool Window::GetBorder()
{
    return GetStyle(WS_BORDER);
}

void Window::SetBorder(bool visible)
{
    SetStyle(WS_BORDER, visible);
}

bool Window::GetSizeBox()
{
    return GetStyle(WS_SIZEBOX);
}

void Window::SetSizeBox(bool visible)
{
    SetStyle(WS_SIZEBOX, visible);
}

bool Window::GetIconVisible()
{
    return GetStyle(WS_SYSMENU);
}

void Window::SetIconVisible(bool visible)
{
    SetStyle(WS_SYSMENU, visible);
}

bool Window::GetTitleBar()
{
    return GetStyle(WS_CAPTION);
}

void Window::SetTitleBar(bool visible)
{
    SetStyle(WS_CAPTION, visible);
}

bool Window::GetTopMost()
{
    return GetExStyle(WS_EX_TOPMOST);
}

void Window::SetTopMost(bool topmost)
{
    SetWindowPos(handle, (topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

void Window::RedrawContent()
{
    if (d2dRenderTarget)
    {
        PostMessage(handle, WM_PAINT, NULL, NULL);
    }
}

DWORD Window::InternalGetExStyle()
{
    return (DWORD)GetWindowLongPtr(handle, GWL_EXSTYLE);
}

void Window::InternalSetExStyle(DWORD exStyle)
{
    LONG_PTR result = SetWindowLongPtr(handle, GWL_EXSTYLE, exStyle);
}

bool Window::GetExStyle(DWORD exStyle)
{
    LONG_PTR Long = InternalGetExStyle();
    return (Long & exStyle) != 0;
}

void Window::SetExStyle(DWORD exStyle, bool available)
{
    DWORD Long = InternalGetExStyle();
    if (available)
    {
        Long |= exStyle;
    }
    else
    {
        Long &= ~exStyle;
    }
    InternalSetExStyle((DWORD)Long);
}

bool Window::GetStyle(DWORD style)
{
    LONG_PTR Long = GetWindowLongPtr(handle, GWL_STYLE);
    return (Long & style) != 0;
}

void Window::SetStyle(DWORD style, bool available)
{
    LONG_PTR Long = GetWindowLongPtr(handle, GWL_STYLE);
    if (available)
    {
        Long |= style;
    }
    else
    {
        Long &= ~style;
    }
    SetWindowLongPtr(handle, GWL_STYLE, Long);
}

bool Window::GetClassStyle(DWORD style)
{
    DWORD Long = GetClassLong(handle, GCL_STYLE);
    return (Long & style) != 0;
}

void Window::SetClassStyle(DWORD style, bool available)
{
    DWORD Long = GetClassLong(handle, GCL_STYLE);
    if (available)
    {
        Long |= style;
    }
    else
    {
        Long &= ~style;
    }
    SetClassLong(handle, GCL_STYLE, Long);
}

MouseInfo Window::ConvertMouse(WPARAM wParam, LPARAM lParam, bool wheelMessage, bool nonClient)
{
    MouseInfo info;

    info.nonClient = false;
    if (nonClient)
    {
        switch (wParam)
        {
        case HTMINBUTTON:
        case HTMAXBUTTON:
        case HTCLOSE:
            break;
        default:
            info.nonClient = true;
            break;
        }
    }

    if (wheelMessage)
    {
        info.wheel = GET_WHEEL_DELTA_WPARAM(wParam);
        wParam = GET_KEYSTATE_WPARAM(wParam);
    }
    else
    {
        info.wheel = 0;
    }

    if (nonClient)
    {
        info.ctrl = IsKeyPressing(VK_CONTROL);
        info.shift = IsKeyPressing(VK_SHIFT);
        info.left = IsKeyPressing(MK_LBUTTON);
        info.middle = IsKeyPressing(MK_MBUTTON);
        info.right = IsKeyPressing(MK_RBUTTON);

        POINTS point = MAKEPOINTS(lParam);
        CPoint offset = GetClientBoundsInScreen().TopLeft();
        info.pt.x = point.x - offset.x;
        info.pt.y = point.y - offset.y;
    }
    else
    {
        info.ctrl = (wParam & MK_CONTROL) != 0;
        info.shift = (wParam & MK_SHIFT) != 0;
        info.left = (wParam & MK_LBUTTON) != 0;
        info.middle = (wParam & MK_MBUTTON) != 0;
        info.right = (wParam & MK_RBUTTON) != 0;

        POINTS point = MAKEPOINTS(lParam);

        if (wheelMessage)
        {
            CPoint offset = GetClientBoundsInScreen().TopLeft();
            info.pt.x = point.x - offset.x;
            info.pt.y = point.y - offset.y;
        }
        else
        {
            info.pt.x = point.x;
            info.pt.y = point.y;
        }
    }
    return info;
}

KeyInfo Window::ConvertKey(WPARAM wParam, LPARAM lParam)
{
    KeyInfo info;
    info.code = wParam;
    info.ctrl = IsKeyPressing(VK_CONTROL);
    info.shift = IsKeyPressing(VK_SHIFT);
    info.alt = IsKeyPressing(VK_MENU);
    info.capslock = IsKeyToggled(VK_CAPITAL);
    return info;
}

CharInfo Window::ConvertChar(WPARAM wParam)
{
    CharInfo info;
    info.code = (wchar_t)wParam;
    info.ctrl = IsKeyPressing(VK_CONTROL);
    info.shift = IsKeyPressing(VK_SHIFT);
    info.alt = IsKeyPressing(VK_MENU);
    info.capslock = IsKeyToggled(VK_CAPITAL);
    return info;
}

void Window::TrackMouse(bool enable)
{
    TRACKMOUSEEVENT trackMouseEvent;
    trackMouseEvent.cbSize = sizeof(trackMouseEvent);
    trackMouseEvent.hwndTrack = handle;
    trackMouseEvent.dwFlags = (enable ? 0 : TME_CANCEL) | TME_HOVER | TME_LEAVE;
    trackMouseEvent.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&trackMouseEvent);
}

bool Window::HandleMessageInternal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    bool transferFocusEvent = false;
    bool nonClient = false;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
        transferFocusEvent = true;
    }
    switch (uMsg)
    {
        // ************************************** moving and sizing
    case WM_MOVING:
    case WM_SIZING:
    {
        LPRECT rawBounds = (LPRECT)lParam;
        CRect bounds(rawBounds);
        Moving(bounds, true);
        if (!bounds.EqualRect(rawBounds))
        {
            *rawBounds = bounds;
            result = TRUE;
        }
    }
    break;
    case WM_NCCREATE:
        handle = hwnd;
        break;
    case WM_CREATE:
        Created();
        break;
    case WM_MOVE:
    case WM_SIZE:
        Moved();
    break;
    // ************************************** state
    case WM_ENABLE:
        wParam == TRUE ? Enabled() : Disabled();
    break;
    case WM_SETFOCUS:
        GotFocus();
    break;
    case WM_KILLFOCUS:
        LostFocus();
    break;
    case WM_ACTIVATE:
        wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE ? Activated() : Deactivated();
    break;
    case WM_SHOWWINDOW:
        wParam == TRUE ? Opened() : Closed();
    break;
    case WM_CLOSE:
    {
        bool cancel = false;
        Closing(cancel);
        return cancel;
    }
    break;
    // ************************************** mouse
    case WM_NCLBUTTONDOWN:
        nonClient = true;
    case WM_LBUTTONDOWN:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        LeftButtonDown(info);
    }
    break;
    case WM_NCLBUTTONUP:
        nonClient = true;
    case WM_LBUTTONUP:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        LeftButtonUp(info);
    }
    break;
    case WM_NCLBUTTONDBLCLK:
        nonClient = true;
    case WM_LBUTTONDBLCLK:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        LeftButtonDoubleClick(info);
    }
    break;
    case WM_NCRBUTTONDOWN:
        nonClient = true;
    case WM_RBUTTONDOWN:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        RightButtonDown(info);
    }
    break;
    case WM_NCRBUTTONUP:
        nonClient = true;
    case WM_RBUTTONUP:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        RightButtonUp(info);
    }
    break;
    case WM_NCRBUTTONDBLCLK:
        nonClient = true;
    case WM_RBUTTONDBLCLK:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        RightButtonDoubleClick(info);
    }
    break;
    case WM_NCMBUTTONDOWN:
        nonClient = true;
    case WM_MBUTTONDOWN:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        MiddleButtonDown(info);
    }
    break;
    case WM_NCMBUTTONUP:
        nonClient = true;
    case WM_MBUTTONUP:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        MiddleButtonUp(info);
    }
    break;
    case WM_NCMBUTTONDBLCLK:
        nonClient = true;
    case WM_MBUTTONDBLCLK:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        MiddleButtonDoubleClick(info);
    }
    break;
    case WM_NCMOUSEMOVE:
        nonClient = true;
    case WM_MOUSEMOVE:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, false, nonClient);
        if (info.pt != mouseLast)
        {
            if (!mouseHoving)
            {
                mouseHoving = true;
                MouseEntered();
                TrackMouse(true);
                MouseMoving(info);
            }
        }
    }
    break;
    // ************************************** wheel
    case WM_MOUSEHWHEEL:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, true, false);
        HorizontalWheel(info);
    }
    break;
    case WM_MOUSEWHEEL:
    {
        MouseInfo info = ConvertMouse(wParam, lParam, true, false);
        VerticalWheel(info);
    }
    break;
    // ************************************** mouse state
    case WM_NCMOUSELEAVE:
        nonClient = true;
    case WM_MOUSELEAVE:
        if (!nonClient)
        {
            mouseLast.x = -1;
            mouseLast.y = -1;
            mouseHoving = false;
            MouseLeaved();
        }
        break;
    case WM_NCMOUSEHOVER:
    case WM_MOUSEHOVER:
        TrackMouse(true);
    break;
    // ************************************** key
    case WM_KEYUP:
    {
        KeyInfo info = ConvertKey(wParam, lParam);
        KeyUp(info);
    }
    break;
    case WM_KEYDOWN:
    {
        KeyInfo info = ConvertKey(wParam, lParam);
        KeyDown(info);
    }
    break;
    case WM_SYSKEYUP:
    {
        KeyInfo info = ConvertKey(wParam, lParam);
        SysKeyUp(info);
    }
    break;
    case WM_SYSKEYDOWN:
    {
        KeyInfo info = ConvertKey(wParam, lParam);
        SysKeyDown(info);
    }
    break;
    case WM_CHAR:
    {
        CharInfo info = ConvertChar(wParam);
        Char(info);
    }
    break;
    // ************************************** painting
    case WM_PAINT:
        Paint();
    break;
    case WM_ERASEBKGND:
        result = 0;
        return true;
    case WM_NCPAINT:
    case WM_SYNCPAINT:
        break;
        // ************************************** IME
    case WM_IME_SETCONTEXT:
        if (wParam == TRUE)
        {
            HIMC imc = ImmGetContext(handle);
            ImmAssociateContext(hwnd, imc);
            ImmReleaseContext(handle, imc);
        }
        break;
    case WM_IME_STARTCOMPOSITION:
        break;
        // ************************************** hit test
    case WM_NCHITTEST:
    {
        POINTS location = MAKEPOINTS(lParam);
        CPoint windowLocation = GetBounds().TopLeft();
        location.x -= (SHORT)windowLocation.x;
        location.y -= (SHORT)windowLocation.y;
        switch (HitTest(CPoint(location.x, location.y)))
        {
        case BorderNoSizing:
            result = HTBORDER;
            return true;
        case BorderLeft:
            result = HTLEFT;
            return true;
        case BorderRight:
            result = HTRIGHT;
            return true;
        case BorderTop:
            result = HTTOP;
            return true;
        case BorderBottom:
            result = HTBOTTOM;
            return true;
        case BorderLeftTop:
            result = HTTOPLEFT;
            return true;
        case BorderRightTop:
            result = HTTOPRIGHT;
            return true;
        case BorderLeftBottom:
            result = HTBOTTOMLEFT;
            return true;
        case BorderRightBottom:
            result = HTBOTTOMRIGHT;
            return true;
        case Title:
            result = HTCAPTION;
            return true;
        case ButtonMinimum:
            result = HTMINBUTTON;
            return true;
        case ButtonMaximum:
            result = HTMAXBUTTON;
            return true;
        case ButtonClose:
            result = HTCLOSE;
            return true;
        case Client:
            result = HTCLIENT;
            return true;
        case Icon:
            result = HTSYSMENU;
            return true;
        }
    }
    break;
    // ************************************** MISC
    case WM_SETCURSOR:
    {
        DWORD hitTestResult = LOWORD(lParam);
        if (hitTestResult == HTCLIENT)
        {
            HCURSOR cursorHandle = LoadCursor(NULL, IDC_ARROW);
            if (::GetCursor() != cursorHandle)
            {
                ::SetCursor(cursorHandle);
            }
            result = TRUE;
            return true;
        }
    }
    break;
    case WM_NCCALCSIZE:
        break;
    case WM_NCACTIVATE:
        break;
    case WM_MOUSEACTIVATE:
        break;
    }

    if (IsWindow(hwnd) != 0 && !GetClassStyle(CS_DROPSHADOW))
    {
        if (transferFocusEvent && IsFocused())
        {
            SetFocus();
        }
    }

    return false;
}

void Window::Render()
{
    if (window && window->IsVisible() && d2dRenderTarget)
    {
        bool success = d2dRenderTarget->StartRendering();
        if (!success)
            return;

        RenderInternal();
        HRESULT result = d2dRenderTarget->StopRendering();
        window->RedrawContent();

        if (FAILED(result))
        {
            if (result == D2DERR_RECREATE_TARGET)
            {
                d2dRenderTarget->ClearRenderTarget();
            }
            else
            {
                ASSERT(!"D2DERR");
            }
        }
    }
}

void Window::RenderInternal()
{
    auto bounds = CRect(CPoint(), GetClientSize());
    for (auto& e : layers)
    {
        e->GetRenderer()->SetRenderTarget(d2dRenderTarget);
        auto b = bounds;
        b.DeflateRect(e->GetRenderRect());
        e->GetRenderer()->Render(b);
    }
}

Window::HitTestResult Window::HitTest(CPoint location)
{
    return NoDecision;
}

void Window::Created()
{
    luaL_openlibs(L);
    lua_ext_register_all(L);
    luaL_loadstring(L, "require 'script.main'");
    int ret;
    if ((ret = lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        ATLTRACE(atlTraceLua, 0, "Error#%d %s\n", ret, lua_tostring(L, -1));
        ATLASSERT(!"Lua failed!");
    }
}

void Window::Moving(CRect& bounds, bool fixSizeOnly)
{
    CRect oldBounds = window->GetBounds();
    const CSize minSize(200, 200);
    CSize minWindowSize = minSize + (oldBounds.Size() - window->GetClientSize());
    if (bounds.Width() < minWindowSize.cx)
    {
        if (fixSizeOnly)
        {
            if (bounds.Width() < minWindowSize.cx)
            {
                bounds.right = bounds.left + minWindowSize.cx;
            }
        }
        else if (oldBounds.left != bounds.left)
        {
            bounds.left = oldBounds.right - minWindowSize.cx;
        }
        else if (oldBounds.right != bounds.right)
        {
            bounds.right = oldBounds.left + minWindowSize.cx;
        }
    }
    if (bounds.Height() < minWindowSize.cy)
    {
        if (fixSizeOnly)
        {
            if (bounds.Height() < minWindowSize.cy)
            {
                bounds.bottom = bounds.top + minWindowSize.cy;
            }
        }
        else if (oldBounds.top != bounds.top)
        {
            bounds.top = oldBounds.bottom - minWindowSize.cy;
        }
        else if (oldBounds.bottom != bounds.bottom)
        {
            bounds.bottom = oldBounds.top + minWindowSize.cy;
        }
    }
}

void Window::Moved()
{
    if (d2dRenderTarget->RecreateRenderTarget(window->GetClientSize()))
        Render();
}

void Window::Enabled()
{

}

void Window::Disabled()
{

}

void Window::GotFocus()
{

}

void Window::LostFocus()
{

}

void Window::Activated()
{

}

void Window::Deactivated()
{

}

void Window::Opened()
{

}

void Window::Closing(bool& cancel)
{

}

void Window::Closed()
{

}

void Window::Paint()
{

}

void Window::Destroying()
{

}

void Window::Destroyed()
{

}

void Window::LeftButtonDown(const MouseInfo& info)
{

}

void Window::LeftButtonUp(const MouseInfo& info)
{

}

void Window::LeftButtonDoubleClick(const MouseInfo& info)
{

}

void Window::RightButtonDown(const MouseInfo& info)
{

}

void Window::RightButtonUp(const MouseInfo& info)
{

}

void Window::RightButtonDoubleClick(const MouseInfo& info)
{

}

void Window::MiddleButtonDown(const MouseInfo& info)
{

}

void Window::MiddleButtonUp(const MouseInfo& info)
{

}

void Window::MiddleButtonDoubleClick(const MouseInfo& info)
{

}

void Window::HorizontalWheel(const MouseInfo& info)
{

}

void Window::VerticalWheel(const MouseInfo& info)
{

}

void Window::MouseMoving(const MouseInfo& info)
{

}

void Window::MouseEntered()
{

}

void Window::MouseLeaved()
{

}

void Window::KeyDown(const KeyInfo& info)
{

}

void Window::KeyUp(const KeyInfo& info)
{

}

void Window::SysKeyDown(const KeyInfo& info)
{

}

void Window::SysKeyUp(const KeyInfo& info)
{

}

void Window::Char(const CharInfo& info)
{

}