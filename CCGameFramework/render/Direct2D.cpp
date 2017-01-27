#include "stdafx.h"
#include "Direct2D.h"
#include "Direct2DRender.h"


D2D1::ColorF GetD2DColor(CColor color)
{
    return D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

static void InitRenderer()
{
    SolidBackgroundElementRenderer::Register();
    GradientBackgroundElementRenderer::Register();
    SolidLabelElementRenderer::Register();
}

Direct2D::Direct2D()
{
    HRESULT hr;
    ID2D1Factory* d2d1;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d1);
    if (SUCCEEDED(hr))
        D2D1Factory.Attach(d2d1);
    IDWriteFactory* dwrite;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite));
    if (SUCCEEDED(hr))
        DWriteFactory.Attach(dwrite);
}

CComPtr<ID2D1Factory> Direct2D::GetDirect2DFactory()
{
    return D2D1Factory;
}

CComPtr<IDWriteFactory> Direct2D::GetDirectWriteFactory()
{
    return DWriteFactory;
}

bool Direct2D::RegisterFactories(PassRefPtr<IGraphicsElementFactory> elementFactory, PassRefPtr<IGraphicsRendererFactory> rendererFactory)
{
    RefPtr<IGraphicsElementFactory> _elementFactory = elementFactory;
    RefPtr<IGraphicsRendererFactory> _rendererFactory = rendererFactory;
    if (_elementFactory && _rendererFactory)
    {
        if (RegisterElementFactory(_elementFactory))
        {
            if (RegisterRendererFactory(_elementFactory->GetElementTypeName(), _rendererFactory))
            {
                return true;
            }
        }
    }
    return false;
}

bool Direct2D::RegisterElementFactory(PassRefPtr<IGraphicsElementFactory> factory)
{
    return elementFactories.insert(std::make_pair(factory->GetElementTypeName(), factory)).second;
}

bool Direct2D::RegisterRendererFactory(const CString& elementTypeName, PassRefPtr<IGraphicsRendererFactory> factory)
{
    return rendererFactories.insert(std::make_pair(elementTypeName, factory)).second;
}

Direct2D& Direct2D::Singleton()
{
    static Direct2D d2d;
    return d2d;
}

PassRefPtr<IGraphicsElementFactory> Direct2D::GetElementFactory(const CString& elementTypeName)
{
    if (elementFactories.empty())
        InitRenderer();
    auto found = elementFactories.find(elementTypeName);
    return found == elementFactories.end() ? nullptr : found->second;
}

PassRefPtr<IGraphicsRendererFactory> Direct2D::GetRendererFactory(const CString& elementTypeName)
{
    if (rendererFactories.empty())
        InitRenderer();
    auto found = rendererFactories.find(elementTypeName);
    return found == rendererFactories.end() ? nullptr : found->second;
}