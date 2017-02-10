#ifndef RENDER_D2D_RENDER_H
#define RENDER_D2D_RENDER_H

#include <WTF/RefPtr.h>
#include <ui/gdi/Gdi.h>
#include "Direct2D.h"
#include "Direct2DRenderTarget.h"

#pragma region Base

enum ElementId
{
    Empty = 1000,
    SolidBackground,
    SolidLabel,
    GradientBackground,
    QRImage = 1100,
};

using Alignment = Gdiplus::StringAlignment;

class IGraphicsElement;
class IGraphicsElementFactory;
class IGraphicsRenderer;
class IGraphicsRendererFactory;

class IGraphicsElement : public RefCounted<IGraphicsElement>
{
public:
    virtual cint GetTypeId() = 0;
    virtual PassRefPtr<IGraphicsElementFactory> GetFactory() = 0;
    virtual PassRefPtr<IGraphicsRenderer> GetRenderer() = 0;
    virtual void SetRenderRect(CRect bounds) = 0;
    virtual CRect GetRenderRect() = 0;
    virtual std::vector<RefPtr<IGraphicsElement>>& GetChildren() = 0;

    struct GraphicsElementFlag
    {
        bool self_visible{ true };
        bool children_visible{ true };
        RawPtr<IGraphicsElement> parent;
    };
    virtual GraphicsElementFlag& GetFlags() = 0;
};

class IGraphicsElementFactory : public RefCounted<IGraphicsElementFactory>
{
public:
    virtual CString GetElementTypeName() = 0;
    virtual PassRefPtr<IGraphicsElement> Create() = 0;
};

class IGraphicsRenderer : public RefCounted<IGraphicsRenderer>
{
public:
    virtual PassRefPtr<IGraphicsRendererFactory> GetFactory() = 0;

    virtual void Initialize(PassRefPtr<IGraphicsElement> element) = 0;
    virtual void Finalize() = 0;
    virtual PassRefPtr<Direct2DRenderTarget> SetRenderTarget(PassRefPtr<Direct2DRenderTarget> renderTarget) = 0;
    virtual void Render(CRect bounds) = 0;
    virtual void OnElementStateChanged() = 0;
    virtual CSize GetMinSize() = 0;
};

class IGraphicsRendererFactory : public RefCounted<IGraphicsRendererFactory>
{
public:
    virtual PassRefPtr<IGraphicsRenderer> Create() = 0;
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
        PassRefPtr<IGraphicsElement> Create()override
        {
            RefPtr<TElement> element = adoptRef(new TElement);
            element->factory = this;
            RefPtr<IGraphicsRendererFactory> rendererFactory = Direct2D::Singleton().GetRendererFactory(GetElementTypeName());
            if (rendererFactory)
            {
                element->renderer = rendererFactory->Create();
                element->renderer->Initialize(element);
            }
            return element;
        }
    };
public:
    static PassRefPtr<TElement> Create()
    {
        return Direct2D::Singleton().GetElementFactory(TElement::GetElementTypeName())->Create();
    }
    PassRefPtr<IGraphicsElementFactory> GetFactory()override
    {
        return factory;
    }
    PassRefPtr<IGraphicsRenderer> GetRenderer()override
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
    std::vector<RefPtr<IGraphicsElement>>& GetChildren()override
    {
        return children;
    }
    GraphicsElementFlag& GetFlags()override
    {
        return flags;
    }
    GraphicsElementFlag flags;
protected:
    RawPtr<IGraphicsElementFactory>			factory;
    RefPtr<IGraphicsRenderer>				renderer;
    CRect                                   bounds;
    std::vector<RefPtr<IGraphicsElement>>   children;
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
        PassRefPtr<IGraphicsRenderer> Create()override
        {
            RefPtr<TRenderer> renderer = adoptRef(new TRenderer);
            renderer->factory = this;
            renderer->element = nullptr;
            renderer->renderTarget = nullptr;
            return renderer;
        }
    };
public:
    GraphicsRenderer()
    {

    }
    static void Register()
    {
        Direct2D::Singleton().RegisterFactories(adoptRef(new TElement::Factory), adoptRef(new TRenderer::Factory));
    }
    PassRefPtr<IGraphicsRendererFactory> GetFactory()override
    {
        return factory;
    }
    void Initialize(PassRefPtr<IGraphicsElement> _element)override
    {
        element = dynamic_cast<TElement*>(_element.get());
        InitializeInternal();
    }
    void Finalize()override
    {
        FinalizeInternal();
    }
    PassRefPtr<Direct2DRenderTarget> SetRenderTarget(PassRefPtr<Direct2DRenderTarget> _renderTarget)override
    {
        RefPtr<TTarget> oldRenderTarget = renderTarget;
        renderTarget = dynamic_cast<Direct2DRenderTarget*>(_renderTarget.get());
        RenderTargetChangedInternal(oldRenderTarget, renderTarget);
        for (RefPtr<IGraphicsElement>& child : element->GetChildren())
        {
            child->GetRenderer()->SetRenderTarget(_renderTarget.get());
        }
        return oldRenderTarget;
    }
    CSize GetMinSize()override
    {
        return minSize;
    }
    void Render(CRect bounds)override
    {
        if (element->flags.children_visible)
        {
            for (RefPtr<IGraphicsElement>& child : element->GetChildren())
            {
                child->GetRenderer()->Render(child->GetRenderRect());
            }
        }
    }
    virtual void InitializeInternal() = 0;
    virtual void FinalizeInternal() = 0;
    virtual void RenderTargetChangedInternal(PassRefPtr<Direct2DRenderTarget> oldRenderTarget, PassRefPtr<Direct2DRenderTarget> newRenderTarget) = 0;

protected:
    RawPtr<IGraphicsRendererFactory>	factory;
    RawPtr<TElement>					element;
    RawPtr<TTarget>						renderTarget;
    CSize								minSize;
};

template <class TElement, class TRenderer, class TBrush, class TBrushProperty>
class GraphicsBrushRenderer : public GraphicsRenderer<TElement, TRenderer, Direct2DRenderTarget>
{
protected:
    void InitializeInternal()override
    {

    }
    void FinalizeInternal()override
    {
        DestroyBrush(renderTarget);
    }
    void RenderTargetChangedInternal(PassRefPtr<Direct2DRenderTarget> oldRenderTarget, PassRefPtr<Direct2DRenderTarget> newRenderTarget)override
    {
        DestroyBrush(oldRenderTarget);
        CreateBrush(newRenderTarget);
    }
    virtual void CreateBrush(PassRefPtr<Direct2DRenderTarget> renderTarget) = 0;
    virtual void DestroyBrush(PassRefPtr<Direct2DRenderTarget> renderTarget) = 0;

    TBrushProperty			oldColor;
    CComPtr<TBrush>			brush;
};

template <class TElement, class TRenderer, class TBrush, class TBrushProperty>
class GraphicsSolidBrushRenderer : public GraphicsBrushRenderer<TElement, TRenderer, TBrush, TBrushProperty>
{
public:
    void OnElementStateChanged()override
    {
        if (renderTarget)
        {
            TBrushProperty color = element->GetColor();
            if (oldColor != color)
            {
                DestroyBrush(renderTarget);
                CreateBrush(renderTarget);
            }
        }
    }

protected:
    void CreateBrush(PassRefPtr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget)
        {
            oldColor = element->GetColor();
            brush = _renderTarget->CreateDirect2DBrush(oldColor);
        }
    }
    void DestroyBrush(PassRefPtr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget && brush)
        {
            _renderTarget->DestroyDirect2DBrush(oldColor);
            brush = nullptr;
        }
    }
};

template <class TElement, class TRenderer, class TBrush, class TBrushProperty>
class GraphicsGradientBrushRenderer : public GraphicsBrushRenderer<TElement, TRenderer, TBrush, TBrushProperty>
{
public:
    void OnElementStateChanged()override
    {
        if (renderTarget)
        {
            TBrushProperty color = TBrushProperty(element->GetColor1(), element->GetColor2());
            if (oldColor != color)
            {
                DestroyBrush(renderTarget);
                CreateBrush(renderTarget);
            }
        }
    }

protected:
    void CreateBrush(PassRefPtr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget)
        {
            oldColor = std::pair<CColor, CColor>(element->GetColor1(), element->GetColor2());
            brush = _renderTarget->CreateDirect2DLinearBrush(oldColor.first, oldColor.second);
        }
    }
    void DestroyBrush(PassRefPtr<Direct2DRenderTarget> _renderTarget)override
    {
        if (_renderTarget && brush)
        {
            _renderTarget->DestroyDirect2DLinearBrush(oldColor.first, oldColor.second);
            brush = nullptr;
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
    void RenderTargetChangedInternal(PassRefPtr<Direct2DRenderTarget> oldRenderTarget, PassRefPtr<Direct2DRenderTarget> newRenderTarget)override;
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

protected:
    CColor color;
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
    Direction direction;
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

    bool GetWrapLine();
    void SetWrapLine(bool value);
    bool GetMultiline();
    void SetMultiline(bool value);
    bool GetWrapLineHeightCalculation();
    void SetWrapLineHeightCalculation(bool value);

protected:
    CColor color;
    Font fontProperties;
    CString text;
    Alignment hAlignment;
    Alignment vAlignment;
    bool wrapLine;
    bool ellipse;
    bool multiline;
    bool wrapLineHeightCalculation;
};

class SolidLabelElementRenderer : public GraphicsRenderer<SolidLabelElement, SolidLabelElementRenderer, Direct2DRenderTarget>
{
public:
    SolidLabelElementRenderer();

    void Render(CRect bounds)override;
    void OnElementStateChanged()override;

protected:
    void CreateBrush(PassRefPtr<Direct2DRenderTarget> _renderTarget);
    void DestroyBrush(PassRefPtr<Direct2DRenderTarget> _renderTarget);
    void CreateTextFormat(PassRefPtr<Direct2DRenderTarget> _renderTarget);
    void DestroyTextFormat(PassRefPtr<Direct2DRenderTarget> _renderTarget);
    void CreateTextLayout();
    void DestroyTextLayout();
    void UpdateMinSize();

    void InitializeInternal()override;
    void FinalizeInternal()override;
    void RenderTargetChangedInternal(PassRefPtr<Direct2DRenderTarget> oldRenderTarget, PassRefPtr<Direct2DRenderTarget> newRenderTarget)override;

    CColor oldColor;
    Font oldFont;
    CString oldText;
    CComPtr<ID2D1SolidColorBrush> brush;
    RefPtr<D2DTextFormatPackage> textFormat;
    CComPtr<IDWriteTextLayout> textLayout;
    cint oldMaxWidth;
};
#pragma endregion SolidLabel

#pragma region Image
template <class TElement, class TRenderer>
class GraphicsImageRenderer : public GraphicsRenderer<TElement, TRenderer, Direct2DRenderTarget>
{
protected:
    void InitializeInternal()override
    {

    }
    void FinalizeInternal()override
    {
        DestroyImage(renderTarget);
    }
    void RenderTargetChangedInternal(PassRefPtr<Direct2DRenderTarget> oldRenderTarget, PassRefPtr<Direct2DRenderTarget> newRenderTarget)override
    {
        DestroyImage(oldRenderTarget);
        CreateImage(newRenderTarget);
    }
    virtual void CreateImage(PassRefPtr<Direct2DRenderTarget> renderTarget) = 0;
    virtual void DestroyImage(PassRefPtr<Direct2DRenderTarget> renderTarget)
    {
        bitmap = nullptr;
    }
    void OnElementStateChanged()override
    {
        DestroyImage(renderTarget);
        CreateImage(renderTarget);
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
    void CreateImage(PassRefPtr<Direct2DRenderTarget> renderTarget)override;
public:
    void Render(CRect bounds)override;
};

#pragma endregion Image

#endif