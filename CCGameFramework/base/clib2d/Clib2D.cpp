#include "stdafx.h"
#include "Clib2D.h"
#include "render/Direct2DRenderTarget.h"
#include "c2dworld.h"

void Clib2DEngine::RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderDefault(rt, bounds);
}

void Clib2DEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderByType(rt, bounds);
}

void Clib2DEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    clib::world = new clib::c2d_world();
    clib::world->init(); // 初始化
    logoFont.size = 20;
    logoFont.fontFamily = "宋体";
    logoFont.bold = false;
    logoFont.italic = false;
    logoFont.underline = false;
    logoColor = CColor(255, 255, 255);
    brushes.static_body_color = CColor(225, 225, 225);
}

void Clib2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    delete clib::world;
}

void Clib2DEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(logoFont); logoTF = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(logoColor); logoBrush = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(brushes.static_body_color);
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        brushes.static_body = newRenderTarget->CreateDirect2DBrush(brushes.static_body_color);
    }
}

int Clib2DEngine::SetType(cint value)
{
    if (clib::world) {
        clib::world->scene(value);
    }
    return 0;
}

const clib::BrushBag& Clib2DEngine::GetBrushBag() const
{
    return brushes;
}

void Clib2DEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    auto now = std::chrono::system_clock::now();
    // 计算每帧时间间隔
    clib::c2d_world::dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - clib::c2d_world::last_clock).count();

    if (clib::c2d_world::dt > FRAME_SPAN) {
        clib::c2d_world::dt = min(clib::c2d_world::dt, FRAME_SPAN);
        clib::c2d_world::dt_inv = 1.0 / clib::c2d_world::dt;
        clib::c2d_world::last_clock = now;
        rt->FillRectangle(
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            bg
        );
        clib::world->step(rt, bounds, brushes);
    }

    CString logo(_T("2D物理引擎系列 clib-2d @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("FPS: %.1f"), clib::c2d_world::dt_inv);
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 100, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("碰撞：%d，休眠: %d"), clib::world->get_collision_size(), clib::world->get_sleeping_size());
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 180, (float)bounds.bottom - 30, (float)bounds.right, (float)bounds.bottom), logoBrush);
}
