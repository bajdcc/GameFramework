#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

DrawSceneBag bag;
Draw3DBag PhysicsEngine::bag3d;

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
    bag3d.camera_pos *= 0;
    bag3d.sphere_pos *= 0;
    bag3d.rotate_front = vector3(0, 0, -1);
    bag3d.rotate_up = vector3(0, 1, 0);
    bag3d.rotate_left = CrossProduct(bag3d.rotate_front, bag3d.rotate_up);
    bag3d.fov = 45.0f;
    bag3d.scale = 0.1f;
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
    if (value <= -1)
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
    if (value & 0x1000)
    {
        if ((type >= 11 && type <= 14) || (type >= 2 && type <= 4))
            painted = false;
        auto k = value & 0xfff;
        switch (k) {
        case 'w':
            bag3d.camera_pos += bag3d.scale * bag3d.rotate_up;
            break;
        case 'a':
            bag3d.camera_pos -= bag3d.scale * bag3d.rotate_left;
            break;
        case 's':
            bag3d.camera_pos -= bag3d.scale * bag3d.rotate_up;
            break;
        case 'd':
            bag3d.camera_pos += bag3d.scale * bag3d.rotate_left;
            break;
        case 'q':
            bag3d.camera_pos += bag3d.scale * bag3d.rotate_front;
            break;
        case 'e':
            bag3d.camera_pos -= bag3d.scale * bag3d.rotate_front;
            break;
        case 't':
            bag3d.sphere_pos.y += bag3d.scale;
            break;
        case 'f':
            bag3d.sphere_pos.x -= bag3d.scale;
            break;
        case 'g':
            bag3d.sphere_pos.y -= bag3d.scale;
            break;
        case 'h':
            bag3d.sphere_pos.x += bag3d.scale;
            break;
        case 'r':
            bag3d.sphere_pos.z -= bag3d.scale;
            break;
        case 'y':
            bag3d.sphere_pos.z += bag3d.scale;
            break;
        case ',':
            bag3d.scale *= 2.0f;
            break;
        case '.':
            bag3d.scale = 0.1f;
            break;
        case 'i':
            bag3d.rotate_front = Normalize(Rotate(bag3d.rotate_left, bag3d.rotate_front, -bag3d.scale));
            bag3d.rotate_up = Normalize(CrossProduct(bag3d.rotate_left, bag3d.rotate_front));
            break;
        case 'j':
            bag3d.rotate_front = Normalize(Rotate(bag3d.rotate_up, bag3d.rotate_front, -bag3d.scale));
            bag3d.rotate_left = Normalize(CrossProduct(bag3d.rotate_front, bag3d.rotate_up));
            break;
        case 'k':
            bag3d.rotate_front = Normalize(Rotate(bag3d.rotate_left, bag3d.rotate_front, bag3d.scale));
            bag3d.rotate_up = Normalize(CrossProduct(bag3d.rotate_left, bag3d.rotate_front));
            break;
        case 'l':
            bag3d.rotate_front = Normalize(Rotate(bag3d.rotate_up, bag3d.rotate_front, bag3d.scale));
            bag3d.rotate_left = Normalize(CrossProduct(bag3d.rotate_front, bag3d.rotate_up));
            break;
        case 'u':
            bag3d.rotate_up = Normalize(Rotate(bag3d.rotate_front, bag3d.rotate_up, -bag3d.scale));
            bag3d.rotate_left = Normalize(CrossProduct(bag3d.rotate_front, bag3d.rotate_up));
            break;
        case 'o':
            bag3d.rotate_up = Normalize(Rotate(bag3d.rotate_front, bag3d.rotate_up, bag3d.scale));
            bag3d.rotate_left = Normalize(CrossProduct(bag3d.rotate_front, bag3d.rotate_up));
            break;
        case 'z':
            bag3d.fov -= 1.0f;
            break;
        case 'x':
            bag3d.fov += 1.0f;
            break;
        default:
            break;
        }
        return 1;
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
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
        }
    }
    else {
        if (locked)
            return -1;
        type = value;
        if (value != 0) {
            painted = false;
            bag3d.fov = 45.0f;
            if (type == 14) {
                bag3d.rotate_front = vector3(0.0f, -1.25f, -1.0f);
                bag3d.rotate_up = vector3(0, 1, 0);
                bag3d.rotate_left = CrossProduct(bag3d.rotate_front, bag3d.rotate_up);
            }
            else {
                bag3d.rotate_front = vector3(0, 0, -1);
                bag3d.rotate_up = vector3(0, 1, 0);
                bag3d.rotate_left = CrossProduct(bag3d.rotate_front, bag3d.rotate_up);
                if (type != 2)
                    bag3d.camera_pos.z = 10;
            }
        }
    }
    return 0;
}

void PhysicsEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{

}

void PhysicsEngine::RenderSingleBitmap(CComPtr<ID2D1RenderTarget> rt, CRect bounds, void(*callback)(BYTE*, int, int))
{
    if (bounds.Width() < 10 || bounds.Height() < 10)
        return;
    if (painted)
    {
        auto _w = bounds.Width(), _h = bounds.Height();
        if (bounds.Width() * _render_asp_h < bounds.Height() * _render_asp_w) {
            _h = bounds.Width() * _render_asp_h / _render_asp_w;
        }
        else {
            _w = bounds.Height() * _render_asp_w / _render_asp_h;
        }
        // 画渲染好的位图
        rt->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + _w, (FLOAT)bounds.top + _h),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        return;
    }
    auto _w = bounds.Width(), _h = bounds.Height();
    _render_asp_w = bounds.Width();
    _render_asp_h = bounds.Height();
    auto _rt = d2drt.lock();
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
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + _w, (FLOAT)bounds.top + _h),
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
    if (bounds.Width() < 10 || bounds.Height() < 10)
        return;
    if (painted)
    {
        auto _w = bounds.Width(), _h = bounds.Height();
        if (bounds.Width() * _render_asp_h < bounds.Height() * _render_asp_w) {
            _h = bounds.Width() * _render_asp_h / _render_asp_w;
        }
        else {
            _w = bounds.Height() * _render_asp_w / _render_asp_h;
        }
        // 画渲染好的位图
        rt->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + _w / 2, (FLOAT)bounds.top + _h),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        rt->DrawBitmap(
            bitmap2,
            D2D1::RectF((FLOAT)bounds.left + _w / 2, (FLOAT)bounds.top, (FLOAT)bounds.left + _w, (FLOAT)bounds.top + _h),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        return;
    }
    auto _w = bounds.Width(), _h = bounds.Height();
    _render_asp_w = bounds.Width();
    _render_asp_h = bounds.Height();
    auto _rt = d2drt.lock();
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
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + _w / 2, (FLOAT)bounds.top + _h),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR // 线性即可
    );

    bitmap2 = _rt->GetBitmapFromWIC(wic);
    bitmap2->CopyFromMemory(&d2dRect, buffer2, rect.Width * 4);
    rt->DrawBitmap(
        bitmap2,
        D2D1::RectF((FLOAT)bounds.left + _w / 2, (FLOAT)bounds.top, (FLOAT)bounds.left + _w, (FLOAT)bounds.top + _h),
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
                buffer[0] = BYTE(x * 255 / width);  //B
                buffer[1] = BYTE(y * 255 / height); //G
                buffer[2] = 0;                      //R
            }
            else if (rd == 1)
            {
                buffer[0] = BYTE(x * 255 / width);  //B
                buffer[1] = 0;                      //G
                buffer[2] = BYTE(y * 255 / height); //R
            }
            else if (rd == 2)
            {
                buffer[0] = 0;                      //B
                buffer[1] = BYTE(x * 255 / width);  //G
                buffer[2] = BYTE(y * 255 / height); //R
            }
            buffer[3] = 255;                        //A
            buffer += 4;
        }
    }
}
