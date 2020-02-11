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
            auto brush = rt->CreateDirect2DBrush(CColor(D2D1::ColorF::Red));
            d2dRenderTarget->FillRectangle(
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                brush
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
    m_rt.Release();
    m_bitmap.Release();
    RecreateImage();
}

void SVG2DElementRenderer::RecreateImage()
{
    auto e = element.lock();
    auto text = e->GetText();
    auto rect = e->GetRenderRect();
}

#pragma endregion SVG
