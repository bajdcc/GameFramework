#include "stdafx.h"
#include "Direct2DRenderTarget.h"
#include "Direct2D.h"
#include <ui/window/Window.h>

static HWND GetHWNDFromWindow(PassRefPtr<Window> window)
{
    if (!window) return NULL;
    return window->GetWindowHandle();
}

Direct2DRenderTarget::Direct2DRenderTarget(PassRefPtr<Window> _window)
    : window(_window)
{
    solidBrushes.SetRenderTarget(this);
    linearBrushes.SetRenderTarget(this);
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

CComPtr<ID2D1RenderTarget> Direct2DRenderTarget::GetDirect2DRenderTarget()
{
    return dynamic_cast<ID2D1RenderTarget*>(d2dRenderTarget.p);
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

bool Direct2DRenderTarget::StartRendering()
{
    if (!d2dRenderTarget)
        return false;
    d2dRenderTarget->BeginDraw();
    d2dRenderTarget->Clear(GetD2DColor(CColor(Gdiplus::Color::Gray)));
    return true;
}

HRESULT Direct2DRenderTarget::StopRendering()
{
    HRESULT result = d2dRenderTarget->EndDraw();
    return result;
}

bool Direct2DRenderTarget::RecreateRenderTarget(CSize size)
{
    if (!d2dRenderTarget)
    {
        ID2D1HwndRenderTarget* renderTarget;
        D2D1_RENDER_TARGET_PROPERTIES tp = D2D1::RenderTargetProperties();
        tp.dpiX = 96;
        tp.dpiY = 96;
        size.cx = __max(size.cx, 1);
        size.cy = __max(size.cy, 1);
        HRESULT hr = Direct2D::Singleton().GetDirect2DFactory()->CreateHwndRenderTarget(
            tp,
            D2D1::HwndRenderTargetProperties(
                GetHWNDFromWindow(window),
                D2D1::SizeU((int)size.cx, (int)size.cy)
            ),
            &renderTarget
        );
        if (SUCCEEDED(hr))
        {
            d2dRenderTarget.Attach(renderTarget);
            d2dRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
            ATLTRACE(atlTraceWindowing, 0, "D2D::Create %s\n", (LPCSTR)size.ToString());
        }
    }
    else if (previousSize != size)
    {
        HRESULT hr = d2dRenderTarget->Resize(D2D1::SizeU((int)size.cx, (int)size.cy));
        ATLTRACE(atlTraceWindowing, 0, "D2D::Resize %s\n", (LPCSTR)size.ToString());
    }
    else
    {
        return false;
    }
    previousSize = size;
    return true;
}

void Direct2DRenderTarget::ClearRenderTarget()
{
    d2dRenderTarget = nullptr;
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
    return linearBrushes.Create(std::pair<CColor, CColor>(c1, c2));
}

void Direct2DRenderTarget::DestroyDirect2DLinearBrush(CColor c1, CColor c2)
{
    linearBrushes.Destroy(std::pair<CColor, CColor>(c1, c2));
}

PassRefPtr<D2DTextFormatPackage> Direct2DRenderTarget::CreateDirect2DTextFormat(const Font& font)
{
    return textFormats.Create(font);
}

void Direct2DRenderTarget::DestroyDirect2DTextFormat(const Font& font)
{
    textFormats.Destroy(font);;
}