#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include "WindowClass.h"
#include "WindowMsgLoop.h"
#include <WTF/RefPtr.h>
#include <WTF/refcounted.h>
#include <render/Direct2DRenderTarget.h>
#include <render/Direct2DRender.h>

class CSize;
class CRect;

struct MouseInfo
{
    bool ctrl;
    bool shift;
    bool left;
    bool middle;
    bool right;
    CPoint pt;
    cint wheel;
    bool nonClient;
};

struct KeyInfo
{
    cint code;
    bool ctrl;
    bool shift;
    bool alt;
    bool capslock;
};

enum WindowEvent
{
    WE_Null = 0,
    WE_Created = 1,
    WE_Moving,
    WE_Moved,
    WE_Enabled,
    WE_Disabled,
    WE_GotFocus,
    WE_LostFocus,
    WE_Activated,
    WE_Deactivated,
    WE_Opened,
    WE_Closing,
    WE_Closed,
    WE_Paint,
    WE_Destroying,
    WE_Destroyed,
    WE_Timer = 100,
    WE_LeftButtonDown = 200,
    WE_LeftButtonUp,
    WE_LeftButtonDoubleClick,
    WE_RightButtonDown,
    WE_RightButtonUp,
    WE_RightButtonDoubleClick,
    WE_MiddleButtonDown,
    WE_MiddleButtonUp,
    WE_MiddleButtonDoubleClick,
    WE_HorizontalWheel,
    WE_VerticalWheel,
    WE_MouseMoving,
    WE_MouseEntered,
    WE_MouseLeaved,
    WE_KeyDown = 300,
    WE_KeyUp,
    WE_SysKeyDown,
    WE_SysKeyUp,
    WE_Char,
};

class Window : public RefCounted<Window>
{
public:
    Window(HWND parent, CString className, CString windowTitle, HINSTANCE hInstance);
    ~Window();

    void Run();
    void Center();

    enum WindowSizeState
    {
        Minimized,
        Restored,
        Maximized,
    };

    enum HitTestResult
    {
        BorderNoSizing,
        BorderLeft,
        BorderRight,
        BorderTop,
        BorderBottom,
        BorderLeftTop,
        BorderRightTop,
        BorderLeftBottom,
        BorderRightBottom,
        Title,
        ButtonMinimum,
        ButtonMaximum,
        ButtonClose,
        Client,
        Icon,
        NoDecision,
    };

    bool HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
    HWND GetWindowHandle() const;
    PassRefPtr<Direct2DRenderTarget> GetD2DRenderTarget();
    CRect GetBounds();
    void SetBounds(const CRect& bounds);
    CSize GetClientSize();
    void SetClientSize(CSize size);
    CRect GetClientBoundsInScreen();
    CSize GetClientWindowSize();
    CString GetTitle();
    void SetTitle(CString _title);
    WindowSizeState GetSizeState();
    void Show();
    void Show(int nCmdShow);
    void ShowDeactivated();
    void ShowRestored();
    void ShowMaximized();
    void ShowMinimized();
    void Hide();
    bool IsVisible();
    void Enable();
    void Disable();
    bool IsEnabled();
    void SetFocus();
    bool IsFocused();
    void SetActivate();
    bool IsActivated();
    void ShowInTaskBar();
    void HideInTaskBar();
    bool IsAppearedInTaskBar();
    void EnableActivate();
    void DisableActivate();
    bool IsEnabledActivate();
    bool RequireCapture();
    bool ReleaseCapture();
    bool IsCapturing();
    bool GetMaximizedBox();
    void SetMaximizedBox(bool visible);
    bool GetMinimizedBox();
    void SetMinimizedBox(bool visible);
    bool GetBorder();
    void SetBorder(bool visible);
    bool GetSizeBox();
    void SetSizeBox(bool visible);
    bool GetIconVisible();
    void SetIconVisible(bool visible);
    bool GetTitleBar();
    void SetTitleBar(bool visible);
    bool GetTopMost();
    void SetTopMost(bool topmost);
    void RedrawContent();
    bool GetExStyle(DWORD exStyle);
    void SetExStyle(DWORD exStyle, bool available);
    bool GetStyle(DWORD style);
    void SetStyle(DWORD style, bool available);
    bool GetClassStyle(DWORD style);
    void SetClassStyle(DWORD style, bool available);
    void SetTimer(cint id, cint elapse);
    void KillTimer(cint id);
    void Redraw();

protected:
    DWORD InternalGetExStyle();
    void InternalSetExStyle(DWORD exStyle);
    MouseInfo ConvertMouse(WPARAM wParam, LPARAM lParam, bool wheelMessage, bool nonClient);
    KeyInfo ConvertKey(WPARAM wParam, LPARAM lParam);
    void TrackMouse(bool enable);
    bool HandleMessageInternal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
    void Render();
    void RenderInternal();

    HitTestResult HitTest(CPoint location);
    void Created();
    void Moving(CRect& bounds, bool fixSizeOnly);
    void Moved();
    void Enabled();
    void Disabled();
    void GotFocus();
    void LostFocus();
    void Activated();
    void Deactivated();
    void Opened();
    void Closing(bool& cancel);
    void Closed();
    void Paint();
    void Destroying();
    void Destroyed();

    void LeftButtonDown(const MouseInfo& info);
    void LeftButtonUp(const MouseInfo& info);
    void LeftButtonDoubleClick(const MouseInfo& info);
    void RightButtonDown(const MouseInfo& info);
    void RightButtonUp(const MouseInfo& info);
    void RightButtonDoubleClick(const MouseInfo& info);
    void MiddleButtonDown(const MouseInfo& info);
    void MiddleButtonUp(const MouseInfo& info);
    void MiddleButtonDoubleClick(const MouseInfo& info);
    void HorizontalWheel(const MouseInfo& info);
    void VerticalWheel(const MouseInfo& info);
    void MouseMoving(const MouseInfo& info);
    void MouseEntered();
    void MouseLeaved();

    void KeyDown(const KeyInfo& info);
    void KeyUp(const KeyInfo& info);
    void SysKeyDown(const KeyInfo& info);
    void SysKeyUp(const KeyInfo& info);
    void Char(const KeyInfo& info);

    void Timer(cint id);

    friend int ui_clear_scene(lua_State *L);
    friend int ui_add_obj(lua_State *L);
    friend int ui_update_obj(lua_State *L);

protected:
    HWND handle;
    CString title;
    CPoint mouseLast;
    WindowClass wndClass;
    WindowMsgLoop winMsgLoop;
    cint mouseHoving;
    RefPtr<Direct2DRenderTarget> d2dRenderTarget;
    std::set<cint> setTimer;

private:
    std::vector<RefPtr<IGraphicsElement>> layers;
    lua_State *L;
};

extern Window *window;

#endif