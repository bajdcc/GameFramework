#include "stdafx.h"
#include "Window.h"
#include "WindowMsgLoop.h"
#include <lua_ext/ext.h>
#include <event2/event.h>
#include <curl/curl.h>

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

event_base* Window::get_event()
{
    return evbase;
}

struct lua_State* Window::get_state()
{
    return L;
}

Window *window;

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
    , L(luaL_newstate())
    , evbase(event_base_new())
{
    window = this;
    winMsgLoop.SetEventBase(evbase);
    curl_global_init(CURL_GLOBAL_ALL);
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
    event_base_free(evbase);
    curl_global_cleanup();
}

void Window::Init()
{
    d2dRenderTarget = std::make_shared<Direct2DRenderTarget>(shared_from_this());
    d2dRenderTarget->Init();
}

void Window::Run()
{
    Init();
    Center();
    Show();
    winMsgLoop.Run();
}

void Window::Center()
{
    CSize screenSize(GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));
    CRect windowBounds = GetBounds();
    windowBounds = CRect(windowBounds.TopLeft(), windowBounds.Size().Max(minSize + GetNonClientSize()));
    CSize size = windowBounds.Size();
    CPoint topLeft = (screenSize - size) / 2;
    SetWindowPos(handle, HWND_TOPMOST, topLeft.x, topLeft.y, size.cx, size.cy, SWP_NOZORDER);
    SetWindowPos(handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

bool Window::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
#ifdef _DEBUG
    if (uMsg != WM_PAINT &&
        uMsg != WM_SETCURSOR &&
        uMsg != WM_NCHITTEST &&
        uMsg != WM_MOUSEMOVE &&
        uMsg != WM_MOUSEHOVER &&
        uMsg != WM_NCMOUSELEAVE &&
        uMsg != WM_SYSTIMER &&
        uMsg != WM_TIMER &&
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

std::shared_ptr<Direct2DRenderTarget> Window::GetD2DRenderTarget()
{
    return d2dRenderTarget;
}

CSize Window::GetNonClientSize()
{
    CRect rtWindow, rtClient;
    GetWindowRect(handle, &rtWindow);
    GetClientRect(handle, &rtClient);
    return rtWindow.Size() - rtClient.Size();
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
    return minSize.Max(rect.Size());
}

CSize Window::GetNonClientWindowSize()
{
	CRect rect;
	GetClientRect(handle, &rect);
	return (minSize + GetNonClientSize()).Max(rect.Size());
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

void Window::SetMinSize(CSize size)
{
	minSize = size;
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

void Window::SetTimer(cint id, cint elapse)
{
    if (setTimer.find(id) != setTimer.end())
        return;
    setTimer.insert(id);
    ::SetTimer(handle, id, elapse, NULL);
}

void Window::KillTimer(cint id)
{
    if (setTimer.find(id) == setTimer.end())
        return;
    setTimer.erase(id);
    ::KillTimer(handle, id);
}

void Window::Redraw()
{
    Render();
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
    info.scan = HIWORD(lParam) & 0x01FF;
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
    // ************************************** timer
    case WM_TIMER:
        Timer(wParam);
        break;
    // ************************************** moving and sizing
    case WM_MOVING:
    case WM_SIZING:
    {
        LPRECT rawBounds = (LPRECT)lParam;
        CRect bounds(rawBounds);
        Moving(bounds);
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
            MouseMoving(info);
            if (!mouseHoving)
            {
                mouseHoving = true;
                MouseEntered();
                TrackMouse(true);
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
        MouseHover();
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
        KeyInfo info = ConvertKey(wParam, lParam);
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
            CursorType cur = arrow;
            decltype(IDC_ARROW) idc = IDC_ARROW;
            lua_getglobal(L, "CurrentCursor");
            if (lua_isnil(L, 1))
            {
                lua_pop(L, 1);
            }
            else
            {
                auto c = CursorType((cint)luaL_checkinteger(L, -1));
                switch (c)
                {
                case Window::hand:
                    idc = IDC_HAND;
                    break;
                case Window::ibeam:
                    idc = IDC_IBEAM;
                default:
                    break;
                }
            }
            HCURSOR cursorHandle = LoadCursor(NULL, idc);
            if (::GetCursor() != cursorHandle)
            {
                ::SetCursor(cursorHandle);
            }
            result = TRUE;
            return true;
        }
    }
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
    if (window && IsVisible() && d2dRenderTarget && !(d2dRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        bool success = d2dRenderTarget->StartRendering();
        if (!success)
            return;

        RenderInternal();
        HRESULT result = d2dRenderTarget->StopRendering();
        //RedrawContent();

        if (FAILED(result))
        {
            if (result == D2DERR_RECREATE_TARGET)
            {
                d2dRenderTarget->ClearRenderTarget();
            }
            else
            {
                ATLASSERT(!"D2DERR");
            }
        }
    }
}

void Window::RenderInternal()
{
    root->GetRenderer()->SetRenderTarget(d2dRenderTarget);
    root->GetRenderer()->Render(root->GetRenderRect());
}

Window::HitTestResult Window::HitTest(CPoint location)
{
    lua_getglobal(L, "CurrentHitTest");
    if (lua_isnil(L, 1))
    {
        lua_pop(L, 1);
        return NoDecision;
    }
    return HitTestResult((cint)luaL_checkinteger(L, -1));
}

static void PostNoArgLuaMsg(lua_State *L, WindowEvent evt)
{
    lua_getglobal(L, "PassEventToScene");
    lua_pushinteger(L, evt);
    lua_call(L, 1, 0);
}

static int LuaPanicHandler(lua_State *L)
{
    ATLTRACE(atlTraceLua, 0, "Error %s\n", lua_tostring(L, -1));
    ATLASSERT(!"Lua failed!");
    return 0;
}

void Window::Created()
{
    root = SolidBackgroundElement::Create();
    root->SetRenderRect(CRect(CPoint(), GetClientSize()));
    lua_atpanic(L, LuaPanicHandler);
    luaL_openlibs(L);
    lua_ext_register_all(L);
    luaL_loadstring(L, "require 'script.main'");
    lua_call(L, 0, 0);
    lua_getglobal(L, "PassEventToScene");
    lua_pushinteger(L, WE_Null);
    lua_call(L, 1, 0);
    PostNoArgLuaMsg(L, WE_Created);
}

void Window::Moving(CRect& bounds)
{
    CRect oldBounds = GetBounds();
	CSize minWindowSize = GetNonClientWindowSize();
    if (bounds.Width() < minWindowSize.cx)
    {
        bounds.right = bounds.left + minWindowSize.cx;
    }
    if (bounds.Height() < minWindowSize.cy)
    {
        bounds.bottom = bounds.top + minWindowSize.cy;
    }
    PostNoArgLuaMsg(L, WE_Moving);
}

void Window::Moved()
{
    root->SetRenderRect(CRect(CPoint(), GetClientWindowSize()));
    if (d2dRenderTarget && d2dRenderTarget->RecreateRenderTarget(GetClientWindowSize()))
        Render();
    PostNoArgLuaMsg(L, WE_Moved);
}

void Window::Enabled()
{
    PostNoArgLuaMsg(L, WE_Enabled);
}

void Window::Disabled()
{
    PostNoArgLuaMsg(L, WE_Disabled);
}

void Window::GotFocus()
{
    PostNoArgLuaMsg(L, WE_GotFocus);
}

void Window::LostFocus()
{
    PostNoArgLuaMsg(L, WE_LostFocus);
}

void Window::Activated()
{
    PostNoArgLuaMsg(L, WE_Activated);
}

void Window::Deactivated()
{
    PostNoArgLuaMsg(L, WE_Deactivated);
}

void Window::Opened()
{
    PostNoArgLuaMsg(L, WE_Opened);
}

void Window::Closing(bool& cancel)
{
    lua_getglobal(L, "PassEventToScene");
    lua_pushinteger(L, WE_Closing);
    lua_call(L, 1, 1);
    cancel = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
}

void Window::Closed()
{
    PostNoArgLuaMsg(L, WE_Closed);
}

void Window::Paint()
{
    PostNoArgLuaMsg(L, WE_Paint);
}

void Window::Destroying()
{
    PostNoArgLuaMsg(L, WE_Destroying);
}

void Window::Destroyed()
{
    root->GetRenderer()->SetRenderTarget(nullptr);
    root = nullptr;
    for (auto& timer : setTimer)
    {
        ::KillTimer(handle, timer);
    }
    setTimer.clear();
    PostNoArgLuaMsg(L, WE_Destroyed);
    if (zplay)
    {
        zplay->Stop();
        zplay->Release();
        zplay = nullptr;
        delete zplaydata;
        zplaydata = nullptr;
    }
}

static void PostMouseLuaMsg(lua_State *L, WindowEvent evt, const MouseInfo& info)
{
    lua_getglobal(L, "PassEventToScene");
    lua_pushinteger(L, evt);
    lua_pushinteger(L, info.pt.x);
    lua_pushinteger(L, info.pt.y);
    lua_newtable(L);
    lua_pushstring(L, "ctrl"); lua_pushboolean(L, info.ctrl); lua_settable(L, -3);
    lua_pushstring(L, "shift"); lua_pushboolean(L, info.shift); lua_settable(L, -3);
    lua_pushstring(L, "left"); lua_pushboolean(L, info.left); lua_settable(L, -3);
    lua_pushstring(L, "middle"); lua_pushboolean(L, info.middle); lua_settable(L, -3);
    lua_pushstring(L, "right"); lua_pushboolean(L, info.right); lua_settable(L, -3);
    lua_pushstring(L, "nonClient"); lua_pushboolean(L, info.nonClient); lua_settable(L, -3);
    lua_pushinteger(L, info.wheel);
    lua_call(L, 5, 0);
}

void Window::LeftButtonDown(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_LeftButtonDown, info);
}

void Window::LeftButtonUp(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_LeftButtonUp, info);
}

void Window::LeftButtonDoubleClick(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_LeftButtonDoubleClick, info);
}

void Window::RightButtonDown(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_RightButtonDown, info);
}

void Window::RightButtonUp(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_RightButtonUp, info);
}

void Window::RightButtonDoubleClick(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_RightButtonDoubleClick, info);
}

void Window::MiddleButtonDown(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_MiddleButtonDown, info);
}

void Window::MiddleButtonUp(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_MiddleButtonUp, info);
}

void Window::MiddleButtonDoubleClick(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_MiddleButtonDoubleClick, info);
}

void Window::HorizontalWheel(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_HorizontalWheel, info);
}

void Window::VerticalWheel(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_VerticalWheel, info);
}

void Window::MouseMoving(const MouseInfo& info)
{
    PostMouseLuaMsg(L, WE_MouseMoving, info);
}

void Window::MouseEntered()
{
    PostNoArgLuaMsg(L, WE_MouseEntered);
}

void Window::MouseLeaved()
{
    PostNoArgLuaMsg(L, WE_MouseLeaved);
}

void Window::MouseHover()
{
    PostNoArgLuaMsg(L, WE_MouseHover);
}

static void PostKeyLuaMsg(lua_State *L, WindowEvent evt, const KeyInfo& info)
{
    lua_getglobal(L, "PassEventToScene");
    lua_pushinteger(L, evt);
    lua_pushinteger(L, info.code);
    lua_pushinteger(L, info.scan);
    lua_newtable(L);
#define LUA_MOUSE_FLAG(name) lua_pushstring(L, #name); lua_pushboolean(L, info.name); lua_settable(L, -3);
    LUA_MOUSE_FLAG(ctrl);
    LUA_MOUSE_FLAG(shift);
    LUA_MOUSE_FLAG(alt);
    LUA_MOUSE_FLAG(capslock);
#undef LUA_MOUSE_FLAG
    lua_call(L, 4, 0);
}

void Window::KeyDown(const KeyInfo& info)
{
    PostKeyLuaMsg(L, WE_KeyDown, info);
}

void Window::KeyUp(const KeyInfo& info)
{
    PostKeyLuaMsg(L, WE_KeyUp, info);
}

void Window::SysKeyDown(const KeyInfo& info)
{
    PostKeyLuaMsg(L, WE_SysKeyDown, info);
}

void Window::SysKeyUp(const KeyInfo& info)
{
    PostKeyLuaMsg(L, WE_SysKeyUp, info);
}

void Window::Char(const KeyInfo& info)
{
    PostKeyLuaMsg(L, WE_Char, info);
}

void Window::Timer(cint id)
{
    lua_getglobal(L, "PassEventToScene");
    lua_pushinteger(L, WE_Timer);
    lua_pushinteger(L, id);
    lua_call(L, 2, 0);
}