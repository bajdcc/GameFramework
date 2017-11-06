#ifndef RENDER_D2D_RENDER_H
#define RENDER_D2D_RENDER_H

#include <ui/gdi/Gdi.h>
#include "Direct2D.h"
#include "Direct2DRenderTarget.h"
#include <base/pe2d/PhysicsEngine2D.h>

#pragma region Base

enum ElementId
{
    Empty = 1000,
    SolidBackground,
    SolidLabel,
    GradientBackground,
    RoundBorder,
    QRImage = 1100,
    Base64Image = 1101,
    WireworldAutomaton = 1102,
    PhysicsEngine2D = 1103,
    Edit = 1200
};

using Alignment = Gdiplus::StringAlignment;

class IGraphicsElement;
class IGraphicsElementFactory;
class IGraphicsRenderer;
class IGraphicsRendererFactory;

class IGraphicsElement : public std::enable_shared_from_this<IGraphicsElement>
{
public:
    virtual ~IGraphicsElement(){}
    virtual cint GetTypeId() = 0;
    virtual std::shared_ptr<IGraphicsElementFactory> GetFactory() = 0;
    virtual std::shared_ptr<IGraphicsRenderer> GetRenderer() = 0;
    virtual void SetRenderRect(CRect bounds) = 0;
    virtual CRect GetRenderRect() = 0;
    virtual std::vector<std::shared_ptr<IGraphicsElement>>& GetChildren() = 0;

    struct GraphicsElementFlag
    {
        bool self_visible{ true };
        bool children_visible{ true };
        std::weak_ptr<IGraphicsElement> parent;
    };
    virtual GraphicsElementFlag& GetFlags() = 0;
};

class IGraphicsElementFactory : public std::enable_shared_from_this<IGraphicsElementFactory>
{
public:
    virtual ~IGraphicsElementFactory() {}
    virtual CString GetElementTypeName() = 0;
    virtual std::shared_ptr<IGraphicsElement> Create() = 0;
};

class IGraphicsRenderer : public std::enable_shared_from_this<IGraphicsRenderer>
{
public:
    virtual ~IGraphicsRenderer() {}
    virtual std::shared_ptr<IGraphicsRendererFactory> GetFactory() = 0;
    virtual void Initialize(std::shared_ptr<IGraphicsElement> element) = 0;
    virtual void Finalize() = 0;
    virtual std::shared_ptr<Direct2DRenderTarget> SetRenderTarget(std::shared_ptr<Direct2DRenderTarget> renderTarget) = 0;
    virtual void Render(CRect bounds) = 0;
    virtual void OnElementStateChanged() = 0;
    virtual CSize GetMinSize() = 0;
};

class IGraphicsRendererFactory : public std::enable_shared_from_this<IGraphicsRendererFactory>
{
public:
    virtual ~IGraphicsRendererFactory() {}
    virtual std::shared_ptr<IGraphicsRenderer> Create() = 0;
};

template <class TElement>
class GraphicsElement : public IGraphicsElement
{
public:
    class Factory : public IGraphicsElementFactory
    {
    public:
        Factory()
        {

        }
        CString GetElementTypeName()override
        {
            return TElement::GetElementTypeName();
        }
        std::shared_ptr<IGraphicsElement> Create()override
        {
            auto element = std::make_shared<TElement>();
            element->factory = shared_from_this();
            auto rendererFactory = Direct2D::Singleton().GetRendererFactory(GetElementTypeName());
            if (rendererFactory)
            {
                element->renderer = rendererFactory->Create();
                element->renderer->Initialize(element);
            }
            return element;
        }
    };
public:
    static std::shared_ptr<TElement> Create()
    {
        return std::dynamic_pointer_cast<TElement, IGraphicsElement>(
            Direct2D::Singleton().GetElementFactory(
                TElement::GetElementTypeName())->Create());
    }
    std::shared_ptr<IGraphicsElementFactory> GetFactory()override
    {
        return factory.lock();
    }
    std::shared_ptr<IGraphicsRenderer> GetRenderer()override
    {
        return renderer;
    }
    void SetRenderRect(CRect bounds)override
    {
        this->bounds = bounds;
    }
    CRect GetRenderRect()override
    {
        return bounds;
    }
    std::vector<std::shared_ptr<IGraphicsElement>>& GetChildren()override
    {
        return children;
    }
    GraphicsElementFlag& GetFlags()override
    {
        return flags;
    }
    GraphicsElementFlag flags;
protected:
    std::weak_ptr<IGraphicsElementFactory> factory;
    std::shared_ptr<IGraphicsRenderer> renderer;
    CRect bounds;
    std::vector<std::shared_ptr<IGraphicsElement>> children;
};

template <class TElement, class TRenderer, class TTarget>
class GraphicsRenderer : public IGraphicsRenderer
{
public:
    class Factory : public IGraphicsRendererFactory
    {
    public:
        Factory()
        {

        }
        std::shared_ptr<IGraphicsRenderer> Create()override
        {
            auto renderer = std::make_shared<TRenderer>();
            renderer->factory = shared_from_this();
            return renderer;
        }
    };
public:
    GraphicsRenderer()
    {

    }
    static void Register()
    {
        Direct2D::Singleton().RegisterFactories(
            std::dynamic_pointer_cast<IGraphicsElementFactory>(std::make_shared<typename TElement::Factory>()),
            std::dynamic_pointer_cast<IGraphicsRendererFactory>(std::make_shared<typename TRenderer::Factory>()));
    }
    std::shared_ptr<IGraphicsRendererFactory> GetFactory()override
    {
        return factory.lock();
    }
    void Initialize(std::shared_ptr<IGraphicsElement> _element)override
    {
        element = std::dynamic_pointer_cast<TElement, IGraphicsElement>(_element);
        InitializeInternal();
    }
    void Finalize()override
    {
        FinalizeInternal();
    }
    std::shared_ptr<Direct2DRenderTarget> SetRenderTarget(std::shared_ptr<Direct2DRenderTarget> _renderTarget)override
    {
        auto oldRenderTarget = renderTarget.lock();
        renderTarget = _renderTarget;
        RenderTargetChangedInternal(oldRenderTarget, renderTarget.lock());
        auto e = element.lock();
        for (std::shared_ptr<IGraphicsElement>& child : e->GetChildren())
        {
            child->GetRenderer()->SetRenderTarget(_renderTarget);
        }
        return oldRenderTarget;
    }
    CSize GetMinSize()override
    {
        return minSize;
    }
    void Render(CRect bounds)override
    {
        auto e = element.lock();
        if (e->flags.children_visible)
        {
            for (std::shared_ptr<IGraphicsElement>& child : e->GetChildren())
            {
                child->GetRenderer()->Render(child->GetRenderRect());
            }
        }
    }
    virtual void InitializeInternal() = 0;
    virtual void FinalizeInternal() = 0;
    virtual void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget) = 0;

protected:
    std::weak_ptr<IGraphicsRendererFactory>	factory;
    std::weak_ptr<TElement> element;
    std::weak_ptr<TTarget> renderTarget;
    CSize minSize;
};

template <class TElement, class TRenderer, class TBrush, class TBrushProperty>
class GraphicsBrushRenderer : public GraphicsRenderer<TElement, TRenderer, Direct2DRenderTarget>
{
    using base = GraphicsRenderer<TElement, TRenderer, Direct2DRenderTarget>;
protected:
    void InitializeInternal()override
    {

    }
    void FinalizeInternal()override
    {
        DestroyBrush(base::renderTarget.lock());
    }
    void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)override
    {
        DestroyBrush(oldRenderTarget);
        CreateBrush(newRenderTarget);
    }
    virtual void CreateBrush(std::shared_ptr<Direct2DRenderTarget> renderTarget) = 0;
    virtual void DestroyBrush(std::shared_ptr<Direct2DRenderTarget> renderTarget) = 0;

    TBrushProperty			oldColor;
    CComPtr<TBrush>			brush;
};

template <class TElement, class TRenderer, class TBrush, class TBrushProperty>
class GraphicsSolidBrushRenderer : public GraphicsBrushRenderer<TElement, TRenderer, TBrush, TBrushProperty>
{
    using base = GraphicsBrushRenderer<TElement, TRenderer, TBrush, TBrushProperty>;
public:
    void OnElementStateChanged()override
    {
        auto e = base::element.lock();
        auto rt = base::renderTarget.lock();
        if (rt)
        {
            TBrushProperty color = e->GetColor();
            if (base::oldColor != color)
            {
                DestroyBrush(rt);
                CreateBrush(rt);
            }
        }
    }

protected:
    void CreateBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget)
        {
            base::oldColor = base::element.lock()->GetColor();
            base::brush = _renderTarget->CreateDirect2DBrush(base::oldColor);
        }
    }
    void DestroyBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget && base::brush)
        {
            _renderTarget->DestroyDirect2DBrush(base::oldColor);
            base::brush = nullptr;
        }
    }
};

template <class TElement, class TRenderer, class TBrush, class TBrushProperty>
class GraphicsGradientBrushRenderer : public GraphicsBrushRenderer<TElement, TRenderer, TBrush, TBrushProperty>
{
    using base = GraphicsBrushRenderer<TElement, TRenderer, TBrush, TBrushProperty>;
public:
    void OnElementStateChanged()override
    {
        auto e = base::element.lock();
        auto rt = base::renderTarget.lock();
        if (rt)
        {
            TBrushProperty color = TBrushProperty(e->GetColor1(), e->GetColor2());
            if (base::oldColor != color)
            {
                DestroyBrush(rt);
                CreateBrush(rt);
            }
        }
    }

protected:
    void CreateBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget)
        {
            auto e = base::element.lock();
            base::oldColor = std::make_pair(e->GetColor1(), e->GetColor2());
            base::brush = _renderTarget->CreateDirect2DLinearBrush(base::oldColor.first, base::oldColor.second);
        }
    }
    void DestroyBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget && base::brush)
        {
            _renderTarget->DestroyDirect2DLinearBrush(base::oldColor.first, base::oldColor.second);
            base::brush = nullptr;
        }
    }
};

#pragma endregion Base

#pragma region Empty
class EmptyElement : public GraphicsElement<EmptyElement>
{
public:
    EmptyElement();
    ~EmptyElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;
};

class EmptyElementRenderer : public GraphicsRenderer<EmptyElement, EmptyElementRenderer, Direct2DRenderTarget>
{
public:
    void InitializeInternal()override;
    void FinalizeInternal()override;
    void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)override;
    void OnElementStateChanged()override;
};
#pragma endregion Empty

#pragma region SolidBackground
class SolidBackgroundElement : public GraphicsElement<SolidBackgroundElement>
{
public:
    SolidBackgroundElement();
    ~SolidBackgroundElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CColor GetColor();
    void SetColor(CColor value);
    bool IsFill();
    void SetFill(bool value);

protected:
    CColor color;
    bool fill{ true };
};

class SolidBackgroundElementRenderer : public GraphicsSolidBrushRenderer<SolidBackgroundElement, SolidBackgroundElementRenderer, ID2D1SolidColorBrush, CColor>
{
public:
    void Render(CRect bounds)override;
};
#pragma endregion SolidBackground

#pragma region GradientBackground
class GradientBackgroundElement : public GraphicsElement<GradientBackgroundElement>
{
public:
    GradientBackgroundElement();
    ~GradientBackgroundElement();

    enum Direction
    {
        Horizontal,
        Vertical,
        Slash,
        Backslash,
    };

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CColor GetColor1();
    void SetColor1(CColor value);
    CColor GetColor2();
    void SetColor2(CColor value);
    void SetColors(CColor value1, CColor value2);
    Direction GetDirection();
    void SetDirection(Direction value);

protected:
    CColor color1;
    CColor color2;
    Direction direction{ Horizontal };
};

class GradientBackgroundElementRenderer : public GraphicsGradientBrushRenderer<GradientBackgroundElement,
    GradientBackgroundElementRenderer, ID2D1LinearGradientBrush, std::pair<CColor, CColor>>
{
public:
    void Render(CRect bounds)override;
};
#pragma endregion GradientBackground

#pragma region SolidLabel
class SolidLabelElement : public GraphicsElement<SolidLabelElement>
{
public:
    SolidLabelElement();
    ~SolidLabelElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CColor GetColor();
    void SetColor(CColor value);
    const Font& GetFont();
    void SetFont(const Font& value);
    const CString& GetText();
    void SetText(const CString& value);

    Alignment GetHorizontalAlignment();
    Alignment GetVerticalAlignment();
    void SetHorizontalAlignment(Alignment value);
    void SetVerticalAlignment(Alignment value);
    void SetAlignments(Alignment horizontal, Alignment vertical);

protected:
    CColor color;
    Font fontProperties;
    CString text;
    Alignment hAlignment{ Alignment::StringAlignmentNear };
    Alignment vAlignment{ Alignment::StringAlignmentNear };
};

class SolidLabelElementRenderer : public GraphicsRenderer<SolidLabelElement, SolidLabelElementRenderer, Direct2DRenderTarget>
{
public:
    SolidLabelElementRenderer();

    void Render(CRect bounds)override;
    void OnElementStateChanged()override;

protected:
    void CreateBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void DestroyBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void CreateTextFormat(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void DestroyTextFormat(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void CreateTextLayout();
    void DestroyTextLayout();
    void UpdateMinSize();

    void InitializeInternal()override;
    void FinalizeInternal()override;
    void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)override;

    CColor oldColor;
    Font oldFont;
    CString oldText;
    CComPtr<ID2D1SolidColorBrush> brush;
    std::shared_ptr<D2DTextFormatPackage> textFormat;
    CComPtr<IDWriteTextLayout> textLayout;
    cint oldMaxWidth{ -1 };
};
#pragma endregion SolidLabel

#pragma region RoundBorder
class RoundBorderElement : public GraphicsElement<RoundBorderElement>
{
public:
    RoundBorderElement();
    ~RoundBorderElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CColor GetColor();
    void SetColor(CColor value);
    FLOAT GetRadius();
    void SetRadius(FLOAT value);
    bool IsFill();
    void SetFill(bool value);

protected:
    CColor color;
    FLOAT radius{ 1.0f };
    bool fill{ true };
};

class RoundBorderElementRenderer : public GraphicsSolidBrushRenderer<RoundBorderElement, RoundBorderElementRenderer, ID2D1SolidColorBrush, CColor>
{
public:
    void Render(CRect bounds)override;
};
#pragma endregion SolidBackground

#pragma region Image
template <class TElement, class TRenderer>
class GraphicsImageRenderer : public GraphicsRenderer<TElement, TRenderer, Direct2DRenderTarget>
{
    using base = GraphicsRenderer<TElement, TRenderer, Direct2DRenderTarget>;
protected:
    void InitializeInternal()override
    {

    }
    void FinalizeInternal()override
    {
        DestroyImage(base::renderTarget.lock());
    }
    void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)override
    {
        DestroyImage(oldRenderTarget);
        CreateImage(newRenderTarget);
    }
    virtual void CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget) = 0;
    virtual void DestroyImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)
    {
        bitmap = nullptr;
    }
    void OnElementStateChanged()override
    {
        DestroyImage(base::renderTarget.lock());
        CreateImage(base::renderTarget.lock());
    }

    CComPtr<ID2D1Bitmap> bitmap;
};

class QRImageElement : public GraphicsElement<QRImageElement>
{
public:
    QRImageElement();
    ~QRImageElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CColor GetColor()const;
    void SetColor(CColor value);
    CStringA GetText()const;
    void SetText(CStringA value);
    FLOAT GetOpacity()const;
    void SetOpacity(FLOAT value);

protected:
    CColor color;
    CStringA text;
    FLOAT opacity;
};

class QRImageElementRenderer : public GraphicsImageRenderer<QRImageElement, QRImageElementRenderer>
{
protected:
    void CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)override;
public:
    void Render(CRect bounds)override;
};

class Base64ImageElement : public GraphicsElement<Base64ImageElement>
{
public:
    Base64ImageElement();
    ~Base64ImageElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CStringA GetText()const;
    void SetText(CStringA value);
    CStringA GetUrl()const;
    void SetUrl(CStringA value);
    FLOAT GetOpacity()const;
    void SetOpacity(FLOAT value);

protected:
    CStringA text;
    CStringA url;
    FLOAT opacity;
};

class Base64ImageElementRenderer : public GraphicsImageRenderer<Base64ImageElement, Base64ImageElementRenderer>
{
protected:
    void CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)override;
public:
    void Render(CRect bounds)override;
private:
    CComPtr<IWICBitmap> wic;
    CStringA text;
    CStringA url;
};

class WireworldAutomatonImageElement : public GraphicsElement<WireworldAutomatonImageElement>
{
public:
    WireworldAutomatonImageElement();
    ~WireworldAutomatonImageElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CStringA GetText()const;
    void SetText(CStringA value);
    FLOAT GetOpacity()const;
    void SetOpacity(FLOAT value);

    void Refresh(int arg);

protected:
    CStringA text;
    FLOAT opacity;
};

class WireworldAutomatonImageElementRenderer : public GraphicsImageRenderer<WireworldAutomatonImageElement, WireworldAutomatonImageElementRenderer>
{
protected:
    void CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)override;
public:
    void Render(CRect bounds)override;
    ~WireworldAutomatonImageElementRenderer();
    void Refresh(int arg);
private:
    CComPtr<IWICBitmap> wic;
    BYTE* buffer{ nullptr };
    std::vector<byte> data;
    WICRect rect;
    D2D1_RECT_U d2dRect;
    std::vector<INT> wires;
    std::vector<INT> heads;
    std::vector<INT> tails;
};

#pragma endregion Image

#pragma region Edit
class EditElement : public GraphicsElement<EditElement>
{
public:
    EditElement();
    ~EditElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    CColor GetColor();
    void SetColor(CColor value);
    const Font& GetFont();
    void SetFont(const Font& value);
    const CString& GetText();
    void SetText(const CString& value);
    const bool IsMultiline();
    void SetMultiline(const bool& value);

protected:
    CColor color;
    Font fontProperties;
    CString text;
    bool multiline{ false };
};

class EditElementRenderer : public GraphicsRenderer<EditElement, EditElementRenderer, Direct2DRenderTarget>
{
public:
    void Render(CRect bounds)override;
    void OnElementStateChanged()override;

protected:
    void CreateBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void DestroyBrush(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void CreateTextFormat(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void DestroyTextFormat(std::shared_ptr<Direct2DRenderTarget> _renderTarget);
    void CreateTextLayout();
    void DestroyTextLayout();
    void UpdateMinSize();

    void InitializeInternal()override;
    void FinalizeInternal()override;
    void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)override;

    CColor oldColor;
    Font oldFont;
    CString oldText;
    CComPtr<ID2D1SolidColorBrush> brush;
    std::shared_ptr<D2DTextFormatPackage> textFormat;
    CComPtr<IDWriteTextLayout> textLayout;
    cint oldMaxWidth{ -1 };
};
#pragma endregion Edit

#pragma region PhysicsEngine2D

class PhysicsEngine2DElement : public GraphicsElement<PhysicsEngine2DElement>
{
public:
    PhysicsEngine2DElement();
    ~PhysicsEngine2DElement();

    static CString GetElementTypeName();

    cint GetTypeId()override;

    FLOAT GetOpacity()const;
    void SetOpacity(FLOAT value);

    cint GetType()const;
    void SetType(cint value);

    void Refresh(int arg);

protected:
    CStringA text;
    FLOAT opacity{ 0 };
    cint type{ 0 };
};

class PhysicsEngine2DElementRenderer : public GraphicsRenderer<PhysicsEngine2DElement, PhysicsEngine2DElementRenderer, Direct2DRenderTarget>
{
public:
    void Render(CRect bounds)override;
    ~PhysicsEngine2DElementRenderer();
    void Refresh(int arg);

    void OnElementStateChanged()override;

protected:
    void InitializeInternal()override;
    void FinalizeInternal()override;
    void RenderTargetChangedInternal(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)override;

private:
    PhysicsEngine pe;
};

#pragma endregion PhysicsEngine2D

#endif