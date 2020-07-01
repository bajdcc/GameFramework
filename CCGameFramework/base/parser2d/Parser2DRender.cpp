#include "stdafx.h"
#include "render/Direct2DRender.h"


Parser2DElement::Parser2DElement()
{
}

Parser2DElement::~Parser2DElement()
{
}

CString Parser2DElement::GetElementTypeName()
{
    return _T("Parser2D");
}

cint Parser2DElement::GetTypeId()
{
    return Parser2D;
}

FLOAT Parser2DElement::GetOpacity() const
{
    return opacity;
}

void Parser2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint Parser2DElement::GetType() const
{
    return type;
}

void Parser2DElement::SetType(cint value)
{
    type = value;
}

int Parser2DElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<Parser2DElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void Parser2DElementRenderer::Render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = r ? r : renderTarget.lock()->GetDirect2DRenderTarget();
        engine.Render(d2dRenderTarget, bounds);
    }
    GraphicsRenderer::Render(bounds, r);
}

Parser2DElementRenderer::~Parser2DElementRenderer()
{
}

int Parser2DElementRenderer::Refresh(int arg)
{
    return engine.SetType(arg);
}

void Parser2DElementRenderer::OnElementStateChanged()
{
}

void Parser2DElementRenderer::InitializeInternal()
{
    engine.Initialize(renderTarget.lock());
}

void Parser2DElementRenderer::FinalizeInternal()
{
    engine.Finalize(renderTarget.lock());
}

void Parser2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    engine.Reset(oldRenderTarget, newRenderTarget);
}
