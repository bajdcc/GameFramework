#include "stdafx.h"
#include "Parser2D.h"
#include "render/Direct2DRenderTarget.h"
#include "cgui.h"
#include <ui\window\Window.h>

#define FRAME (1.0 / 30)

extern int g_argc;

void Parser2DEngine::RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderDefault(rt, bounds);
}

void Parser2DEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderByType(rt, bounds);
}

void Parser2DEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    logoFont.size = 20;
    logoFont.fontFamily = "宋体";
    logoFont.bold = false;
    logoFont.italic = false;
    logoFont.underline = false;
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
    clib::cvm::global_state.ui = this;
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

void Parser2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    reset();
    clib::cvm::global_state.ui = nullptr;
}

void Parser2DEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(logoFont); logoTF = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(logoColor); logoBrush = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(brushes.cmdFont); brushes.cmdTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(brushes.gbkFont); brushes.gbkTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(backup_font); font_format = nullptr;
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        brushes.cmdTF = newRenderTarget->CreateDirect2DTextFormat(brushes.cmdFont);
        brushes.gbkTF = newRenderTarget->CreateDirect2DTextFormat(brushes.gbkFont);
        font_format = newRenderTarget->CreateDirect2DTextFormat(font);
        d2drt = newRenderTarget;
        cur_bursh = newRenderTarget->CreateDirect2DBrush(CColor());
    }
}

int Parser2DEngine::SetType(cint value)
{
    if (value == -100) {
        paused = !paused;
        return 1;
    }
    if (value == -101) {
        clib::cgui::singleton().reset();
        reset();
        return 1;
    }
    clib::cgui::singleton().input(value);
    return 0;
}

static char* ipsf(double ips) {
    static char _ipsf[32];
    if (ips < 1e3) {
        sprintf(_ipsf, "%.1f", ips);
    }
    else if (ips < 1e6) {
        sprintf(_ipsf, "%.1fK", ips * 1e-3);
    }
    else if (ips < 1e9) {
        sprintf(_ipsf, "%.1fM", ips * 1e-6);
    }
    return _ipsf;
}

void Parser2DEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    auto now = std::chrono::system_clock::now();
    // 计算每帧时间间隔
    dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_clock).count();
    cycles += clib::cgui::singleton().reset_cycles();

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
    clib::cgui::singleton().draw(rt, bounds, brushes, paused, dt_inv * FRAME);
    if (clib::cvm::global_state.gui)
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
            clear(0xff000000); // BLACK
        }
        if (auto_fresh >= 1)
        {
            bitmap = nullptr;
            rt2->GetBitmap(&bitmap);
        }
        if (auto_fresh == 2) {
            auto_fresh = 0;
        }
        if (clib::cvm::global_state.gui_blur > 0.0f) {
            CComPtr<ID2D1Effect> gaussianBlurEffect;
            auto dev = Direct2D::Singleton().GetDirect2DDeviceContext();
            dev->CreateEffect(CLSID_D2D1GaussianBlur, &gaussianBlurEffect);
            gaussianBlurEffect->SetInput(0, bitmap);
            gaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, clib::cvm::global_state.gui_blur);
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
    CString logo(_T("脚本操作系统 clibparser @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("FPS: %2.1f IPS: %S"), inv, ipsf(ips));
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 210, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);
}
