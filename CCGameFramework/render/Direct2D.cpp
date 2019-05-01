#include "stdafx.h"
#include "Direct2D.h"
#include "Direct2DRender.h"
#ifdef _DEBUG
#include <DXGIDebug.h>
#endif

D2D1::ColorF GetD2DColor(CColor color)
{
    return D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

static void InitRenderer()
{
    EmptyElementRenderer::Register();
    SolidBackgroundElementRenderer::Register();
    GradientBackgroundElementRenderer::Register();
    SolidLabelElementRenderer::Register();
    RoundBorderElementRenderer::Register();
    QRImageElementRenderer::Register();
    Base64ImageElementRenderer::Register();
    WireworldAutomatonImageElementRenderer::Register();
    PhysicsEngine2DElementRenderer::Register();
    X86WindowElementRenderer::Register();
    EditElementRenderer::Register();
    Clib2DElementRenderer::Register();
    Parser2DElementRenderer::Register();
}

Direct2D::Direct2D()
{
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr;
    ID2D1Factory* d2d1;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d1);
    if (SUCCEEDED(hr))
        D2D1Factory.Attach(d2d1);
    else
        ATLASSERT(!"D2D1CreateFactory failed");
    IDWriteFactory* dwrite;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite));
    if (SUCCEEDED(hr))
        DWriteFactory.Attach(dwrite);
    else
        ATLASSERT(!"DWriteCreateFactory failed");
    hr = WICImagingFactory.CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
        ATLASSERT(!"WICImagingFactory failed");
}

Direct2D::~Direct2D()
{
    WICImagingFactory = nullptr;
    DWriteFactory = nullptr;
    D2D1Factory = nullptr;
    ReportLiveObjects();
    elementFactories.clear();
    rendererFactories.clear();
    ::CoUninitialize();
}

void Direct2D::ReportLiveObjects()
{
#ifdef _DEBUG
    HRESULT hr;
    HMODULE hDxgiDebug = GetModuleHandle(_T("Dxgidebug.dll"));
    if (!hDxgiDebug) return;
    typedef HRESULT(WINAPI *pfnDXGIGetDebugInterface)(REFIID riid, void **ppDebug);
    pfnDXGIGetDebugInterface _DXGIGetDebugInterface = (pfnDXGIGetDebugInterface)GetProcAddress(hDxgiDebug, "DXGIGetDebugInterface");
    if (!_DXGIGetDebugInterface) return;
    CComPtr<IDXGIDebug1> dxgiDebug;
    hr = _DXGIGetDebugInterface(__uuidof(IDXGIDebug1), (void**)&dxgiDebug);
    if (FAILED(hr))	return;
    hr = dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    if (FAILED(hr))	return;
#endif
}

CComPtr<ID2D1Factory> Direct2D::GetDirect2DFactory()
{
    return D2D1Factory;
}

CComPtr<IDWriteFactory> Direct2D::GetDirectWriteFactory()
{
    return DWriteFactory;
}

CComPtr<IWICImagingFactory> Direct2D::GetWICImagingFactory()
{
    return WICImagingFactory;
}

bool Direct2D::RegisterFactories(std::shared_ptr<IGraphicsElementFactory> elementFactory, std::shared_ptr<IGraphicsRendererFactory> rendererFactory)
{
    std::shared_ptr<IGraphicsElementFactory> _elementFactory = elementFactory;
    std::shared_ptr<IGraphicsRendererFactory> _rendererFactory = rendererFactory;
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

bool Direct2D::RegisterElementFactory(std::shared_ptr<IGraphicsElementFactory> factory)
{
    return elementFactories.insert(std::make_pair(factory->GetElementTypeName(), factory)).second;
}

bool Direct2D::RegisterRendererFactory(const CString& elementTypeName, std::shared_ptr<IGraphicsRendererFactory> factory)
{
    return rendererFactories.insert(std::make_pair(elementTypeName, factory)).second;
}

Direct2D& Direct2D::Singleton()
{
    static Direct2D d2d;
    return d2d;
}

std::shared_ptr<IGraphicsElementFactory> Direct2D::GetElementFactory(const CString& elementTypeName)
{
    if (elementFactories.empty())
        InitRenderer();
    auto found = elementFactories.find(elementTypeName);
    return found == elementFactories.end() ? nullptr : found->second;
}

std::shared_ptr<IGraphicsRendererFactory> Direct2D::GetRendererFactory(const CString& elementTypeName)
{
    if (rendererFactories.empty())
        InitRenderer();
    auto found = rendererFactories.find(elementTypeName);
    return found == rendererFactories.end() ? nullptr : found->second;
}