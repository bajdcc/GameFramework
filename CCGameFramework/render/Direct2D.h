#ifndef RENDER_D2D_H
#define RENDER_D2D_H
#include <ui/gdi/Gdi.h>

D2D1::ColorF GetD2DColor(CColor color);

class IGraphicsElement;
class IGraphicsElementFactory;
class IGraphicsRenderer;
class IGraphicsRendererFactory;

class Direct2D
{
    Direct2D();
    ~Direct2D();

    static void ReportLiveObjects();

    CComPtr<ID2D1Factory> D2D1Factory;
    CComPtr<IDWriteFactory> DWriteFactory;
    CComPtr<IWICImagingFactory> WICImagingFactory;

    typedef std::map<CString, std::shared_ptr<IGraphicsElementFactory>> elementFactoryMap;
    typedef std::map<CString, std::shared_ptr<IGraphicsRendererFactory>> rendererFactoryMap;

    elementFactoryMap elementFactories;
    rendererFactoryMap rendererFactories;

    bool RegisterElementFactory(std::shared_ptr<IGraphicsElementFactory> factory);
    bool RegisterRendererFactory(const CString& elementTypeName, std::shared_ptr<IGraphicsRendererFactory> factory);

public:
    CComPtr<ID2D1Factory> GetDirect2DFactory();
    CComPtr<IDWriteFactory> GetDirectWriteFactory();
    CComPtr<IWICImagingFactory> GetWICImagingFactory();
    bool RegisterFactories(std::shared_ptr<IGraphicsElementFactory> elementFactory, std::shared_ptr<IGraphicsRendererFactory> rendererFactory);
    std::shared_ptr<IGraphicsElementFactory> GetElementFactory(const CString& elementTypeName);
    std::shared_ptr<IGraphicsRendererFactory> GetRendererFactory(const CString& elementTypeName);

    static Direct2D& Singleton();
};

#endif