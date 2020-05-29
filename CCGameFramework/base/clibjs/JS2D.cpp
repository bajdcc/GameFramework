#include "stdafx.h"
#include "JS2D.h"
#include "cjsgui.h"
#include "render/Direct2DRenderTarget.h"
#include <ui\window\Window.h>

#define FRAME (1.0 / 30)

extern int g_argc;

void JS2DEngine::RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderDefault(rt, bounds);
}

void JS2DEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderByType(rt, bounds);
}

void JS2DEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
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
    g_argc = 0;
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

void JS2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
}

void JS2DEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(bgColorLog); bg_log = nullptr;
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

void JS2DEngine::reset()
{
    auto_fresh = 1;
    GLOBAL_STATE.gui = false;
    rt2.Release();
    bitmap.Release();
    rect.Width = 0;
    rect.Height = 0;
    cur_bursh.Release();
}

int JS2DEngine::SetType(cint value)
{
    if (value == -100) {
        paused = !paused;
        return 1;
    }
    if (value == -101) {
        clib::cjsgui::singleton().reset();
        reset();
        return 1;
    }
    if (value == -103) {
        GLOBAL_STATE.is_logging = !GLOBAL_STATE.is_logging;
        return 1;
    }
    if (value == -102) {
        return clib::cjsgui::singleton().cursor();
    }
    if (value == -104) {
        clib::cjsgui::singleton().output();
        return 0;
    }
    if (value == -105) {
        clib::cjsgui::singleton().clear_cache();
        return 0;
    }
    if (value & 0x40000) {
        GLOBAL_STATE.mouse_x = value & 0xffff;
        return 0;
    }
    if (value & 0x80000) {
        GLOBAL_STATE.mouse_y = value & 0xffff;
        return 0;
    }
    if (value & 0x100000 && !GLOBAL_STATE.gui) {
        clib::cjsgui::singleton().hit(value & 0xffff);
        return 0;
    }
    if (value > 0 && clib::cjsgui::singleton().try_input(value)) {
        return 0;
    }
    clib::cjsgui::singleton().input_call(value);
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

void JS2DEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    auto now = std::chrono::system_clock::now();
    // 计算每帧时间间隔
    dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_clock).count();
    cycles += clib::cjsgui::singleton().reset_cycles();

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
    clib::cjsgui::singleton().draw(rt, bounds, brushes, paused, dt_inv * FRAME);
    if (GLOBAL_STATE.gui)
    {
        if (!bitmap)
        {
            if (rect.Width == 0 || rect.Height == 0) {
                auto size = bounds.Size();
                rect.Width = size.cx;
                rect.Height = size.cy;
            }
            rt2 = d2drt.lock()->CreateBitmapRenderTarget(D2D1::SizeF((float)rect.Width, (float)rect.Height));
            rt2->GetBitmap(&bitmap);
            if (!cur_bursh)
                cur_bursh = d2drt.lock()->CreateDirect2DBrush(CColor());
            rt2->BeginDraw();
            rt2->Clear(D2D1::ColorF(D2D1::ColorF::Black));
            rt2->EndDraw();
        }
        if (auto_fresh >= 1)
        {
            bitmap = nullptr;
            rt2->GetBitmap(&bitmap);
        }
        if (auto_fresh == 2) {
            auto_fresh = 0;
        }
        if (GLOBAL_STATE.gui_blur > 0.0f) {
            CComPtr<ID2D1Effect> gaussianBlurEffect;
            auto dev = Direct2D::Singleton().GetDirect2DDeviceContext();
            dev->CreateEffect(CLSID_D2D1GaussianBlur, &gaussianBlurEffect);
            gaussianBlurEffect->SetInput(0, bitmap);
            gaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, GLOBAL_STATE.gui_blur);
            dev->DrawImage(
                gaussianBlurEffect.p,
                D2D1::Point2F((FLOAT)bounds.left, (FLOAT)bounds.top),
                D2D1::RectF(0.0f, 0.0f, (FLOAT)bounds.Width(), (FLOAT)bounds.Height()),
                D2D1_INTERPOLATION_MODE_LINEAR
            );
        }
        else
            rt->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
            );
    }
    else if (bitmap) {
        reset();
    }

    CString logo(_T("JavaScript解释器 clibjs @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("屏幕（%d）"), clib::cjsgui::singleton().current_screen());
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 35, (float)bounds.left + 200, (float)bounds.top + 60), logoBrush);

    logo.Format(_T("FPS: %2.1f IPS: %S"), inv, ipsf(ips));
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 210, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);

    logo = clib::cjsgui::singleton().get_disp(clib::types::D_STAT);
    if (!logo.IsEmpty()) {
        auto line = int(logo[0] - L'0');
        rt->DrawText(logo.GetBuffer(0) + 1, logo.GetLength() - 1, loggingTF->textFormat,
            D2D1::RectF((float)bounds.left + 10, (float)bounds.bottom - (line)*brushes.gbkFont.size, (float)bounds.left + 200, (float)bounds.bottom), logoBrush);
    }

    if (GLOBAL_STATE.is_logging) {
        const int span = loggingFont.size;
        const int wspan = brushes.gbkFont.size;
        auto R = D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 10, (float)bounds.right - 10, (float)bounds.top + 60);
        rt->FillRectangle(
            D2D1::RectF((float)bounds.left, (float)bounds.top, (float)bounds.right, (float)bounds.bottom),
            bg_log
        );
        for (auto& l : GLOBAL_STATE.logging) {
            rt->DrawText(l.GetBuffer(0), l.GetLength(), loggingTF->textFormat, R, logoBrush);
            R.top += span;
            R.bottom += span;
            if (R.top + span >= bounds.bottom) {
                break;
            }
        }
        R.top += span;
        R.bottom = (float)bounds.bottom;
        auto disp = clib::cjsgui::singleton().get_disp(clib::types::D_HANDLE);
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R.top += span;
        auto lines = 3;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        disp = clib::cjsgui::singleton().get_disp(clib::types::D_WINDOW);
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R = D2D1::RectF((float)bounds.right - 400, (float)bounds.top + 10, (float)bounds.right - 10, (float)bounds.bottom);
        disp = clib::cjsgui::singleton().get_disp(clib::types::D_PS);
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        lines = 3;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        auto RM = R;
        RM.top += lines * span;
        auto side = lines < 20;
        if (side)
            R.top = RM.top;
        else
            R.left -= 600;
        disp = clib::cjsgui::singleton().get_disp(clib::types::D_HTOP);
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        lines = 0;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        if (side) {
            R.top += lines * wspan + span;
        }
        else {
            R = RM;
            R.top += 3 * span;
        }
        disp = clib::cjsgui::singleton().get_disp(clib::types::D_MEM);
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
    }
}
