#ifndef PE2D_PE2D_H
#define PE2D_PE2D_H
#include "render/Direct2DAllocator.h"
#include <memory>


class PhysicsEngine
{
public:
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget);

private:
    CComPtr<ID2D1SolidColorBrush> bg;
    CColor bgColor;
};

#endif