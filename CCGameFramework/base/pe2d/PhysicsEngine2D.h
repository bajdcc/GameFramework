#ifndef PE2D_PE2D_H
#define PE2D_PE2D_H
#include "render/Direct2DAllocator.h"
#include <memory>
#include "Geometries.h"


class PhysicsEngine
{
public:
    void RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget);

    void SetType(cint value);

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
    void Render2DShadow(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render2DSolid(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

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

private:
    CComPtr<ID2D1SolidColorBrush> bg;
    CColor bgColor;
    cint type{ 0 };
    std::weak_ptr<Direct2DRenderTarget> d2drt;
    bool painted{ false };
    CComPtr<ID2D1Bitmap> bitmap;
    CComPtr<ID2D1Bitmap> bitmap2;
};

#endif