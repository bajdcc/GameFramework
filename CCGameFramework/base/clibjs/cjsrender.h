//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSRENDER_H
#define CLIBJS_CJSRENDER_H

#include <memory>
#include <cassert>
#include <string>
#include <ui\gdi\Gdi.h>
#include <d2d1.h>
#include <atlcomcli.h>
#include <unordered_map>
#include <render\Direct2DRenderTarget.h>

namespace clib {

    class cjsrender_element_base;
    class cjsrender_element_factory;
    class cjsrender_renderer_base;
    class cjsrender_renderer_factory;

    enum cjsrender_t
    {
        r_empty = 0,
        r_rect,
        r_round,
        r_label,
        r_qr,
    };

    class cjsrender_element_base : public std::enable_shared_from_this<cjsrender_element_base>
    {
    public:
        using ref = std::shared_ptr<cjsrender_element_base>;
        virtual ~cjsrender_element_base() = default;
        virtual int get_type() = 0;
        virtual std::shared_ptr<cjsrender_element_factory> get_factory() = 0;
        virtual std::shared_ptr<cjsrender_renderer_base> get_renderer() = 0;
    };

    class cjsrender_element_factory : public std::enable_shared_from_this<cjsrender_element_factory>
    {
    public:
        using ref = std::shared_ptr<cjsrender_element_factory>;
        using weak_ref = std::weak_ptr<cjsrender_element_factory>;
        virtual ~cjsrender_element_factory() = default;
        virtual std::string get_name() = 0;
        virtual cjsrender_element_base::ref create() = 0;
    };

    class cjsrender_renderer_base : public std::enable_shared_from_this<cjsrender_renderer_base>
    {
    public:
        using ref = std::shared_ptr<cjsrender_renderer_base>;
        virtual ~cjsrender_renderer_base() = default;
        virtual std::shared_ptr<cjsrender_renderer_factory> get_factory() = 0;
        virtual void init(cjsrender_element_base::ref element) = 0;
        virtual void destroy() = 0;
        virtual void init2() = 0;
        virtual void destroy2() = 0;
        virtual void render(CRect bounds, CComPtr<ID2D1RenderTarget> = nullptr) = 0;
        virtual void on_changed() = 0;
        virtual CSize get_min_size() = 0;
    };

    class cjsrender_renderer_factory : public std::enable_shared_from_this<cjsrender_renderer_factory>
    {
    public:
        using ref = std::shared_ptr<cjsrender_renderer_factory>;
        using weak_ref = std::weak_ptr<cjsrender_renderer_factory>;
        virtual ~cjsrender_renderer_factory() = default;
        virtual cjsrender_renderer_base::ref create() = 0;
    };

    class cjsrender
    {
    public:
        cjsrender() = default;
        ~cjsrender() = default;

        using elementFactoryMap = std::unordered_map<std::string, cjsrender_element_factory::ref>;
        using rendererFactoryMap = std::unordered_map<std::string, cjsrender_renderer_factory::ref>;

        elementFactoryMap elementFactories;
        rendererFactoryMap rendererFactories;

        bool register_element_factory(cjsrender_element_factory::ref factory);
        bool register_renderer_factory(const std::string& elementTypeName, cjsrender_renderer_factory::ref factory);

        cjsrender_element_factory::ref get_element_factory(const std::string& elementTypeName);
        cjsrender_renderer_factory::ref get_renderer_factory(const std::string& elementTypeName);
        bool register_factories(cjsrender_element_factory::ref elementFactory, cjsrender_renderer_factory::ref rendererFactory);

        static cjsrender& singleton();
    };

    template <class TElement>
    class cjsrender_element : public cjsrender_element_base
    {
    public:
        class factory : public cjsrender_element_factory
        {
        public:
            factory() = default;
            std::string get_name()override
            {
                return TElement::get_name();
            }
            cjsrender_element_base::ref create()override
            {
                auto element = std::make_shared<TElement>();
                element->_factory = shared_from_this();
                auto rendererFactory = cjsrender::singleton().get_renderer_factory(get_name());
                assert(rendererFactory);
                if (rendererFactory)
                {
                    element->renderer = rendererFactory->create();
                    element->renderer->init(element);
                }
                return element;
            }
        };
    public:
        static std::shared_ptr<TElement> create()
        {
            return std::dynamic_pointer_cast<TElement, cjsrender_element_base>(
                cjsrender::singleton().get_element_factory(
                    TElement::get_name())->create());
        }
        cjsrender_element_factory::ref get_factory()override
        {
            return _factory.lock();
        }
        cjsrender_renderer_base::ref get_renderer()override
        {
            return renderer;
        }
    protected:
        cjsrender_element_factory::weak_ref _factory;
        cjsrender_renderer_base::ref renderer;
    };

    template <class TElement, class TRenderer, class TTarget>
    class cjsrender_renderer : public cjsrender_renderer_base
    {
    public:
        class factory : public cjsrender_renderer_factory
        {
        public:
            factory() = default;
            cjsrender_renderer_base::ref create()override
            {
                auto renderer = std::make_shared<TRenderer>();
                renderer->_factory = shared_from_this();
                return renderer;
            }
        };
    public:
        cjsrender_renderer() = default;
        static void register_element()
        {
            cjsrender::singleton().register_factories(
                std::dynamic_pointer_cast<cjsrender_element_factory>(std::make_shared<typename TElement::factory>()),
                std::dynamic_pointer_cast<cjsrender_renderer_factory>(std::make_shared<typename TRenderer::factory>()));
        }
        cjsrender_renderer_factory::ref get_factory()override
        {
            return _factory.lock();
        }
        void init(cjsrender_element_base::ref _element)override
        {
            element = std::dynamic_pointer_cast<TElement, cjsrender_element_base>(_element);
            init2();
        }
        void destroy()override
        {
            destroy2();
        }
        CSize get_min_size()override
        {
            return minSize;
        }
        void on_changed()override
        {
            destroy2();
            init2();
        }
        virtual void render(CRect bounds, CComPtr<ID2D1RenderTarget> rt) = 0;

    protected:
        cjsrender_renderer_factory::weak_ref _factory;
        std::weak_ptr<TElement> element;
        CSize minSize;
    };

#pragma region rect
    class cjsrender_rect : public cjsrender_element<cjsrender_rect>
    {
    public:
        using ref = std::shared_ptr<cjsrender_rect>;
        using weak_ref = std::weak_ptr<cjsrender_rect>;

        cjsrender_rect() = default;
        ~cjsrender_rect();

        static std::string get_name();

        int get_type()override;

        CColor get_color() const;
        void set_color(CColor value);
        bool is_fill() const;
        void set_fill(bool value);

    protected:
        CColor color;
        bool fill{ true };
    };

    class cjsrender_rect_renderer : public cjsrender_renderer<cjsrender_rect, cjsrender_rect_renderer, Direct2DRenderTarget>
    {
    public:
        void render(CRect bounds, CComPtr<ID2D1RenderTarget>)override;
        void init2()override;
        void destroy2()override;
    private:
        CComPtr<ID2D1SolidColorBrush> brush;
    };
#pragma endregion rect

#pragma region round
    class cjsrender_round : public cjsrender_element<cjsrender_round>
    {
    public:
        using ref = std::shared_ptr<cjsrender_round>;
        using weak_ref = std::weak_ptr<cjsrender_round>;

        cjsrender_round() = default;
        ~cjsrender_round();

        static std::string get_name();

        int get_type()override;

        CColor get_color() const;
        void set_color(CColor value);
        bool is_fill() const;
        void set_fill(bool value);
        float get_radius() const;
        void set_radius(float value);

    protected:
        CColor color;
        bool fill{ true };
        float radius{ 0 };
    };

    class cjsrender_round_renderer : public cjsrender_renderer<cjsrender_round, cjsrender_round_renderer, Direct2DRenderTarget>
    {
    public:
        void render(CRect bounds, CComPtr<ID2D1RenderTarget>)override;
        void init2()override;
        void destroy2()override;
    private:
        CComPtr<ID2D1SolidColorBrush> brush;
    };
#pragma endregion round

#pragma region label
    class cjsrender_label : public cjsrender_element<cjsrender_label>
    {
    public:
        using ref = std::shared_ptr<cjsrender_label>;
        using weak_ref = std::weak_ptr<cjsrender_label>;
        using Alignment = Gdiplus::StringAlignment;

        cjsrender_label() = default;
        ~cjsrender_label();

        static std::string get_name();

        int get_type()override;

        CColor get_color() const;
        void set_color(CColor value);
        const Font& get_font() const;
        void set_font(const Font& value);
        const CString& get_text() const;
        void set_text(const CString& value);

        Alignment get_horizontal_alignment();
        Alignment get_vertical_alignment();
        void set_horizontal_alignment(Alignment value);
        void set_vertical_alignment(Alignment value);

    protected:
        CColor color;
        Font fontProperties;
        CString text;
        Alignment hAlignment{ Alignment::StringAlignmentNear };
        Alignment vAlignment{ Alignment::StringAlignmentNear };
    };

    class cjsrender_label_renderer : public cjsrender_renderer<cjsrender_label, cjsrender_label_renderer, Direct2DRenderTarget>
    {
    public:
        using Alignment = cjsrender_label::Alignment;
        void render(CRect bounds, CComPtr<ID2D1RenderTarget>)override;
        void init2()override;
        void destroy2()override;
        void on_changed()override;
    private:
        void update_min_size();
        void create_layout();
        void create_format();
    private:
        CComPtr<ID2D1SolidColorBrush> brush;
        std::shared_ptr<D2DTextFormatPackage> textFormat;
        CComPtr<IDWriteTextLayout> textLayout;
        cint oldMaxWidth{ -1 };
    };
#pragma endregion label

#pragma region qr
    class cjsrender_qr : public cjsrender_element<cjsrender_qr>
    {
    public:
        using ref = std::shared_ptr<cjsrender_qr>;
        using weak_ref = std::weak_ptr<cjsrender_qr>;

        cjsrender_qr() = default;
        ~cjsrender_qr();

        static std::string get_name();

        int get_type()override;

        CColor get_color() const;
        void set_color(CColor value);
        std::string get_text() const;
        void set_text(const std::string& value);
        CColor get_background() const;
        void set_background(CColor value);

    protected:
        CColor color;
        CColor background{ 255,255,255 };
        bool fill{ true };
        std::string text;
    };

    class cjsrender_qr_renderer : public cjsrender_renderer<cjsrender_qr, cjsrender_qr_renderer, Direct2DRenderTarget>
    {
    public:
        void render(CRect bounds, CComPtr<ID2D1RenderTarget>)override;
        void init2()override;
        void destroy2()override;
    private:
        CComPtr<ID2D1Bitmap> bitmap;
    };
#pragma endregion qr
}

#endif