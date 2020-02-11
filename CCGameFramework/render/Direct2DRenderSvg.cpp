#include "stdafx.h"
#include "Direct2DRender.h"
#include <nanosvg\nanosvg.h>
#include <nanosvg\nanosvgrast.h>

#pragma region SVG
SVG2DElement::SVG2DElement()
{
}

SVG2DElement::~SVG2DElement()
{
}

CString SVG2DElement::GetElementTypeName()
{
    return _T("SVG2D");
}

cint SVG2DElement::GetTypeId()
{
    return SVG2D;
}

FLOAT SVG2DElement::GetOpacity() const
{
    return opacity;
}

void SVG2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint SVG2DElement::GetType() const
{
    return type;
}

void SVG2DElement::SetType(cint value)
{
    type = value;
}

CStringA SVG2DElement::GetText() const
{
    return text;
}

void SVG2DElement::SetText(const CStringA& value)
{
    if (text != value)
    {
        text = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

void SVG2DElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        auto rt = renderTarget.lock();
        auto d2dRenderTarget = rt->GetDirect2DRenderTarget();
        if (m_bitmap) {
            d2dRenderTarget->DrawBitmap(
                m_bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                e->GetOpacity(),
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
            );
        }
    }
    GraphicsRenderer::Render(bounds);
}

SVG2DElementRenderer::~SVG2DElementRenderer()
{
}

void SVG2DElementRenderer::OnElementStateChanged()
{
    RecreateImage();
}

void SVG2DElementRenderer::InitializeInternal()
{
}

void SVG2DElementRenderer::FinalizeInternal()
{
}

void SVG2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    RecreateImage();
}

void SVG2DElementRenderer::RecreateImage()
{
    m_bitmap.Release();
    auto e = element.lock();
    auto rt = renderTarget.lock();
    auto text = e->GetText();
    if (text.IsEmpty())return;
    auto rast = nsvgCreateRasterizer();
    auto image = nsvgParse(text.GetBuffer(0), "px", 96);
    auto w = (int)image->width;
    auto h = (int)image->height;
    e->SetRenderRect(CRect(CPoint(), CSize(w, h)));
    auto WICBitmap = rt->CreateBitmap(w, h);
    m_bitmap = rt->GetBitmapFromWIC(WICBitmap);
    auto img = new BYTE[w * h * 4];
    nsvgRasterize(rast, image, 0, 0, 1, img, w, h, w * 4);
    auto d2dRect = D2D1::RectU(0, 0, w, h);
    m_bitmap->CopyFromMemory(&d2dRect, img, w * 4);
    delete[]img;
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
}

#pragma endregion SVG
