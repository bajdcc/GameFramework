#ifndef RENDER_D2D_RENDERTARGET_H
#define RENDER_D2D_RENDERTARGET_H

#include <WTF/PassRefPtr.h>
#include <WTF/refcounted.h>
#include <ui/gdi/Gdi.h>
#include "Direct2DAllocator.h"

class Window;

class Direct2DRenderTarget : public RefCounted<Direct2DRenderTarget>
{
public:
    Direct2DRenderTarget(PassRefPtr<Window> _window);
    ~Direct2DRenderTarget();

    CComPtr<ID2D1RenderTarget> GetDirect2DRenderTarget();
    void SetTextAntialias(bool antialias, bool verticalAntialias);
    bool StartRendering();
    HRESULT StopRendering();

    bool RecreateRenderTarget(CSize size);
    void ClearRenderTarget();

    CComPtr<ID2D1SolidColorBrush>		CreateDirect2DBrush(CColor color);
    void								DestroyDirect2DBrush(CColor color);
    CComPtr<ID2D1LinearGradientBrush>	CreateDirect2DLinearBrush(CColor c1, CColor c2);
    void								DestroyDirect2DLinearBrush(CColor c1, CColor c2);
    PassRefPtr<D2DTextFormatPackage>	CreateDirect2DTextFormat(const Font& font);
    void								DestroyDirect2DTextFormat(const Font& font);

protected:
    CComPtr<IDWriteRenderingParams> CreateRenderingParams(DWRITE_RENDERING_MODE renderingMode, CComPtr<IDWriteRenderingParams> defaultParams, CComPtr<IDWriteFactory> dwriteFactory);

    RawPtr<Window> window;
    CComPtr<ID2D1HwndRenderTarget> d2dRenderTarget;
    CSize previousSize;

    CachedSolidBrushAllocator solidBrushes;
    CachedLinearBrushAllocator linearBrushes;
    CachedTextFormatAllocator textFormats;

    CComPtr<IDWriteRenderingParams> noAntialiasParams;
    CComPtr<IDWriteRenderingParams> horizontalAntialiasParams;
    CComPtr<IDWriteRenderingParams> bidirectionalAntialiasParams;
};

#endif