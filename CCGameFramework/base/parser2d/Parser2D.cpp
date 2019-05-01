#include "stdafx.h"
#include "Parser2D.h"
#include "render/Direct2DRenderTarget.h"
#include "cgui.h"

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
    logoColor = CColor(255, 255, 255);
    last_clock = std::chrono::system_clock::now();
    dt = 30;
    dt_inv = 1.0 / dt;
    g_argc = 0;
}

void Parser2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
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
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        brushes.cmdTF = newRenderTarget->CreateDirect2DTextFormat(brushes.cmdFont);
    }
}

int Parser2DEngine::SetType(cint value)
{
    if (value == -100) {
        paused = !paused;
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

    if (dt > FRAME) {
        ips = cycles * dt;
        cycles = 0;
        dt_inv = 1.0 / dt;
        last_clock = now;
        rt->FillRectangle(
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            bg
        );
        clib::cgui::singleton().draw(rt, bounds, brushes, paused, dt_inv * FRAME);
    }

    CString logo(_T("脚本操作系统 clibparser @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("IPS: %S FPS: %.1f"), ipsf(ips), dt_inv);
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 200, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);
}
