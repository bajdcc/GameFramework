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
    CComPtr<ID2D1Device> GetDirect2DDevice();
    CComPtr<ID2D1DeviceContext> GetDirect2DDeviceContext();
    CComPtr<IDXGIDevice1> GetDXGIDevice();
    CComPtr<ID3D11Device> GetDirect3DDevice();
    CComPtr<ID3D11DeviceContext> GetDirect3DDeviceContext();
    CComPtr<IDXGISwapChain> GetDXGISwapChain();
    bool RegisterFactories(std::shared_ptr<IGraphicsElementFactory> elementFactory, std::shared_ptr<IGraphicsRendererFactory> rendererFactory);
    std::shared_ptr<IGraphicsElementFactory> GetElementFactory(const CString& elementTypeName);
    std::shared_ptr<IGraphicsRendererFactory> GetRendererFactory(const CString& elementTypeName);

    static Direct2D& Singleton();
    void Init();

private:
    CComPtr<ID2D1Factory> D2D1Factory;
    CComPtr<IDWriteFactory> DWriteFactory;
    CComPtr<IWICImagingFactory> WICImagingFactory;
    CComPtr<ID2D1Device> D2D1Device;
    CComPtr<ID2D1DeviceContext> D2D1DeviceContext;
    CComPtr<IDXGIDevice1> DXGIDevice;
    CComPtr<ID3D11Device> D3D11Device;
    CComPtr<ID3D11DeviceContext> D3D11DeviceContext;
    CComPtr<IDXGISwapChain> DXGISwapChain;
    CComPtr<ID2D1Bitmap1> D2D1RenderTarget;
    D3D_FEATURE_LEVEL FeatureLevel;
};

#endif