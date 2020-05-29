#include "stdafx.h"
#include "JS2D.h"
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

int JS2DEngine::SetType(cint value)
{
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
}
