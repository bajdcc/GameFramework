#include "stdafx.h"
#include "render/Direct2DRender.h"


Mice2DElement::Mice2DElement()
{
}

Mice2DElement::~Mice2DElement()
{
}

CString Mice2DElement::GetElementTypeName()
{
    return _T("Mice2D");
}

cint Mice2DElement::GetTypeId()
{
    return Mice2D;
}

FLOAT Mice2DElement::GetOpacity() const
{
    return opacity;
}

void Mice2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint Mice2DElement::GetType() const
{
    return type;
}

void Mice2DElement::SetType(cint value)
{
    type = value;
}

int Mice2DElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<Mice2DElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void Mice2DElementRenderer::Render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = r ? r : renderTarget.lock()->GetDirect2DRenderTarget();
        engine.Render(d2dRenderTarget, bounds);
    }
    GraphicsRenderer::Render(bounds, r);
}

Mice2DElementRenderer::~Mice2DElementRenderer()
{
}

int Mice2DElementRenderer::Refresh(int arg)
{
    return engine.SetType(arg);
}

void Mice2DElementRenderer::OnElementStateChanged()
{
}

void Mice2DElementRenderer::InitializeInternal()
{
    engine.Initialize(renderTarget.lock());
}

void Mice2DElementRenderer::FinalizeInternal()
{
    engine.Finalize(renderTarget.lock());
}

void Mice2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    engine.Reset(oldRenderTarget, newRenderTarget);
}
