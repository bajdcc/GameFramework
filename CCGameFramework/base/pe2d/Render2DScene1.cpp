#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

#define N 64
#define MAX_STEP 64
#define MAX_DISTANCE 5.0f
#define EPSILON 1e-6f
#define BIAS 1e-4f
#define MAX_DEPTH 5
#define SCAN_N 10

extern float PI2;
extern DrawSceneBag bag;

static color trace1(float ox, float oy, float dx, float dy) {
    static color black;
    
    return black;
}

static color sample1(float x, float y) {
    color sum;
    for (auto i = 0; i < N; i++) {
        const auto a = PI2 * (i + float(rand()) / RAND_MAX) / N;
        sum.Add(trace1(x, y, cosf(a), sinf(a)));
    }
    return sum * (1.0f / N);
}

static void DrawScene1(int part)
{
    auto buffer = bag.g_buf;
    auto width = bag.g_width;
    auto height = bag.g_height;
    auto m = min(width, height);
    for (auto y = 0; y < height; y++)
    {
        if (y % 4 == part)
        {
            for (auto x = 0; x < width; x++)
            {
                const auto color = sample1(float(x) / m, float(y) / m);
                buffer[0] = BYTE(fminf(color.b, 1.0f) * 255.0f);
                buffer[1] = BYTE(fminf(color.g, 1.0f) * 255.0f);
                buffer[2] = BYTE(fminf(color.r, 1.0f) * 255.0f);
                buffer[3] = 255;
                buffer += 4;
            }
        }
        else
            buffer += 4 * width;
    }
    bag.mtx.lock();
    bag.g_cnt++;
    if (bag.g_cnt == 4)
        *bag.g_painted = true;
    bag.mtx.unlock();
}

void PhysicsEngine::Render2DScene1(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    scene = DrawScene1;
    RenderSceneIntern(rt, bounds);
}
