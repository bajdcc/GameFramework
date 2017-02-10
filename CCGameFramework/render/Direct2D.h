#ifndef RENDER_D2D_H
#define RENDER_D2D_H
#include <ui/gdi/Gdi.h>
#include <WTF/RefPtr.h>

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

    typedef std::map<CString, RefPtr<IGraphicsElementFactory>> elementFactoryMap;
    typedef std::map<CString, RefPtr<IGraphicsRendererFactory>> rendererFactoryMap;

    elementFactoryMap elementFactories;
    rendererFactoryMap rendererFactories;

    bool RegisterElementFactory(PassRefPtr<IGraphicsElementFactory> factory);
    bool RegisterRendererFactory(const CString& elementTypeName, PassRefPtr<IGraphicsRendererFactory> factory);

public:
    CComPtr<ID2D1Factory> GetDirect2DFactory();
    CComPtr<IDWriteFactory> GetDirectWriteFactory();
    bool RegisterFactories(PassRefPtr<IGraphicsElementFactory> elementFactory, PassRefPtr<IGraphicsRendererFactory> rendererFactory);
    PassRefPtr<IGraphicsElementFactory> GetElementFactory(const CString& elementTypeName);
    PassRefPtr<IGraphicsRendererFactory> GetRendererFactory(const CString& elementTypeName);

    static Direct2D& Singleton();
};

#endif