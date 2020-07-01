#include "stdafx.h"
#include "render/Direct2DRender.h"


Clib2DElement::Clib2DElement()
{
}

Clib2DElement::~Clib2DElement()
{
}

CString Clib2DElement::GetElementTypeName()
{
    return _T("Clib2D");
}

cint Clib2DElement::GetTypeId()
{
    return Clib2D;
}

FLOAT Clib2DElement::GetOpacity() const
{
    return opacity;
}

void Clib2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint Clib2DElement::GetType() const
{
    return type;
}

void Clib2DElement::SetType(cint value)
{
    type = value;
}

int Clib2DElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<Clib2DElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void Clib2DElementRenderer::Render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = r ? r : renderTarget.lock()->GetDirect2DRenderTarget();
        engine.Render(d2dRenderTarget, bounds);
    }
    GraphicsRenderer::Render(bounds, r);
}

Clib2DElementRenderer::~Clib2DElementRenderer()
{
}

int Clib2DElementRenderer::Refresh(int arg)
{
    return engine.SetType(arg);
}

void Clib2DElementRenderer::OnElementStateChanged()
{
}

void Clib2DElementRenderer::InitializeInternal()
{
    engine.Initialize(renderTarget.lock());
}

void Clib2DElementRenderer::FinalizeInternal()
{
    engine.Finalize(renderTarget.lock());
}

void Clib2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    engine.Reset(oldRenderTarget, newRenderTarget);
}
