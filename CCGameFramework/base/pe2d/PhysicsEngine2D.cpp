#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

void PhysicsEngine::RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    if (!d2drt.lock()) return;
    // 示例参见 http://www.cnblogs.com/miloyip/archive/2010/03/29/1698953.html
    switch (type)
    {
    case 0:
        RenderDefault(rt, bounds);
        break;
    case 1:
        RenderSimpleColor(rt, bounds);
        break;
    }
}

void PhysicsEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    rt->FillRectangle(
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        bg
    );
    RenderByType(rt, bounds);
}

void PhysicsEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    bgColor = Gdiplus::Color::LightGoldenrodYellow;
}

void PhysicsEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
}

void PhysicsEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (newRenderTarget) d2drt = newRenderTarget;
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
    }
}

void PhysicsEngine::SetType(cint value)
{
    type = value;
    switch (value)
    {
    case 1:
        painted = false;
        break;
    }
}

void PhysicsEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{

}

void PhysicsEngine::RenderSimpleColor(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    if (bounds.Width() < 256 || bounds.Height() < 256)
        return;
    if (painted)
    {
        // 画渲染好的位图
        rt->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        return;
    }
    auto _rt = d2drt.lock();
    auto _w = 256, _h = 256;
    auto wic = _rt->CreateBitmap(_w, _h);
    WICRect rect;
    rect.X = 0;
    rect.Y = 0;
    rect.Width = _w;
    rect.Height = _h;
    auto buffer = new BYTE[rect.Width * rect.Height * 4];
    auto hr = wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);
    auto read = buffer;
    auto rd = rand() % 3;
    for (auto y = 0; y < _h; y++)
    {
        for (auto x = 0; x < _w; x++)
        {
            if (rd == 0)
            {
                read[0] = BYTE(x * 255 / _w); //B
                read[1] = BYTE(y * 255 / _h); //G
                read[2] = 0;                  //R
            }
            else if (rd == 1)
            {
                read[0] = BYTE(x * 255 / _w); //B
                read[1] = 0;                  //G
                read[2] = BYTE(y * 255 / _h); //R
            }
            else if (rd == 2)
            {
                read[0] = 0;                  //B
                read[1] = BYTE(x * 255 / _w); //G
                read[2] = BYTE(y * 255 / _h); //R
            }
            read[3] = 255; //A
            read += 4;
        }
    }
    auto d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
    bitmap = _rt->GetBitmapFromWIC(wic);
    bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
    delete[]buffer;
    rt->DrawBitmap(
        bitmap,
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR // 线性即可
    );
    painted = true;
}
