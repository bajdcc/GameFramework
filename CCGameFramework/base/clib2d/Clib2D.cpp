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

#define MAKE_COLOR(r,g,b) CColor((BYTE)(255 * r), (BYTE)(255 * g), (BYTE)(255 * b))

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
    brushes.colors.push_back(MAKE_COLOR(0.9f, 0.9f, 0.9f));
    brushes.colors.push_back(MAKE_COLOR(0.3f, 0.3f, 0.3f));
    brushes.colors.push_back(MAKE_COLOR(0.8f, 0.8f, 0.0f));
    brushes.colors.push_back(MAKE_COLOR(0.8f, 0.2f, 0.4f));
    brushes.colors.push_back(MAKE_COLOR(0.8f, 0.2f, 0.2f));
    brushes.colors.push_back(MAKE_COLOR(0.0f, 1.0f, 0.0f));
    brushes.colors.push_back(MAKE_COLOR(0.2f, 0.2f, 0.2f));
    brushes.colors.push_back(MAKE_COLOR(0.0f, 1.0f, 0.0f));
    brushes.colors.push_back(MAKE_COLOR(0.12f, 0.12f, 0.12f));
    brushes.colors.push_back(MAKE_COLOR(0.9f, 0.7f, 0.4f));
    brushes.colors.push_back(MAKE_COLOR(0.6f, 0.6f, 0.6f));
    brushes.colors.push_back(MAKE_COLOR(1.0f, 0.2f, 0.2f));
    brushes.colors.push_back(MAKE_COLOR(0.2f, 0.5f, 0.4f));
}

#undef MAKE_COLOR

void Clib2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    delete clib::world;
}

void Clib2DEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    using namespace clib;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(logoFont); logoTF = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(logoColor); logoBrush = nullptr;
        brushes.colors.clear();
        for (int i = b_static; i < b__end; ++i) {
            oldRenderTarget->DestroyDirect2DBrush(brushes.colors[i]);
        }
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        brushes.brushes.clear();
        for (int i = b_static; i < b__end; ++i) {
            brushes.brushes.push_back(newRenderTarget->CreateDirect2DBrush(brushes.colors[i]));
        }
    }
}

// 屏幕坐标到世界坐标的变换
clib::v2 Clib2DEngine::screen2world(int x, int y) {
    return clib::c2d_world::translate(rect, x, y); // 没搞明白这个10，先用着
}

int Clib2DEngine::SetType(cint value)
{
    if (value & 0x1000)
    {
        auto key = value & 0xfff;
        clib::world->key(key);
    }
    else if (value & 0x2000)
    {
        auto key = value & 0xfff;
    }
    else if (value & 0x4000)
    {
        mouseX = value & 0xfff;
    }
    else if (value & 0x8000)
    {
        mouseY = value & 0xfff;
    }
    else if (value & 0x10000)
    {
        auto key = value & 0xfff;
        switch (key)
        {
        case 1:
            clib::world->mouse(screen2world(mouseX, mouseY), 1);
            break;
        case 2:
            clib::world->mouse(screen2world(mouseX, mouseY), 0);
            break;
        case 3:
            clib::world->motion(screen2world(mouseX, mouseY));
            break;
        default:
            break;
        }
    }
    else if (clib::world) {
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

    auto inv = 1.0 / clib::c2d_world::dt;
    if (clib::c2d_world::dt > FRAME_SPAN) {
        clib::c2d_world::dt = min(clib::c2d_world::dt, FRAME_SPAN);
        clib::c2d_world::dt_inv = 1.0 / clib::c2d_world::dt;
    }

	clib::c2d_world::last_clock = now;
	rt->FillRectangle(
		D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
		bg
	);
	clib::world->step(rt, bounds, brushes);
	rect = bounds;

    CString logo(_T("2D物理引擎系列 clib-2d @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("FPS: %.1f"), inv);
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 100, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("碰撞：%d，休眠: %d"), clib::world->get_collision_size(), clib::world->get_sleeping_size());
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 180, (float)bounds.bottom - 30, (float)bounds.right, (float)bounds.bottom), logoBrush);
}
