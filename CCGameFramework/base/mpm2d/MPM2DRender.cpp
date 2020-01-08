#include "stdafx.h"
#include "render/Direct2DRender.h"


MPM2DElement::MPM2DElement()
{
}

MPM2DElement::~MPM2DElement()
{
}

CString MPM2DElement::GetElementTypeName()
{
    return _T("MPM2D");
}

cint MPM2DElement::GetTypeId()
{
    return MPM2D;
}

FLOAT MPM2DElement::GetOpacity() const
{
    return opacity;
}

void MPM2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint MPM2DElement::GetType() const
{
    return type;
}

void MPM2DElement::SetType(cint value)
{
    type = value;
}

int MPM2DElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<MPM2DElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void MPM2DElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget.lock()->GetDirect2DRenderTarget();
        engine.Render(d2dRenderTarget, bounds);
    }
    GraphicsRenderer::Render(bounds);
}

MPM2DElementRenderer::~MPM2DElementRenderer()
{
}

int MPM2DElementRenderer::Refresh(int arg)
{
    return engine.SetType(arg);
}

void MPM2DElementRenderer::OnElementStateChanged()
{
}

void MPM2DElementRenderer::InitializeInternal()
{
    engine.Initialize(renderTarget.lock());
}

void MPM2DElementRenderer::FinalizeInternal()
{
    engine.Finalize(renderTarget.lock());
}

void MPM2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    engine.Reset(oldRenderTarget, newRenderTarget);
}
