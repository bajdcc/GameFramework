#ifndef RENDER_D2D_RENDERTARGET_H
#define RENDER_D2D_RENDERTARGET_H

#include <ui/gdi/Gdi.h>
#include "Direct2DAllocator.h"

class Window;

class Direct2DRenderTarget : public std::enable_shared_from_this<Direct2DRenderTarget>
{
public:
    Direct2DRenderTarget(std::weak_ptr<Window> _window);
    ~Direct2DRenderTarget();

    void Init();

    CComPtr<ID2D1RenderTarget> GetDirect2DRenderTarget();
    D2D1_WINDOW_STATE CheckWindowState();
    void SetTextAntialias(bool antialias, bool verticalAntialias);
    bool StartRendering();
    HRESULT StopRendering();

    bool RecreateRenderTarget(CSize size);
    void ClearRenderTarget();

    CComPtr<ID2D1SolidColorBrush> CreateDirect2DBrush(CColor color);
    void DestroyDirect2DBrush(CColor color);
    CComPtr<ID2D1LinearGradientBrush> CreateDirect2DLinearBrush(CColor c1, CColor c2);
    void DestroyDirect2DLinearBrush(CColor c1, CColor c2);
    std::shared_ptr<D2DTextFormatPackage> CreateDirect2DTextFormat(const Font& font);
    void DestroyDirect2DTextFormat(const Font& font);

    CComPtr<IWICBitmap> CreateBitmap(UINT width, UINT height);
    CComPtr<IWICBitmap> CreateImageFromFile(const CStringA& path, int index = 0);
    CComPtr<IWICBitmap> CreateImageFromMemory(LPVOID buffer, int length, int index = 0);
    CComPtr<IWICBitmap> CreateImageFromBitmap(HBITMAP handle, int index = 0);
    CComPtr<IWICBitmap> CreateImageFromIcon(HICON handle, int index = 0);

    CComPtr<ID2D1Bitmap> GetBitmapFromWIC(CComPtr<IWICBitmap> bitmap);

protected:
    CComPtr<IDWriteRenderingParams> CreateRenderingParams(DWRITE_RENDERING_MODE renderingMode, CComPtr<IDWriteRenderingParams> defaultParams, CComPtr<IDWriteFactory> dwriteFactory);
    CComPtr<IWICBitmap> GetBitmap(CComPtr<IWICBitmapDecoder> source, int index);

    std::weak_ptr<Window> window;
    CComPtr<ID2D1HwndRenderTarget> d2dRenderTarget;
    CSize previousSize;

    CachedSolidBrushAllocator solidBrushes;
    CachedLinearBrushAllocator linearBrushes;
    CachedTextFormatAllocator textFormats;

    CComPtr<IDWriteRenderingParams> noAntialiasParams;
    CComPtr<IDWriteRenderingParams> horizontalAntialiasParams;
    CComPtr<IDWriteRenderingParams> bidirectionalAntialiasParams;

    CComPtr<IWICImagingFactory> imagingFactory;
};

#endif