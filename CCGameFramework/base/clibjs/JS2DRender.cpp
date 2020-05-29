#include "stdafx.h"
#include "render/Direct2DRender.h"


JS2DElement::JS2DElement()
{
}

JS2DElement::~JS2DElement()
{
}

CString JS2DElement::GetElementTypeName()
{
    return _T("JS2D");
}

cint JS2DElement::GetTypeId()
{
    return JS2D;
}

FLOAT JS2DElement::GetOpacity() const
{
    return opacity;
}

void JS2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint JS2DElement::GetType() const
{
    return type;
}

void JS2DElement::SetType(cint value)
{
    type = value;
}

int JS2DElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<JS2DElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void JS2DElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget.lock()->GetDirect2DRenderTarget();
        engine.Render(d2dRenderTarget, bounds);
    }
    GraphicsRenderer::Render(bounds);
}

JS2DElementRenderer::~JS2DElementRenderer()
{
}

int JS2DElementRenderer::Refresh(int arg)
{
    return engine.SetType(arg);
}

void JS2DElementRenderer::OnElementStateChanged()
{
}

void JS2DElementRenderer::InitializeInternal()
{
    engine.Initialize(renderTarget.lock());
}

void JS2DElementRenderer::FinalizeInternal()
{
    engine.Finalize(renderTarget.lock());
}

void JS2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    engine.Reset(oldRenderTarget, newRenderTarget);
}
