#include "stdafx.h"
#include "Direct2DAllocator.h"
#include "Direct2D.h"
#include "Direct2DRenderTarget.h"


CComPtr<IDWriteTextFormat> CachedTextFormatAllocator::CreateDirect2DFont(const Font& font)
{
    CComPtr<IDWriteFactory> dwriteFactory = Direct2D::Singleton().GetDirectWriteFactory();
    CComPtr<IDWriteTextFormat> format;
    HRESULT hr = dwriteFactory->CreateTextFormat(
        CString(font.fontFamily),
        NULL,
        (font.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL),
        (font.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL),
        DWRITE_FONT_STRETCH_NORMAL,
        (FLOAT)font.size,
        _T(""),
        &format);
    if (SUCCEEDED(hr))
    {
        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        return format;
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<D2DTextFormatPackage> CachedTextFormatAllocator::CreateInternal(const Font& font)
{
    auto textFormat = std::make_shared<D2DTextFormatPackage>();
    textFormat->textFormat = CreateDirect2DFont(font);
    textFormat->trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
    textFormat->trimming.delimiter = 0;
    textFormat->trimming.delimiterCount = 0;

    IDWriteInlineObject* ellipseInlineObject;
    HRESULT hr = Direct2D::Singleton().GetDirectWriteFactory()->CreateEllipsisTrimmingSign(textFormat->textFormat, &ellipseInlineObject);
    if (SUCCEEDED(hr))
    {
        textFormat->ellipseInlineObject.Attach(ellipseInlineObject);
    }
    return textFormat;
}

CComPtr<ID2D1SolidColorBrush> CachedSolidBrushAllocator::CreateInternal(const CColor& color)
{
    CComPtr<ID2D1RenderTarget> renderTarget = guiRenderTarget->GetDirect2DRenderTarget();
    CComPtr<ID2D1SolidColorBrush> brush;
    HRESULT hr = renderTarget->CreateSolidColorBrush(GetD2DColor(color), &brush);
    if (SUCCEEDED(hr))
    {
        return brush;
    }
    else
    {
        return nullptr;
    }
}

CComPtr<ID2D1LinearGradientBrush> CachedLinearBrushAllocator::CreateInternal(const std::pair<CColor, CColor>& colors)
{
    CComPtr<ID2D1RenderTarget> renderTarget = guiRenderTarget->GetDirect2DRenderTarget();
    CComPtr<ID2D1GradientStopCollection> stopCollection;
    {
        D2D1_GRADIENT_STOP stops[2];
        stops[0].color = GetD2DColor(colors.first);
        stops[0].position = 0.0f;
        stops[1].color = GetD2DColor(colors.second);
        stops[1].position = 1.0f;

        HRESULT hr = renderTarget->CreateGradientStopCollection(
            stops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &stopCollection);
        if (FAILED(hr))
            return nullptr;
    }

    CComPtr<ID2D1LinearGradientBrush> brush;
    {
        D2D1_POINT_2F points[2] = { { 0, 0 },{ 0, 0 } };
        HRESULT hr = renderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(points[0], points[1]),
            stopCollection,
            &brush);
        if (FAILED(hr))
            return nullptr;
    }
    return brush;
}