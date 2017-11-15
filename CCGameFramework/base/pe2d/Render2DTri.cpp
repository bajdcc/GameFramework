#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

#define SCAN_N 10

extern float PI2;

extern float circleSDF(float x, float y, float cx, float cy, float r);
extern float boxSDF(float x, float y, float cx, float cy, float theta, float sx, float sy);
extern float planeSDF(float x, float y, float px, float py, float nx, float ny);

struct Result
{
    float sd;           // 带符号距离（signed distance）
    color emissive;     // 自发光强度（emissive）
    color reflectivity; // 反射系数
};

extern Result(*g_scene)(float x, float y);

extern Result unionOp(Result a, Result b);
extern Result intersectOp(Result a, Result b);
extern Result subtractOp(Result a, Result b);

static Result scene_tri(float x, float y)
{
    Result a = { circleSDF(x, y, 1.1f, 0.2f, 0.1f), color(Gdiplus::Color::Red), color::make_color(0.0f) };
    Result b = { circleSDF(x, y, 0.8f, 0.5f, 0.1f), color(Gdiplus::Color::Blue), color::make_color(0.0f) };
    Result c = { circleSDF(x, y, 1.4f, 0.5f, 0.1f), color(Gdiplus::Color::Green), color::make_color(0.0f) };
    Result d = { circleSDF(x, y, 1.1f, 0.8f, 0.1f), color(Gdiplus::Color::White), color::make_color(0.0f) };
    Result h = { planeSDF(x, y, 0.0f, 0.0f, 0.0f, -1.0f), color(0.0f, 0.0f, 0.0f), color(0.99f, 0.99f, 0.99f) };
    Result i = { circleSDF(x, y, 1.1f, 0.5f, 0.45f), color(0.0f, 0.0f, 0.0f), color(0.99f, 0.99f, 0.99f) };

    return unionOp(unionOp(unionOp(a, b), unionOp(c, d)), subtractOp(h, i));
}

extern void gradient(float x, float y, float* nx, float* ny);
extern void reflect(float ix, float iy, float nx, float ny, float* rx, float* ry);
extern color trace(float ox, float oy, float dx, float dy, int depth);
extern color sample(float x, float y);

extern void DrawSceneGlobal(int part);

void PhysicsEngine::Render2DTri(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    g_scene = ::scene_tri;
    scene = DrawSceneGlobal;;
    RenderSceneIntern(rt, bounds);
}
