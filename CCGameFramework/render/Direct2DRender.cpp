#include "stdafx.h"
#include "Direct2DRender.h"
#include "Direct2D.h"

#pragma region Empty
EmptyElement::EmptyElement()
{

}

EmptyElement::~EmptyElement()
{
    renderer->Finalize();
}

CString EmptyElement::GetElementTypeName()
{
    return _T("Empty");
}

cint EmptyElement::GetTypeId()
{
    return Empty;
}

void EmptyElementRenderer::InitializeInternal()
{

}

void EmptyElementRenderer::FinalizeInternal()
{

}

void EmptyElementRenderer::RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{

}

void EmptyElementRenderer::OnElementStateChanged()
{

}
#pragma endregion Empty

#pragma region SolidBackground
SolidBackgroundElement::SolidBackgroundElement()
{

}

SolidBackgroundElement::~SolidBackgroundElement()
{
    renderer->Finalize();
}

CString SolidBackgroundElement::GetElementTypeName()
{
    return _T("SolidBackground");
}

cint SolidBackgroundElement::GetTypeId()
{
    return SolidBackground;
}

CColor SolidBackgroundElement::GetColor()
{
    return color;
}

void SolidBackgroundElement::SetColor(CColor value)
{
    if (color != value)
    {
        color = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

void SolidBackgroundElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    auto rt = renderTarget.lock();
    if (e->flags.self_visible)
    {
        auto d2dRenderTarget = rt->GetDirect2DRenderTarget();
        d2dRenderTarget->FillRectangle(
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            brush
        );
    }
    GraphicsRenderer::Render(bounds);
}
#pragma endregion SolidBackground

#pragma region GradientBackground
GradientBackgroundElement::GradientBackgroundElement()
{

}

GradientBackgroundElement::~GradientBackgroundElement()
{
    renderer->Finalize();
}

CString GradientBackgroundElement::GetElementTypeName()
{
    return _T("GradientBackground");
}

cint GradientBackgroundElement::GetTypeId()
{
    return GradientBackground;
}

CColor GradientBackgroundElement::GetColor1()
{
    return color1;
}

void GradientBackgroundElement::SetColor1(CColor value)
{
    if (color1 != value)
    {
        color1 = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

CColor GradientBackgroundElement::GetColor2()
{
    return color2;
}

void GradientBackgroundElement::SetColor2(CColor value)
{
    if (color2 != value)
    {
        color2 = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

void GradientBackgroundElement::SetColors(CColor value1, CColor value2)
{
    if (color1 != value1 || color2 != value2)
    {
        color1 = value1;
        color2 = value2;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

GradientBackgroundElement::Direction GradientBackgroundElement::GetDirection()
{
    return direction;
}

void GradientBackgroundElement::SetDirection(Direction value)
{
    direction = value;
}

void GradientBackgroundElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    auto rt = renderTarget.lock();
    if (e->flags.self_visible)
    {
        D2D1_POINT_2F points[2];
        switch (e->GetDirection())
        {
        case GradientBackgroundElement::Horizontal:
        {
            points[0].x = (FLOAT)bounds.left;
            points[0].y = (FLOAT)bounds.top;
            points[1].x = (FLOAT)bounds.right;
            points[1].y = (FLOAT)bounds.top;
        }
        break;
        case GradientBackgroundElement::Vertical:
        {
            points[0].x = (FLOAT)bounds.left;
            points[0].y = (FLOAT)bounds.top;
            points[1].x = (FLOAT)bounds.left;
            points[1].y = (FLOAT)bounds.bottom;
        }
        break;
        case GradientBackgroundElement::Slash:
        {
            points[0].x = (FLOAT)bounds.right;
            points[0].y = (FLOAT)bounds.top;
            points[1].x = (FLOAT)bounds.left;
            points[1].y = (FLOAT)bounds.bottom;
        }
        break;
        case GradientBackgroundElement::Backslash:
        {
            points[0].x = (FLOAT)bounds.left;
            points[0].y = (FLOAT)bounds.top;
            points[1].x = (FLOAT)bounds.right;
            points[1].y = (FLOAT)bounds.bottom;
        }
        break;
        }

        brush->SetStartPoint(points[0]);
        brush->SetEndPoint(points[1]);

        auto d2dRenderTarget = rt->GetDirect2DRenderTarget();
        d2dRenderTarget->FillRectangle(
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            brush
        );
    }
    GraphicsRenderer::Render(bounds);
}
#pragma endregion GradientBackground

#pragma region SolidLabel
SolidLabelElement::SolidLabelElement()
{
    fontProperties.fontFamily = _T("Microsoft Yahei");
    fontProperties.size = 12;
}

SolidLabelElement::~SolidLabelElement()
{
    renderer->Finalize();
}

CString SolidLabelElement::GetElementTypeName()
{
    return _T("SolidLabel");
}

cint SolidLabelElement::GetTypeId()
{
    return SolidLabel;
}

CColor SolidLabelElement::GetColor()
{
    return color;
}

void SolidLabelElement::SetColor(CColor value)
{
    if (color != value)
    {
        color = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

const Font& SolidLabelElement::GetFont()
{
    return fontProperties;
}

void SolidLabelElement::SetFont(const Font& value)
{
    if (fontProperties != value)
    {
        fontProperties = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

const CString& SolidLabelElement::GetText()
{
    return text;
}

void SolidLabelElement::SetText(const CString& value)
{
    if (text != value)
    {
        text = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

Alignment SolidLabelElement::GetHorizontalAlignment()
{
    return hAlignment;
}

Alignment SolidLabelElement::GetVerticalAlignment()
{
    return vAlignment;
}

void SolidLabelElement::SetHorizontalAlignment(Alignment value)
{
    SetAlignments(value, vAlignment);
}

void SolidLabelElement::SetVerticalAlignment(Alignment value)
{
    SetAlignments(hAlignment, value);
}

void SolidLabelElement::SetAlignments(Alignment horizontal, Alignment vertical)
{
    if (hAlignment != horizontal || vAlignment != vertical)
    {
        hAlignment = horizontal;
        vAlignment = vertical;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

bool SolidLabelElement::GetWrapLine()
{
    return wrapLine;
}

void SolidLabelElement::SetWrapLine(bool value)
{
    if (wrapLine != value)
    {
        wrapLine = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

bool SolidLabelElement::GetMultiline()
{
    return multiline;
}

void SolidLabelElement::SetMultiline(bool value)
{
    if (multiline != value)
    {
        multiline = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

bool SolidLabelElement::GetWrapLineHeightCalculation()
{
    return wrapLineHeightCalculation;
}

void SolidLabelElement::SetWrapLineHeightCalculation(bool value)
{
    if (wrapLineHeightCalculation != value)
    {
        wrapLineHeightCalculation = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

SolidLabelElementRenderer::SolidLabelElementRenderer()
{

}

void SolidLabelElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        if (!textLayout)
        {
            CreateTextLayout();
        }

        cint x = 0;
        cint y = 0;
        switch (e->GetHorizontalAlignment())
        {
        case Alignment::StringAlignmentNear:
            x = bounds.left;
            break;
        case Alignment::StringAlignmentCenter:
            x = bounds.left + (bounds.Width() - minSize.cx) / 2;
            break;
        case Alignment::StringAlignmentFar:
            x = bounds.right - minSize.cx;
            break;
        }
        switch (e->GetVerticalAlignment())
        {
        case Alignment::StringAlignmentNear:
            y = bounds.top;
            break;
        case Alignment::StringAlignmentCenter:
            y = bounds.top + (bounds.Height() - minSize.cy) / 2;
            break;
        case Alignment::StringAlignmentFar:
            y = bounds.bottom - minSize.cy;
            break;
        }

        auto rt = renderTarget.lock();
        rt->SetTextAntialias(oldFont.antialias, oldFont.verticalAntialias);

        if (!e->GetMultiline() && !e->GetWrapLine())
        {
            auto d2dRenderTarget = rt->GetDirect2DRenderTarget();
            d2dRenderTarget->DrawTextLayout(
                D2D1::Point2F((FLOAT)x, (FLOAT)y),
                textLayout,
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
            );
        }
        else
        {
            auto dwriteFactory = Direct2D::Singleton().GetDirectWriteFactory();
            DWRITE_TRIMMING trimming;
            CComPtr<IDWriteInlineObject> inlineObject;
            textLayout->GetTrimming(&trimming, &inlineObject);
            textLayout->SetWordWrapping(e->GetWrapLine() ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
            switch (e->GetHorizontalAlignment())
            {
            case Alignment::StringAlignmentNear:
                textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                break;
            case Alignment::StringAlignmentCenter:
                textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                break;
            case Alignment::StringAlignmentFar:
                textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                break;
            }
            if (!e->GetMultiline() && !e->GetWrapLine())
            {
                switch (e->GetVerticalAlignment())
                {
                case Alignment::StringAlignmentNear:
                    textLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
                    break;
                case Alignment::StringAlignmentCenter:
                    textLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                    break;
                case Alignment::StringAlignmentFar:
                    textLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
                    break;
                }
            }

            CRect textBounds = bounds;
            textLayout->SetMaxWidth((FLOAT)textBounds.Width());
            textLayout->SetMaxHeight((FLOAT)textBounds.Height());

            auto d2dRenderTarget = rt->GetDirect2DRenderTarget();
            d2dRenderTarget->DrawTextLayout(
                D2D1::Point2F((FLOAT)textBounds.left, (FLOAT)textBounds.top),
                textLayout,
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
            );

            textLayout->SetTrimming(&trimming, inlineObject);
            if (oldMaxWidth != textBounds.Width())
            {
                oldMaxWidth = textBounds.Width();
                UpdateMinSize();
            }
        }
    }
    GraphicsRenderer::Render(bounds);
}

void SolidLabelElementRenderer::OnElementStateChanged()
{
    auto e = element.lock();
    auto rt = renderTarget.lock();
    if (rt)
    {
        CColor color = e->GetColor();
        if (oldColor != color)
        {
            DestroyBrush(rt);
            CreateBrush(rt);
        }

        Font font = e->GetFont();
        if (oldFont != font)
        {
            DestroyTextFormat(rt);
            CreateTextFormat(rt);
        }
    }
    oldText = e->GetText();
    UpdateMinSize();
}

void SolidLabelElementRenderer::CreateBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget)
{
    if (_renderTarget)
    {
        oldColor = element.lock()->GetColor();
        brush = _renderTarget->CreateDirect2DBrush(oldColor);
    }
}

void SolidLabelElementRenderer::DestroyBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget)
{
    if (_renderTarget && brush)
    {
        _renderTarget->DestroyDirect2DBrush(oldColor);
        brush = nullptr;
    }
}

void SolidLabelElementRenderer::CreateTextFormat(std::shared_ptr<Direct2DRenderTarget> _renderTarget)
{
    if (_renderTarget)
    {
        oldFont = element.lock()->GetFont();
        textFormat = _renderTarget->CreateDirect2DTextFormat(oldFont);
    }
}

void SolidLabelElementRenderer::DestroyTextFormat(std::shared_ptr<Direct2DRenderTarget> _renderTarget)
{
    if (_renderTarget && textFormat)
    {
        _renderTarget->DestroyDirect2DTextFormat(oldFont);
        textFormat = nullptr;
    }
}

void SolidLabelElementRenderer::CreateTextLayout()
{
    if (textFormat)
    {
        BSTR _text = oldText.AllocSysString();
        HRESULT hr = Direct2D::Singleton().GetDirectWriteFactory()->CreateTextLayout(
            _text,
            oldText.GetLength(),
            textFormat->textFormat,
            0,
            0,
            &textLayout);
        SysFreeString(_text);
        if (SUCCEEDED(hr))
        {
            if (oldFont.underline)
            {
                DWRITE_TEXT_RANGE textRange;
                textRange.startPosition = 0;
                textRange.length = oldText.GetLength();
                textLayout->SetUnderline(TRUE, textRange);
            }
            if (oldFont.strikeline)
            {
                DWRITE_TEXT_RANGE textRange;
                textRange.startPosition = 0;
                textRange.length = oldText.GetLength();
                textLayout->SetStrikethrough(TRUE, textRange);
            }
        }
        else
        {
            textLayout = nullptr;
        }
    }
}

void SolidLabelElementRenderer::DestroyTextLayout()
{
    if (textLayout)
    {
        textLayout = nullptr;
    }
}

void SolidLabelElementRenderer::UpdateMinSize()
{
    float maxWidth = 0;
    DestroyTextLayout();
    bool calculateSizeFromTextLayout = false;
    auto e = element.lock();
    auto rt = renderTarget.lock();
    if (rt)
    {
        if (e->GetWrapLine())
        {
            if (e->GetWrapLineHeightCalculation())
            {
                CreateTextLayout();
                if (textLayout)
                {
                    maxWidth = textLayout->GetMaxWidth();
                    if (oldMaxWidth != -1)
                    {
                        textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
                        textLayout->SetMaxWidth((float)oldMaxWidth);
                    }
                    calculateSizeFromTextLayout = true;
                }
            }
        }
        else
        {
            CreateTextLayout();
            if (textLayout)
            {
                maxWidth = textLayout->GetMaxWidth();
                calculateSizeFromTextLayout = true;
            }
        }
    }
    if (calculateSizeFromTextLayout)
    {
        DWRITE_TEXT_METRICS metrics;
        HRESULT hr = textLayout->GetMetrics(&metrics);
        if (SUCCEEDED(hr))
        {
            cint width = 0;
            if (!e->GetWrapLine() && !e->GetMultiline())
            {
                width = (cint)ceil(metrics.widthIncludingTrailingWhitespace);
            }
            minSize = CSize(width, (cint)ceil(metrics.height));
        }
        textLayout->SetMaxWidth(maxWidth);
    }
    else
    {
        minSize = CSize();
    }
}

void SolidLabelElementRenderer::InitializeInternal()
{

}

void SolidLabelElementRenderer::FinalizeInternal()
{
    DestroyTextLayout();
    auto rt = renderTarget.lock();
    DestroyBrush(rt);
    DestroyTextFormat(rt);
}

#pragma endregion SolidLabel

#pragma region RoundBorder

RoundBorderElement::RoundBorderElement()
{

}

RoundBorderElement::~RoundBorderElement()
{
    renderer->Finalize();
}

CString RoundBorderElement::GetElementTypeName()
{
    return _T("RoundBorder");
}

cint RoundBorderElement::GetTypeId()
{
    return RoundBorder;
}

CColor RoundBorderElement::GetColor()
{
    return color;
}

void RoundBorderElement::SetColor(CColor value)
{
    if (color != value)
    {
        color = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

FLOAT RoundBorderElement::GetRadius()
{
    return radius;
}

void RoundBorderElement::SetRadius(FLOAT value)
{
    if (radius != value)
    {
        radius = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

void RoundBorderElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    auto rt = renderTarget.lock();
    if (e->flags.self_visible)
    {
        auto d2dRenderTarget = rt->GetDirect2DRenderTarget();
        d2dRenderTarget->FillRoundedRectangle(
            D2D1::RoundedRect(
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                e->GetRadius(),
                e->GetRadius()
            ),
            brush
        );
    }
    GraphicsRenderer::Render(bounds);
}

#pragma endregion RoundBorder