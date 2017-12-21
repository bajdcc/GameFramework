#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"
#include "Geometries2D.h"
#include "lua_ext/ext.h"

#define N 16
#define TRACE_POINT 1
#define TRACE_N 256

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

#ifdef TRACE_POINT
static void setpixel(int x, int y)
{
    const auto pixel = &bag.g_buf[(y * bag.g_width + x) * 4];
    pixel[0] = 0;
    pixel[1] = 0;
    pixel[2] = 0;
    pixel[3] = 255;
}

// 画直线
// Refer: https://zhuanlan.zhihu.com/p/30553006
// Modified from https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
static void bresenham(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (setpixel(x0, y0), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}
#endif

static void DrawScene2(int part)
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
    {
        *bag.g_painted = true;
#ifdef TRACE_POINT
        const auto _mouse_x = int(g_ui_map["CG-2-MOUSE-X"]);
        const auto _mouse_y = int(g_ui_map["CG-2-MOUSE-Y"]);
        const auto mouse_x = 1.0f * _mouse_x / m;
        const auto mouse_y = 1.0f * _mouse_y / m;
        if (0 <= mouse_x && 0 <= mouse_y && mouse_x < width && mouse_y < height)
        {
            for (auto i = 0; i < TRACE_N; i++) {
                const auto a = PI2 * (i + float(rand()) / RAND_MAX) / TRACE_N;
                const auto r = root->sample(vector2(mouse_x, mouse_y), vector2(cosf(a), sinf(a)));
                if (r.body)
                {
                    if (i % 2 == 0)
                        bresenham(_mouse_x, _mouse_y, int(r.min_pt.position.x * m), int(r.min_pt.position.y * m));
                    else
                        bresenham(_mouse_x, _mouse_y, int(r.max_pt.position.x * m), int(r.max_pt.position.y * m));
                }
            }
        }
#endif
    }
    bag.mtx.unlock();
}

void PhysicsEngine::Render2DScene2(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    scene = DrawScene2;

    // --------------------------------------
    // 场景设置
    if (!buf.get())
    {
		root = Geo2DFactory:: or (
			Geo2DFactory:: and (
				Geo2DFactory::new_circle(1.3f, 0.5f, 0.4f, color(2.0f, 1.0f, 1.0f)),
				Geo2DFactory::new_circle(1.7f, 0.5f, 0.4f, color(2.0f, 1.0f, 1.0f))),
			Geo2DFactory:: sub (
				Geo2DFactory::new_circle(0.5f, 0.5f, 0.4f, color(1.0f, 1.0f, 2.0f)),
				Geo2DFactory::new_circle(0.9f, 0.5f, 0.4f, color(1.0f, 1.0f, 2.0f))));
    }

    // --------------------------------------

    RenderSceneIntern(rt, bounds);

    if (painted)
        root.reset();
}
