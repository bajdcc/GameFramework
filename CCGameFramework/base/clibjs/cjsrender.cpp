//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsrender.h"
#include "cjsgui.h"
#include <render\Direct2D.h>
#include <libqrencode\qrencode.h>

namespace clib {

    static void init_renderer()
    {
        cjsrender_rect_renderer::register_element();
        cjsrender_round_renderer::register_element();
        cjsrender_label_renderer::register_element();
        cjsrender_qr_renderer::register_element();
    }

    bool cjsrender::register_element_factory(cjsrender_element_factory::ref factory)
    {
        return elementFactories.insert({ factory->get_name(), factory }).second;
    }

    bool cjsrender::register_renderer_factory(const std::string& elementTypeName, cjsrender_renderer_factory::ref factory)
    {
        return rendererFactories.insert({ elementTypeName, factory }).second;
    }

    cjsrender_element_factory::ref cjsrender::get_element_factory(const std::string& elementTypeName)
    {
        if (elementFactories.empty())
            init_renderer();
        auto found = elementFactories.find(elementTypeName);
        assert(found != elementFactories.end());
        return found == elementFactories.end() ? nullptr : found->second;
    }

    cjsrender_renderer_factory::ref cjsrender::get_renderer_factory(const std::string& elementTypeName)
    {
        if (rendererFactories.empty())
            init_renderer();
        auto found = rendererFactories.find(elementTypeName);
        assert(found != rendererFactories.end());
        return found == rendererFactories.end() ? nullptr : found->second;
    }

    bool cjsrender::register_factories(cjsrender_element_factory::ref elementFactory, cjsrender_renderer_factory::ref rendererFactory)
    {
        if (elementFactory && rendererFactory)
        {
            if (register_element_factory(elementFactory))
            {
                if (register_renderer_factory(elementFactory->get_name(), rendererFactory))
                {
                    return true;
                }
            }
        }
        return false;
    }

    cjsrender& cjsrender::singleton()
    {
        static cjsrender r;
        return r;
    }

#pragma region rect

    cjsrender_rect::~cjsrender_rect()
    {
        renderer->destroy2();
    }

    std::string cjsrender_rect::get_name()
    {
        return "rect";
    }

    int cjsrender_rect::get_type()
    {
        return r_rect;
    }

    CColor cjsrender_rect::get_color() const
    {
        return color;
    }

    void cjsrender_rect::set_color(CColor value)
    {
        if (color != value)
        {
            color = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    bool cjsrender_rect::is_fill() const
    {
        return fill;
    }

    void cjsrender_rect::set_fill(bool value)
    {
        if (fill != value)
        {
            fill = value;
        }
    }

    void cjsrender_rect_renderer::render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
    {
        auto e = element.lock();
        if (e->is_fill())
        {
            r->FillRectangle(
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                brush
            );
        }
        else
        {
            r->DrawRectangle(
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                brush
            );
        }
    }

    void cjsrender_rect_renderer::init2()
    {
        if (GLOBAL_STATE.canvas.lock())
            brush = GLOBAL_STATE.canvas.lock()->CreateDirect2DBrush(element.lock()->get_color());
    }

    void cjsrender_rect_renderer::destroy2()
    {
        brush = nullptr;
    }

#pragma endregion rect

#pragma region round

    cjsrender_round::~cjsrender_round()
    {
        renderer->destroy2();
    }

    std::string cjsrender_round::get_name()
    {
        return "round";
    }

    int cjsrender_round::get_type()
    {
        return r_round;
    }

    CColor cjsrender_round::get_color() const
    {
        return color;
    }

    void cjsrender_round::set_color(CColor value)
    {
        if (color != value)
        {
            color = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    bool cjsrender_round::is_fill() const
    {
        return fill;
    }

    void cjsrender_round::set_fill(bool value)
    {
        if (fill != value)
        {
            fill = value;
        }
    }

    float cjsrender_round::get_radius() const
    {
        return radius;
    }

    void cjsrender_round::set_radius(float value)
    {
        if (radius != value)
        {
            radius = value;
        }
    }

    void cjsrender_round_renderer::render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
    {
        auto e = element.lock();
        if (e->is_fill())
        {
            r->FillRoundedRectangle(
                D2D1::RoundedRect(
                    D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                    e->get_radius(),
                    e->get_radius()
                ),
                brush
            );
        }
        else
        {
            r->DrawRoundedRectangle(
                D2D1::RoundedRect(
                    D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                    e->get_radius(),
                    e->get_radius()
                ),
                brush
            );
        }
    }

    void cjsrender_round_renderer::init2()
    {
        if (GLOBAL_STATE.canvas.lock())
            brush = GLOBAL_STATE.canvas.lock()->CreateDirect2DBrush(element.lock()->get_color());
    }

    void cjsrender_round_renderer::destroy2()
    {
        brush = nullptr;
    }

#pragma endregion round

#pragma region label

    cjsrender_label::~cjsrender_label()
    {
        renderer->destroy2();
    }

    std::string cjsrender_label::get_name()
    {
        return "label";
    }

    int cjsrender_label::get_type()
    {
        return r_label;
    }

    CColor cjsrender_label::get_color() const
    {
        return color;
    }

    void cjsrender_label::set_color(CColor value)
    {
        if (color != value)
        {
            color = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    const Font& cjsrender_label::get_font() const
    {
        return fontProperties;
    }

    void cjsrender_label::set_font(const Font& value)
    {
        if (fontProperties != value)
        {
            fontProperties = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    const CString& cjsrender_label::get_text() const
    {
        return text;
    }

    void cjsrender_label::set_text(const CString& value)
    {
        if (text != value)
        {
            text = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    cjsrender_label::Alignment cjsrender_label::get_horizontal_alignment()
    {
        return hAlignment;
    }

    cjsrender_label::Alignment cjsrender_label::get_vertical_alignment()
    {
        return vAlignment;
    }

    void cjsrender_label::set_horizontal_alignment(Alignment value)
    {
        if (hAlignment != value)
        {
            hAlignment = value;
        }
    }

    void cjsrender_label::set_vertical_alignment(Alignment value)
    {
        if (vAlignment != value)
        {
            vAlignment = value;
        }
    }

    void cjsrender_label_renderer::render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
    {
        auto e = element.lock();
        if (bounds.IsRectEmpty())
            return;

        if (!textLayout)
        {
            create_layout();
        }

        cint x = 0;
        cint y = 0;
        switch (e->get_horizontal_alignment())
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
        switch (e->get_vertical_alignment())
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

        auto rt = GLOBAL_STATE.canvas.lock();
        rt->SetTextAntialias(e->get_font().antialias, e->get_font().verticalAntialias);

        DWRITE_TRIMMING trimming;
        CComPtr<IDWriteInlineObject> inlineObject;
        textLayout->GetTrimming(&trimming, &inlineObject);
        textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
        trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
        trimming.delimiter = 1;
        trimming.delimiterCount = 3;
        switch (e->get_horizontal_alignment())
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

        switch (e->get_vertical_alignment())
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

        CRect textBounds = bounds;
        textLayout->SetMaxWidth((FLOAT)textBounds.Width());
        textLayout->SetMaxHeight((FLOAT)textBounds.Height());
        textLayout->SetTrimming(&trimming, inlineObject);

        r->DrawTextLayout(
            D2D1::Point2F((FLOAT)textBounds.left, (FLOAT)textBounds.top),
            textLayout,
            brush,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
        );

        if (oldMaxWidth != textBounds.Width())
        {
            oldMaxWidth = textBounds.Width();
            update_min_size();
        }
    }

    void cjsrender_label_renderer::init2()
    {
        if (GLOBAL_STATE.canvas.lock()) {
            brush = GLOBAL_STATE.canvas.lock()->CreateDirect2DBrush(element.lock()->get_color());
            create_format();
        }
    }

    void cjsrender_label_renderer::destroy2()
    {
        brush = nullptr;
        textLayout = nullptr;
        textFormat = nullptr;
    }

    void cjsrender_label_renderer::on_changed()
    {
        cjsrender_renderer::on_changed();
        update_min_size();
    }

    void cjsrender_label_renderer::update_min_size()
    {
        float maxWidth = 0;
        textLayout = nullptr;
        bool calculateSizeFromTextLayout = false;
        auto e = element.lock();
        create_layout();
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
        if (calculateSizeFromTextLayout)
        {
            DWRITE_TEXT_METRICS metrics;
            HRESULT hr = textLayout->GetMetrics(&metrics);
            if (SUCCEEDED(hr))
            {
                cint width = (cint)ceil(metrics.widthIncludingTrailingWhitespace);
                minSize = CSize(width, (cint)ceil(metrics.height));
            }
            textLayout->SetMaxWidth(maxWidth);
        }
        else
        {
            minSize = CSize();
        }
    }

    void cjsrender_label_renderer::create_layout()
    {
        if (textFormat)
        {
            auto e = element.lock();
            BSTR _text = e->get_text().AllocSysString();
            HRESULT hr = Direct2D::Singleton().GetDirectWriteFactory()->CreateTextLayout(
                _text,
                e->get_text().GetLength(),
                textFormat->textFormat,
                0,
                0,
                &textLayout);
            SysFreeString(_text);
            if (SUCCEEDED(hr))
            {
                if (e->get_font().underline)
                {
                    DWRITE_TEXT_RANGE textRange;
                    textRange.startPosition = 0;
                    textRange.length = e->get_text().GetLength();
                    textLayout->SetUnderline(TRUE, textRange);
                }
                if (e->get_font().strikeline)
                {
                    DWRITE_TEXT_RANGE textRange;
                    textRange.startPosition = 0;
                    textRange.length = e->get_text().GetLength();
                    textLayout->SetStrikethrough(TRUE, textRange);
                }
            }
            else
            {
                textLayout = nullptr;
            }
        }
    }

    void cjsrender_label_renderer::create_format() {
        auto font = element.lock()->get_font();
        textFormat = GLOBAL_STATE.canvas.lock()->CreateDirect2DTextFormat(font);
        if (!(textFormat && textFormat->textFormat)) {
            auto newFont = font;
            newFont.fontFamily = _T("Microsoft Yahei");
            newFont.size = 12;
            element.lock()->set_font(newFont);
        }
    }

#pragma endregion label

#pragma region qr

    cjsrender_qr::~cjsrender_qr()
    {
        renderer->destroy2();
    }

    std::string cjsrender_qr::get_name()
    {
        return "qr";
    }

    int cjsrender_qr::get_type()
    {
        return r_qr;
    }

    CColor cjsrender_qr::get_color() const
    {
        return color;
    }

    void cjsrender_qr::set_color(CColor value)
    {
        if (color != value)
        {
            color = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    std::string cjsrender_qr::get_text() const
    {
        return text;
    }

    void cjsrender_qr::set_text(const std::string& value)
    {
        if (text != value)
        {
            text = value;
            if (renderer)
            {
                renderer->destroy2();
                if (!text.empty())
                    renderer->on_changed();
            }
        }
    }

    CColor cjsrender_qr::get_background() const
    {
        return background;
    }

    void cjsrender_qr::set_background(CColor value)
    {
        if (background != value)
        {
            background = value;
            if (renderer)
            {
                renderer->on_changed();
            }
        }
    }

    void cjsrender_qr_renderer::render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
    {
        auto e = element.lock();
        if (!bitmap && !e->get_text().empty()) {
            init2();
        }
        if (bitmap) {
            r->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
            );
        }
    }

    void cjsrender_qr_renderer::init2()
    {
        if (!bitmap && GLOBAL_STATE.canvas.lock()) {
            auto e = element.lock();
            if (e->get_text().empty())
                return;
            auto qrcode = QRcode_encodeString(
                e->get_text().c_str(),
                0,
                QR_ECLEVEL_H,
                QR_MODE_8,
                1
            );
            auto rt = GLOBAL_STATE.canvas.lock();
            auto WICBitmap = rt->CreateBitmap(qrcode->width, qrcode->width);
            bitmap = rt->GetBitmapFromWIC(WICBitmap);
            WICRect rect;
            rect.X = 0;
            rect.Y = 0;
            rect.Width = bitmap->GetPixelSize().width;
            rect.Height = bitmap->GetPixelSize().height;
            std::vector<BYTE> buffer(rect.Width * rect.Height * 4);
            auto count = rect.Width * rect.Height;
            DWORD* read = (DWORD*)buffer.data();
            auto color = e->get_color();
            auto background = e->get_background();
            for (auto i = 0; i < count; i++)
            {
                if ((qrcode->data[i] & 1) == 0)
                {
                    *reinterpret_cast<DWORD*>(read) = background.value;
                }
                else
                {
                    *reinterpret_cast<DWORD*>(read) = color.value;
                }
                read++;
            }
            QRcode_free(qrcode);
            D2D1_RECT_U d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
            bitmap->CopyFromMemory(&d2dRect, buffer.data(), rect.Width * 4);
        }
    }

    void cjsrender_qr_renderer::destroy2()
    {
        bitmap = nullptr;
    }

#pragma endregion qr
}

