#include "stdafx.h"
#include "Direct2DRenderTarget.h"
#include "Direct2D.h"
#include <ui/window/Window.h>

Direct2DRenderTarget::Direct2DRenderTarget(std::weak_ptr<Window> _window)
    : window(_window)
{
    imagingFactory = Direct2D::Singleton().GetWICImagingFactory();
    CComPtr<IDWriteFactory> dwriteFactory = Direct2D::Singleton().GetDirectWriteFactory();
    CComPtr<IDWriteRenderingParams> defaultParams;
    HRESULT hr = dwriteFactory->CreateRenderingParams(&defaultParams);
    if (SUCCEEDED(hr))
    {
        noAntialiasParams = CreateRenderingParams(DWRITE_RENDERING_MODE_CLEARTYPE_GDI_NATURAL, defaultParams, dwriteFactory);
        horizontalAntialiasParams = CreateRenderingParams(DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL, defaultParams, dwriteFactory);
        bidirectionalAntialiasParams = CreateRenderingParams(DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC, defaultParams, dwriteFactory);
    }
}

Direct2DRenderTarget::~Direct2DRenderTarget()
{

}

void Direct2DRenderTarget::Init()
{
    solidBrushes.SetRenderTarget(shared_from_this());
    linearBrushes.SetRenderTarget(shared_from_this());
    d2dRenderTarget = Direct2D::Singleton().GetDirect2DDeviceContext();
}

CComPtr<ID2D1RenderTarget> Direct2DRenderTarget::GetDirect2DRenderTarget()
{
    return d2dRenderTarget;
}

void Direct2DRenderTarget::SetTextAntialias(bool antialias, bool verticalAntialias)
{
    CComPtr<IDWriteRenderingParams> params;
    if (!antialias)
    {
        params = noAntialiasParams;
    }
    else if (!verticalAntialias)
    {
        params = horizontalAntialiasParams;
    }
    else
    {
        params = bidirectionalAntialiasParams;
    }
    if (params.p != nullptr && d2dRenderTarget.p != nullptr)
    {
        d2dRenderTarget->SetTextRenderingParams(params);
    }
}

void Direct2DRenderTarget::StartRendering()
{
    d2dRenderTarget->BeginDraw();
    d2dRenderTarget->Clear(GetD2DColor(CColor(Gdiplus::Color::Gray)));
}

HRESULT Direct2DRenderTarget::StopRendering()
{
    return d2dRenderTarget->EndDraw();
}

HRESULT Direct2DRenderTarget::Present()
{
    return Direct2D::Singleton().GetDXGISwapChain()->Present(1, 0);
}

CComPtr<IDWriteRenderingParams> Direct2DRenderTarget::CreateRenderingParams(DWRITE_RENDERING_MODE renderingMode, CComPtr<IDWriteRenderingParams> defaultParams, CComPtr<IDWriteFactory> dwriteFactory)
{
    IDWriteRenderingParams* renderingParams;
    FLOAT gamma = defaultParams->GetGamma();
    FLOAT enhancedContrast = defaultParams->GetEnhancedContrast();
    FLOAT clearTypeLevel = defaultParams->GetClearTypeLevel();
    DWRITE_PIXEL_GEOMETRY pixelGeometry = defaultParams->GetPixelGeometry();
    HRESULT hr = dwriteFactory->CreateCustomRenderingParams(
        gamma,
        enhancedContrast,
        clearTypeLevel,
        pixelGeometry,
        renderingMode,
        &renderingParams);
    if (SUCCEEDED(hr))
    {
        CComPtr<IDWriteRenderingParams> RenderingParams;
        RenderingParams.Attach(renderingParams);
        return RenderingParams;
    }
    else
    {
        return nullptr;
    }
}

CComPtr<IWICBitmap> Direct2DRenderTarget::GetBitmap(CComPtr<IWICBitmapDecoder> source, int index)
{
    CComPtr<IWICBitmapFrameDecode> frameDecode;
    HRESULT hr = source->GetFrame(index, &frameDecode);
    if (FAILED(hr))
        ATLVERIFY(!"GetFrame failed");
    CComPtr<IWICFormatConverter> converter;
    hr = imagingFactory->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr))
    {
        converter->Initialize(
            frameDecode,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.0f,
            WICBitmapPaletteTypeCustom);
    }

    CComPtr<IWICBitmap> bitmap;
    CComPtr<IWICBitmapSource> convertedBitmapSource;
    if (converter)
    {
        convertedBitmapSource = converter;
    }
    else
    {
        convertedBitmapSource = frameDecode;
    }
    hr = imagingFactory->CreateBitmapFromSource(convertedBitmapSource, WICBitmapCacheOnLoad, &bitmap);
    if (FAILED(hr))
        ATLVERIFY(!"CreateBitmapFromSource failed");

    return bitmap;
}

CComPtr<ID2D1SolidColorBrush> Direct2DRenderTarget::CreateDirect2DBrush(CColor color)
{
    return solidBrushes.Create(color);
}

void Direct2DRenderTarget::DestroyDirect2DBrush(CColor color)
{
    solidBrushes.Destroy(color);
}

CComPtr<ID2D1LinearGradientBrush> Direct2DRenderTarget::CreateDirect2DLinearBrush(CColor c1, CColor c2)
{
    return linearBrushes.Create(std::make_pair(c1, c2));
}

void Direct2DRenderTarget::DestroyDirect2DLinearBrush(CColor c1, CColor c2)
{
    linearBrushes.Destroy(std::make_pair(c1, c2));
}

std::shared_ptr<D2DTextFormatPackage> Direct2DRenderTarget::CreateDirect2DTextFormat(const Font& font)
{
    return textFormats.Create(font);
}

CComPtr<IWICBitmap> Direct2DRenderTarget::CreateBitmap(UINT width, UINT height)
{
    CComPtr<IWICBitmap> bitmap;
    HRESULT hr = imagingFactory->CreateBitmap(
        width,
        height,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapCacheOnDemand,
        &bitmap);
    if (FAILED(hr))
        ATLVERIFY(!"CreateBitmap failed");
    return bitmap;
}

CComPtr<IWICBitmap> Direct2DRenderTarget::CreateImageFromFile(const CStringA& path, int index)
{
    CComPtr<IWICBitmapDecoder> bitmapDecoder;
    HRESULT hr = imagingFactory->CreateDecoderFromFilename(
        CString(path),
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &bitmapDecoder);
    if (FAILED(hr))
        ATLVERIFY(!"CreateDecoderFromFilename failed");
    return GetBitmap(bitmapDecoder, index);
}

CComPtr<IWICBitmap> Direct2DRenderTarget::CreateImageFromMemory(LPVOID buffer, int length, int index)
{
    CComPtr<IStream> stream = SHCreateMemStream((const BYTE*)buffer, length);
    if (!stream)
    {
        ATLVERIFY(!"SHCreateMemStream failed");
    }
    CComPtr<IWICBitmapDecoder> bitmapDecoder;
    HRESULT hr = imagingFactory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnDemand, &bitmapDecoder);
    if (FAILED(hr))
        ATLVERIFY(!"CreateDecoderFromStream failed");
    return GetBitmap(bitmapDecoder, index);
}

CComPtr<IWICBitmap> Direct2DRenderTarget::CreateImageFromBitmap(HBITMAP handle, int index)
{
    CComPtr<IWICBitmap> bitmap;
    HRESULT hr = imagingFactory->CreateBitmapFromHBITMAP(handle, NULL, WICBitmapUseAlpha, &bitmap);
    if (FAILED(hr))
        ATLVERIFY(!"CreateBitmapFromHBITMAP failed");
    return bitmap;
}

CComPtr<IWICBitmap> Direct2DRenderTarget::CreateImageFromIcon(HICON handle, int index)
{
    CComPtr<IWICBitmap> bitmap;
    HRESULT hr = imagingFactory->CreateBitmapFromHICON(handle, &bitmap);
    if (FAILED(hr))
        ATLVERIFY(!"CreateBitmapFromHICON failed");
    return bitmap;
}

CComPtr<ID2D1Bitmap> Direct2DRenderTarget::GetBitmapFromWIC(CComPtr<IWICBitmap> bitmap)
{
    CComPtr<ID2D1Bitmap> d2dBitmap;
    HRESULT hr = d2dRenderTarget->CreateBitmapFromWicBitmap(
        bitmap,
        &d2dBitmap
    );
    if (FAILED(hr))
        ATLVERIFY(!"CreateBitmapFromWicBitmap failed");
    return d2dBitmap;
}

void Direct2DRenderTarget::DestroyDirect2DTextFormat(const Font& font)
{
    textFormats.Destroy(font);
}

CComPtr<ID2D1BitmapRenderTarget> Direct2DRenderTarget::CreateBitmapRenderTarget(D2D1_SIZE_F size)
{
    CComPtr<ID2D1BitmapRenderTarget> rt;
    HRESULT hr = d2dRenderTarget->CreateCompatibleRenderTarget(size, &rt);
    if (FAILED(hr))
        ATLVERIFY(!"CreateCompatibleRenderTarget failed");
    return rt;
}
