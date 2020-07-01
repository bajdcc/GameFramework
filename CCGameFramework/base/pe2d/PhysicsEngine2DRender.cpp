#include "stdafx.h"
#include "render/Direct2DRender.h"


PhysicsEngine2DElement::PhysicsEngine2DElement()
{
}

PhysicsEngine2DElement::~PhysicsEngine2DElement()
{
}

CString PhysicsEngine2DElement::GetElementTypeName()
{
    return _T("PhysicsEngine2D");
}

cint PhysicsEngine2DElement::GetTypeId()
{
    return PhysicsEngine2D;
}

FLOAT PhysicsEngine2DElement::GetOpacity() const
{
    return opacity;
}

void PhysicsEngine2DElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint PhysicsEngine2DElement::GetType() const
{
    return type;
}

void PhysicsEngine2DElement::SetType(cint value)
{
    type = value;
}

int PhysicsEngine2DElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<PhysicsEngine2DElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void PhysicsEngine2DElementRenderer::Render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = r ? r : renderTarget.lock()->GetDirect2DRenderTarget();
        pe.Render(d2dRenderTarget, bounds);
    }
    GraphicsRenderer::Render(bounds, r);
}

PhysicsEngine2DElementRenderer::~PhysicsEngine2DElementRenderer()
{
}

int PhysicsEngine2DElementRenderer::Refresh(int arg)
{
    return pe.SetType(arg);
}

void PhysicsEngine2DElementRenderer::OnElementStateChanged()
{
}

void PhysicsEngine2DElementRenderer::InitializeInternal()
{
    pe.Initialize(renderTarget.lock());
}

void PhysicsEngine2DElementRenderer::FinalizeInternal()
{
    pe.Finalize(renderTarget.lock());
}

void PhysicsEngine2DElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    pe.Reset(oldRenderTarget, newRenderTarget);
}
