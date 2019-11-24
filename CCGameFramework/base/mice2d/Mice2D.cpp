#include "stdafx.h"
#include "Mice2D.h"
#include "render/Direct2DRenderTarget.h"
#include <ui\window\Window.h>
#include <random>

#define FRAME (1.0f / 30.0f)
#define MICE_N 10

void Mice2DEngine::init(std::shared_ptr<Direct2DRenderTarget> rt)
{
    rt2 = rt;
    bag.brush = rt->CreateDirect2DBrush(D2D1::ColorF::Black);
    bag.random = std::default_random_engine((uint32_t)time(nullptr));
    auto& e = bag.random;
    std::uniform_real_distribution<decimal> dr{ -100.0f, 100.0f };
    std::uniform_real_distribution<decimal> da{ 0, 2.0f * (decimal)M_PI };
    std::uniform_int_distribution<int> di{ 0, 255 };
    for (auto i = 0; i < MICE_N; i++) {
        mice2d::MiceAtom atom;
        atom.id = global_id++;
        atom.pt = vector2(dr(e) * 2.0f, dr(e));
        atom.angle = da(e);
        atom.bodyF = CColor(di(e), di(e), di(e));
        atom.body = rt->CreateDirect2DBrush(atom.bodyF);
        mices.push_back(atom);
    }
    bag.tail = rt->CreatePathGeometry();
    CComPtr<ID2D1GeometrySink> pSink;
    bag.tail->Open(&pSink.p);
    pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
    pSink->BeginFigure({ 0, 20 }, D2D1_FIGURE_BEGIN_HOLLOW);
    pSink->AddBezier({ { -5, 22 },{ -5, 22 },{ 0, 25 } });
    pSink->AddBezier({ { 5, 27 },{ 5, 32 },{ 0, 30 } });
    pSink->AddBezier({ { -5, 32 },{ -5, 42 },{ 0, 35 } });
    pSink->EndFigure(D2D1_FIGURE_END_OPEN);
    pSink->Close();
}

void Mice2DEngine::destroy(std::shared_ptr<Direct2DRenderTarget> rt)
{
    for (auto& mice : mices) {
        rt->DestroyDirect2DBrush(mice.bodyF);
        mice.body = nullptr;
    }
    mices.clear();
    bag.brush.Release();
    bag.tail.Release();
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
    bgColor = CColor(253, 237, 159);
    bgColorLog = CColor(128, 128, 128, 200);
    logoColor = CColor(0, 0, 0, 240);
    last_clock = std::chrono::system_clock::now();
    dt = 30.0f;
    dt_inv = 1.0f / dt;
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
        oldRenderTarget->DestroyDirect2DBrush(bgColorLog); bg_log = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(logoFont); logoTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(loggingFont); loggingTF = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(logoColor); logoBrush = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(backup_font); font_format = nullptr;
        destroy(oldRenderTarget);
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        bg_log = newRenderTarget->CreateDirect2DBrush(bgColorLog);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        loggingTF = newRenderTarget->CreateDirect2DTextFormat(loggingFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        font_format = newRenderTarget->CreateDirect2DTextFormat(font);
        init(newRenderTarget);
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

static char* ipsf(decimal ips) {
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
    dt = std::chrono::duration_cast<std::chrono::duration<decimal>>(now - last_clock).count();
    cycles++;

    auto inv = 1.0f / dt;
    if (dt > FRAME) {
        ips = cycles * dt;
        cycles = 0;
        dt = min(dt, FRAME);
        dt_inv = 1.0f / dt;
        last_clock = now;
        tick(bounds);
    }

    rt->FillRectangle(
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        bg
    );
    
    draw(rt, bounds, dt_inv * FRAME);

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
        std::wstringstream wss;
        {
            TCHAR buf[255];
            wss << L"-- Mice Information --" << std::endl;
            for (auto& mice : mices) {
                _snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"Mice #%2d: x= %4.2f, y= %4.2f, atan= %1.2f, angle= %1.2f, angleF= %1.2f, speed= %1.2f, ret= %1d",
                    mice.id, mice.pt.x, mice.pt.y, mice.angleToCenter * M_1_PI * 180.0f,
                    mice.angle * M_1_PI * 180.0f, mice.angleF, mice.speedF, mice.needReturn);
                wss << buf << std::endl;
            }
        }
        auto disp = CString(wss.str().c_str());
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R.top += span;
        auto lines = 1;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        {
            wss.str(L"");
            TCHAR buf[255];
            wss << L"-- Environment --" << std::endl;
            auto size = bounds.Size();
            _snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"Screen= (%4d, %4d)", (int)size.cx, (int)size.cy);
            wss << buf << std::endl;
        }
        disp = CString(wss.str().c_str());
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

void Mice2DEngine::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, decimal fps) {
    if (!paused) {
        for (auto& mice : mices) {
            mice.draw(rt, bounds, bag);
        }
    }
}

void Mice2DEngine::tick(const CRect& bounds)
{
    if (!paused) {
        for (auto& mice : mices) {
            mice.tick(dt, bounds, bag);
        }
    }
}

void Mice2DEngine::reset()
{
    auto rt = rt2.lock();
    if (!rt) return;
    destroy(rt);
    init(rt);
}
