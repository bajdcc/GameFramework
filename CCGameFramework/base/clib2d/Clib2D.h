#ifndef CLIB2D_CLIB2D_H
#define CLIB2D_CLIB2D_H
#include "render/Direct2DAllocator.h"
#include "c2d.h"
#include <memory>

class Clib2DEngine
{
public:
    void RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget);

    int SetType(cint value);

    const clib::BrushBag& GetBrushBag() const;

private:
    void RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

private:
    Font logoFont;
    CColor logoColor;
    std::shared_ptr<D2DTextFormatPackage> logoTF;
    CComPtr<ID2D1SolidColorBrush> logoBrush;
    clib::BrushBag brushes;
};

#endif