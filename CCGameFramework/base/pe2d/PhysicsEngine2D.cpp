#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

void PhysicsEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    rt->FillRectangle(
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        bg
    );
}

void PhysicsEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    bgColor = Gdiplus::Color::LightGoldenrodYellow;
}

void PhysicsEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    rt->DestroyDirect2DBrush(bgColor);
}

void PhysicsEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor);
        bg = nullptr;
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
    }
}
