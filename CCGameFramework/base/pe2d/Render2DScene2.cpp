#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"
#include "Geometries2D.h"

#define N 256

extern float PI2;
extern DrawSceneBag bag;

std::shared_ptr<Geo2DObject> root;

static color trace2(float ox, float oy, float dx, float dy) {
    const auto r = root->sample(vector2(ox, oy), vector2(dx, dy));
    if (r.body)
    {
        return r.body->L;
    }
    static color black;
    return black;
}

static color sample2(float x, float y) {
    color sum;
    for (auto i = 0; i < N; i++) {
        const auto a = PI2 * (i + float(rand()) / RAND_MAX) / N;
        const auto c = trace2(x, y, cosf(a), sinf(a));
        sum.Add(c);
    }
    return sum * (1.0f / N);
}

static void DrawScene2(int part)
{
    auto buffer = bag.g_buf;
    auto width = bag.g_width;
    auto height = bag.g_height;
    auto m = min(width, height);
    auto k = sample2(1.3f,0.5f);
    for (auto y = 0; y < height; y++)
    {
        if (y % 4 == part)
        {
            for (auto x = 0; x < width; x++)
            {
                const auto color = sample2(float(x) / m, float(y) / m);
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

void PhysicsEngine::Render2DScene2(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    scene = DrawScene2;

    // --------------------------------------
    // 场景设置
    if (!buf.get())
    {
        root = Geo2DFactory::and(
            Geo2DFactory::new_circle(0.9f, 0.5f, 0.4f, color(1.0f, 1.0f, 1.0f)),
            Geo2DFactory::new_circle(1.3f, 0.5f, 0.4f, color(1.0f, 1.0f, 1.0f)));
    }

    // --------------------------------------

    RenderSceneIntern(rt, bounds);

    if (painted)
        root.reset();
}
