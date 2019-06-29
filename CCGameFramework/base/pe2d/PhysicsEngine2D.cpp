#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

DrawSceneBag bag;

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
    case 2:
        RenderSimpleSphere(rt, bounds);
        break;
    case 3:
        RenderMaterialSphere(rt, bounds);
        break;
    case 4:
        RenderReflectSphere(rt, bounds);
        break;
    case 11:
        RenderDirectionalLight(rt, bounds);
        break;
    case 12:
        RenderPointLight(rt, bounds);
        break;
    case 13:
        RenderSpotLight(rt, bounds);
        break;
    case 14:
        RenderTriLight(rt, bounds);
        break;
    case 21:
        Render2DLight(rt, bounds);
        break;
    case 22:
        Render2DSolid(rt, bounds);
        break;
    case 23:
        Render2DReflect(rt, bounds);
        break;
    case 24:
        Render2DTri(rt, bounds);
        break;
    case 25:
        Render2DRefraction(rt, bounds);
        break;
    case 26:
        Render2DFont(rt, bounds);
        break;
    case 41:
        Render2DScene1(rt, bounds);
        break;
    case 42:
        Render2DScene2(rt, bounds);
        break;
    case 43:
        Render2DScene3(rt, bounds);
        break;
    case 44:
        Render2DScene4(rt, bounds);
        break;
    case 45:
        Render2DScene5(rt, bounds);
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

int PhysicsEngine::SetType(cint value)
{
    if (value == -1)
        return painted ? 1 : 0;
    if (value >= 21 && value <= 50)
    {
        if (locked)
        {
            if (painted)
            {
                locked = false;
                return 1;
            }
            return 0;
        }
        if (painted)
        {
            painted = false;
            locked = true;
            type = value;
        }
        return 0;
    }
    if (locked)
        return -1;
    type = value;
    if (value != 0)
        painted = false;
    return 0;
}

void PhysicsEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{

}

void PhysicsEngine::RenderSingleBitmap(CComPtr<ID2D1RenderTarget> rt, CRect bounds, void(*callback)(BYTE*, int, int))
{
    if (bounds.Width() < 256 || bounds.Height() < 256)
        return;
    if (painted)
    {
        // 画渲染好的位图
        rt->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + 256, (FLOAT)bounds.top + 256),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        return;
    }
    auto _rt = d2drt.lock();
    auto _w = 256, _h = 256;
    bag.g_width = _w;
    bag.g_height = _h;
    auto wic = _rt->CreateBitmap(_w, _h);
    WICRect rect;
    rect.X = 0;
    rect.Y = 0;
    rect.Width = _w;
    rect.Height = _h;
    auto buffer = new BYTE[rect.Width * rect.Height * 4];
    auto hr = wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);

    // ----------------------------------------------------
    // 位图渲染开始
    callback(buffer, _w, _h);
    // ----------------------------------------------------

    auto d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
    bitmap = _rt->GetBitmapFromWIC(wic);
    bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
    rt->DrawBitmap(
        bitmap,
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + 256, (FLOAT)bounds.top + 256),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR // 线性即可
    );

    delete[]buffer;
    painted = true;
}

void PhysicsEngine::RenderSimpleColor(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderSimpleIntern);
}

void PhysicsEngine::RenderSimpleSphere(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    if (bounds.Width() < 256 || bounds.Height() < 256)
        return;
    if (painted)
    {
        // 画渲染好的位图
        rt->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + 256, (FLOAT)bounds.top + 256),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        rt->DrawBitmap(
            bitmap2,
            D2D1::RectF((FLOAT)bounds.left + 256, (FLOAT)bounds.top, (FLOAT)bounds.left + 512, (FLOAT)bounds.top + 256),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        return;
    }
    auto _rt = d2drt.lock();
    auto _w = 256, _h = 256;
    bag.g_width = _w;
    bag.g_height = _h;
    auto wic = _rt->CreateBitmap(_w, _h);
    WICRect rect;
    rect.X = 0;
    rect.Y = 0;
    rect.Width = _w;
    rect.Height = _h;
    auto buffer = new BYTE[rect.Width * rect.Height * 4];
    auto buffer2 = new BYTE[rect.Width * rect.Height * 4];
    auto hr = wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);

    // ----------------------------------------------------
    // 位图渲染开始
    RenderSphereIntern(buffer, buffer2, _w, _h);
    // ----------------------------------------------------

    auto d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
    bitmap = _rt->GetBitmapFromWIC(wic);
    bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
    rt->DrawBitmap(
        bitmap,
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + 256, (FLOAT)bounds.top + 256),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR // 线性即可
    );

    bitmap2 = _rt->GetBitmapFromWIC(wic);
    bitmap2->CopyFromMemory(&d2dRect, buffer2, rect.Width * 4);
    rt->DrawBitmap(
        bitmap2,
        D2D1::RectF((FLOAT)bounds.left + 256, (FLOAT)bounds.top, (FLOAT)bounds.left + 512, (FLOAT)bounds.top + 256),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR // 线性即可
    );

    delete[]buffer;
    delete[]buffer2;
    painted = true;
}

void PhysicsEngine::RenderMaterialSphere(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderMaterialIntern);
}

void PhysicsEngine::RenderReflectSphere(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderReflectIntern);
}

void PhysicsEngine::RenderDirectionalLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderDirectionalLight);
}

void PhysicsEngine::RenderPointLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderPointLight);
}

void PhysicsEngine::RenderSpotLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderSpotLight);
}

void PhysicsEngine::RenderTriLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap(rt, bounds, RenderTriLight);
}

void PhysicsEngine::RenderSimpleIntern(BYTE* buffer, cint width, cint height)
{
    const auto rd = rand() % 3;
    for (auto y = 0; y < height; y++)
    {
        for (auto x = 0; x < width; x++)
        {
            if (rd == 0)
            {
                buffer[0] = BYTE(x * 255 / width); //B
                buffer[1] = BYTE(y * 255 / height); //G
                buffer[2] = 0;                  //R
            }
            else if (rd == 1)
            {
                buffer[0] = BYTE(x * 255 / width); //B
                buffer[1] = 0;                  //G
                buffer[2] = BYTE(y * 255 / height); //R
            }
            else if (rd == 2)
            {
                buffer[0] = 0;                  //B
                buffer[1] = BYTE(x * 255 / width); //G
                buffer[2] = BYTE(y * 255 / height); //R
            }
            buffer[3] = 255; //A
            buffer += 4;
        }
    }
}
