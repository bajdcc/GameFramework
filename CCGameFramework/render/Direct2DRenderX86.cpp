#include "stdafx.h"
#include "Direct2DRender.h"

#pragma region X86

X86WindowElement::X86WindowElement()
{
}

X86WindowElement::~X86WindowElement()
{
}

CString X86WindowElement::GetElementTypeName()
{
    return _T("X86 Window");
}

cint X86WindowElement::GetTypeId()
{
    return X86Window;
}

CStringA X86WindowElement::GetText() const
{
    return text;
}

void X86WindowElement::SetText(CStringA value)
{
    text = value;
}

FLOAT X86WindowElement::GetOpacity() const
{
    return opacity;
}

void X86WindowElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

int X86WindowElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<X86WindowElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void X86WindowElementRenderer::CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)
{
    if (renderTarget)
    {
        auto e = element.lock();
        auto txt = e->GetText();
        if (!buffer)
        {
            if (txt.IsEmpty())
                return;
            {
                int _w, _h;
                _w = 400, _h = 300;
                auto line = new char[_w + 1];
                wic = renderTarget->CreateBitmap(_w, _h);
                data.resize(_w * _h);
                rect.X = 0;
                rect.Y = 0;
                rect.Width = _w;
                rect.Height = _h;
                buffer = new BYTE[rect.Width * rect.Height * 4];
                HRESULT hr = wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);
                if (FAILED(hr))
                    ATLASSERT(!"CopyPixels failed");
                BYTE* read = buffer;
                for (auto i = 0, k = 0; i < _h; i++)
                {
                    for (auto j = 0; j < _w; j++)
                    {
                        ((DWORD*)read)[0] = 0xFFFFFFFF;
                        read += 4;
                    }
                }
                d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
                delete[] line;
            }
        }
        if (wic)
        {
            if (!bitmap)
                bitmap = renderTarget->GetBitmapFromWIC(wic);
            bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
        }
    }
}

void X86WindowElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible && bitmap)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget.lock()->GetDirect2DRenderTarget();
        d2dRenderTarget->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            e->GetOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
    }
    GraphicsImageRenderer::Render(bounds);
}

X86WindowElementRenderer::~X86WindowElementRenderer()
{
    if (buffer) delete buffer;
}

int X86WindowElementRenderer::Refresh(int arg)
{
    if (arg == 1) buffer = nullptr;
    if (arg != 0) return -1;
    if (bitmap)
    {
        bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
    }
    return 0;
}

#pragma endregion X86
