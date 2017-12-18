#ifndef PE2D_PE2D_H
#define PE2D_PE2D_H
#include "render/Direct2DAllocator.h"
#include <memory>
#include "Geometries.h"

class DrawSceneBag
{
public:
    volatile bool* g_painted{ nullptr };
    volatile BYTE* g_buf{ nullptr };
    volatile int g_width{ 0 }, g_height{ 0 };
    volatile int g_cnt{ 0 };
    std::mutex mtx;
};

extern DrawSceneBag bag;

class PhysicsEngine
{
public:
    void RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget);

    int SetType(cint value);

    using scene_t = void(*)(int);

private:
    void RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderSingleBitmap(CComPtr<ID2D1RenderTarget> rt, CRect bounds, void(*callback)(BYTE*, cint, cint));

    void RenderSimpleColor(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderSimpleSphere(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderMaterialSphere(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderReflectSphere(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void RenderDirectionalLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderPointLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderSpotLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void RenderTriLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Render2DLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DSolid(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DReflect(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DTri(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DRefraction(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DFont(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Render2DScene1(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DScene2(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

private:
    static void RenderSimpleIntern(BYTE* buffer, cint width, cint height);
    static void RenderSphereIntern(BYTE* buffer, BYTE* buffer2, cint width, cint height);
    static void RenderMaterialIntern(BYTE* buffer, cint width, cint height);
    static void RenderReflectIntern(BYTE* buffer, cint width, cint height);
    static color RenderReflectRecursive(World& world, const Ray& ray, int maxReflect);

    static void RenderDirectionalLight(BYTE* buffer, cint width, cint height);
    static void RenderPointLight(BYTE* buffer, cint width, cint height);
    static void RenderSpotLight(BYTE* buffer, cint width, cint height);
    static void RenderTriLight(BYTE* buffer, cint width, cint height);
    static void RenderLightIntern(World&, const PerspectiveCamera&, BYTE* buffer, cint width, cint height);

    void RenderSceneIntern(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

private:
    CComPtr<ID2D1SolidColorBrush> bg;
    CColor bgColor;
    cint type{ 0 };
    std::weak_ptr<Direct2DRenderTarget> d2drt;
    bool painted{ true };
    bool locked{ false };
    std::auto_ptr<BYTE> buf;
    WICRect rect;
    CComPtr<ID2D1Bitmap> bitmap;
    CComPtr<ID2D1Bitmap> bitmap2;
    scene_t scene;
};

#endif