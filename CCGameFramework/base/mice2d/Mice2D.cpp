#include "stdafx.h"
#include "Mice2D.h"
#include "render/Direct2DRenderTarget.h"
#include <ui\window\Window.h>

#define FRAME (1.0 / 30)


// 光栅化算法

bool Mice2DEngine::check_cord(int x, int y) const
{
    return bitmap && x >= 0 && y >= 0 && x < rect.Width && y < rect.Height;
}

bool Mice2DEngine::ready() const
{
    return bitmap != nullptr;
}

void Mice2DEngine::move_to(int x, int y)
{
    if (check_cord(x, y)) {
        cur_pt.x = x;
        cur_pt.y = y;
    }
}

// CHECKED X, Y
bool Mice2DEngine::setpixel(int x, int y) {
    if (!bitmap) return false;
    rt2->BeginDraw();
    rt2->DrawRectangle(D2D1::RectF((FLOAT)x, (FLOAT)y, (FLOAT)x, (FLOAT)y), cur_bursh);
    rt2->EndDraw();
    return true;
}

// 画直线
// Refer: https://zhuanlan.zhihu.com/p/30553006
// Modified from https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
void Mice2DEngine::bresenham(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (setpixel(x0, y0), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void Mice2DEngine::line_to(int x, int y)
{
    if (check_cord(x, y)) {
        rt2->BeginDraw();
        rt2->DrawLine(D2D1::Point2F((FLOAT)cur_pt.x, (FLOAT)cur_pt.y),
            D2D1::Point2F((FLOAT)x, (FLOAT)y), cur_bursh);
        rt2->EndDraw();
        cur_pt.x = x;
        cur_pt.y = y;
    }
}

void Mice2DEngine::draw_point(int x, int y)
{
    if (check_cord(x, y)) {
        setpixel(x, y);
    }
}

int Mice2DEngine::get_width() const
{
    return rect.Width;
}

int Mice2DEngine::get_height() const
{
    return rect.Height;
}

void Mice2DEngine::set_color(uint c)
{
    if (!bitmap) return;
    cur_bursh = nullptr;
    auto hr = rt2->CreateSolidColorBrush(D2D1::ColorF(c & 0xffffff, ((FLOAT)(c >> 24)) / 255.0f), &cur_bursh);
    if (FAILED(hr))
        ATLVERIFY(!"CreateSolidColorBrush failed");
}

void Mice2DEngine::clear(uint c)
{
    if (!bitmap) return;
    rt2->BeginDraw();
    rt2->Clear(D2D1::ColorF(c & 0xffffff, ((FLOAT)(c >> 24)) / 255.0f));
    rt2->EndDraw();
}

void Mice2DEngine::fill_rect(int x, int y)
{
    if (check_cord(x, y)) {
        rt2->BeginDraw();
        rt2->FillRectangle(D2D1::RectF((FLOAT)cur_pt.x, (FLOAT)cur_pt.y, (FLOAT)x + 1, (FLOAT)y + 1), cur_bursh);
        rt2->EndDraw();
        cur_pt.x = x;
        cur_pt.y = y;
    }
}

int Mice2DEngine::set_fresh(int fresh)
{
    if (fresh == -1) return auto_fresh;
    auto_fresh = fresh;
    return fresh;
}

void Mice2DEngine::reset()
{
    auto_fresh = 1;
    rt2.Release();
    bitmap.Release();
    rect.Width = 0;
    rect.Height = 0;
    cur_bursh.Release();
}

void Mice2DEngine::create_font()
{
    if (d2drt.lock() && font.size > 0 && !font.fontFamily.IsEmpty()) {
        auto d = d2drt.lock();
        d->DestroyDirect2DTextFormat(backup_font);
        font_format = d->CreateDirect2DTextFormat(font);
        backup_font.fontFamily = font.fontFamily;
        backup_font.size = font.size;
        backup_font.bold = font.bold;
        backup_font.italic = font.italic;
        backup_font.underline = font.underline;
        backup_font.strikeline = font.strikeline;
        backup_font.antialias = font.antialias;
        backup_font.verticalAntialias = font.verticalAntialias;
    }
}

void Mice2DEngine::set_font_size(int size)
{
    if (size > 0 && size < 100)
        font.size = size;
}

void Mice2DEngine::set_font_family(const string_t& name)
{
    if (!name.empty())
        font.fontFamily = name.c_str();
}

void Mice2DEngine::draw_font(const string_t& text)
{
    if (!bitmap) return;
    CString t(CStringA(text.c_str()));
    rt2->BeginDraw();
    rt2->DrawText(t.GetBuffer(0), t.GetLength(), font_format->textFormat,
        D2D1::RectF((float)cur_pt.x, (float)cur_pt.y,
        (float)rect.Width, (float)rect.Height), cur_bursh);
    rt2->EndDraw();
}

void Mice2DEngine::RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderDefault(rt, bounds);
}

void Mice2DEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderByType(rt, bounds);
}

void Mice2DEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    logoFont.size = 20;
    logoFont.fontFamily = "宋体";
    logoFont.bold = false;
    logoFont.italic = false;
    logoFont.underline = false;
    loggingFont.size = 12;
    loggingFont.fontFamily = "Courier New";
    loggingFont.bold = false;
    loggingFont.italic = false;
    loggingFont.underline = false;
    bgColorLog = CColor(0, 0, 0, 220);
    brushes.cmdFont.size = 16;
    brushes.cmdFont.fontFamily = "Courier New";
    brushes.cmdFont.bold = false;
    brushes.cmdFont.italic = false;
    brushes.cmdFont.underline = false;
    brushes.gbkFont.size = 16;
    brushes.gbkFont.fontFamily = "楷体";
    brushes.gbkFont.bold = false;
    brushes.gbkFont.italic = false;
    brushes.gbkFont.underline = false;
    logoColor = CColor(255, 255, 255);
    last_clock = std::chrono::system_clock::now();
    dt = 30;
    dt_inv = 1.0 / dt;
    rect.X = 0;
    rect.Y = 0;
    font.size = 20;
    font.fontFamily = "宋体";
    font.bold = false;
    font.italic = false;
    font.underline = false;
    backup_font.size = 20;
    backup_font.fontFamily = "宋体";
    backup_font.bold = false;
    backup_font.italic = false;
    backup_font.underline = false;
}

void Mice2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    reset();
}

void Mice2DEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(bgColorLog); bg = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(logoFont); logoTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(loggingFont); loggingTF = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(logoColor); logoBrush = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(brushes.cmdFont); brushes.cmdTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(brushes.gbkFont); brushes.gbkTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(backup_font); font_format = nullptr;
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        bg_log = newRenderTarget->CreateDirect2DBrush(bgColorLog);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        loggingTF = newRenderTarget->CreateDirect2DTextFormat(loggingFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        brushes.cmdTF = newRenderTarget->CreateDirect2DTextFormat(brushes.cmdFont);
        brushes.gbkTF = newRenderTarget->CreateDirect2DTextFormat(brushes.gbkFont);
        font_format = newRenderTarget->CreateDirect2DTextFormat(font);
        d2drt = newRenderTarget;
        cur_bursh = newRenderTarget->CreateDirect2DBrush(CColor());
    }
}

int Mice2DEngine::SetType(cint value)
{
    if (value == -100) {
        paused = !paused;
        return 1;
    }
    if (value == -101) {
        reset();
        return 1;
    }
    if (value == -103) {
        global_state.is_logging = !global_state.is_logging;
        return 1;
    }
    if (value == -102) {
        return 1;
    }
    if (value & 0x40000) {
        global_state.mouse_x = value & 0xffff;
        return 0;
    }
    if (value & 0x80000) {
        global_state.mouse_y = value & 0xffff;
        return 0;
    }
    if (value & 0x100000) {
        // hit(value & 0xffff)
        return 0;
    }
    // input(value)
    return 0;
}

static char* ipsf(double ips) {
    static char _ipsf[32];
    if (ips < 1e3) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1f", ips);
    }
    else if (ips < 1e6) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1fK", ips * 1e-3);
    }
    else if (ips < 1e9) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1fM", ips * 1e-6);
    }
    return _ipsf;
}

void Mice2DEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    auto now = std::chrono::system_clock::now();
    // 计算每帧时间间隔
    dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_clock).count();
    cycles++;

    auto inv = 1.0 / dt;
    if (dt > FRAME) {
        ips = cycles * dt;
        cycles = 0;
        dt = min(dt, FRAME);
        dt_inv = 1.0 / dt;
        last_clock = now;
    }

    rt->FillRectangle(
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        bg
    );
    // draw(rt, bounds, brushes, paused, dt_inv * FRAME)

    CString logo(_T("找食游戏 @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("FPS: %2.1f Turn: %S"), inv, ipsf(ips));
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 210, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);

    if (global_state.is_logging) {
        const int span = 12;
        auto R = D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 10, (float)bounds.right - 10, (float)bounds.top + 60);
        rt->FillRectangle(
            D2D1::RectF((float)bounds.left, (float)bounds.top, (float)bounds.right, (float)bounds.bottom),
            bg_log
        );
        for (auto& l : global_state.logging) {
            rt->DrawText(l.GetBuffer(0), l.GetLength(), loggingTF->textFormat, R, logoBrush);
            R.top += span;
            R.bottom += span;
            if (R.top + span >= bounds.bottom) {
                break;
            }
        }
        R.top += span;
        R.bottom = (float)bounds.bottom;
        auto disp = CString(_T("显示1"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R.top += span;
        auto lines = 1;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        disp = CString(_T("显示2"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R = D2D1::RectF((float)bounds.right - 400, (float)bounds.top + 10, (float)bounds.right - 10, (float)bounds.bottom);
        disp = CString(_T("显示3"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        lines = 3;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        disp = CString(_T("显示4"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        lines = 3;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        disp = CString(_T("显示5"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
    }
}
