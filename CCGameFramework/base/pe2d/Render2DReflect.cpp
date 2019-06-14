#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

#define SCAN_N 10

#define N 64
#define MAX_STEP 64
#define MAX_DISTANCE 5.0f
#define EPSILON 1e-6f
#define BIAS 1e-4f
#define MAX_DEPTH 5

extern float PI2;

extern float circleSDF(float x, float y, float cx, float cy, float r);
extern float boxSDF(float x, float y, float cx, float cy, float theta, float sx, float sy);
extern float planeSDF(float x, float y, float px, float py, float nx, float ny);

struct Result
{
	float sd;           // 带符号距离（signed distance）
	color emissive;     // 自发光强度（emissive）
	color reflectivity; // 反射系数
	float eta;          // 折射率
	color absorption;   // 吸收率
};

extern Result unionOp(Result a, Result b);
extern Result intersectOp(Result a, Result b);
extern Result subtractOp(Result a, Result b);

static Result scene_ref(float x, float y)
{
    Result a = { circleSDF(x, y, 1.1f, 0.3f, 0.15f), color(0.0f, 0.0f, 4.8f), color(0.0f, 0.0f, 0.0f) };
    Result b = { boxSDF(x, y, 0.6f, 0.2f, PI2 / 16.0f, 0.1f, 0.1f), color(4.8f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f) };
    Result c = { boxSDF(x, y, 1.5f, 0.2f, PI2 / 16.0f, 0.1f, 0.1f), color(0.0f, 4.8f, 0.0f), color(0.0f, 0.0f, 0.0f) };
    Result d = { planeSDF(x, y, 0.0f, 0.5f, 0.0f, -1.0f), color(0.0f, 0.0f, 0.0f), color(0.9f, 0.9f, 0.9f) };
    Result e = { circleSDF(x, y, 1.1f, 0.5f, 0.4f), color(0.0f, 0.0f, 0.0f), color(0.9f, 0.9f, 0.9f) };

    return unionOp(unionOp(a, unionOp(b, c)), subtractOp(d, e));
}

Result (*g_scene)(float x, float y);

/**
 * \brief 求梯度
 * \param x X坐标
 * \param y Y坐标
 * \param nx 要求的法向量X坐标
 * \param ny 要求的法向量Y坐标
 */
extern void gradient(float x, float y, float* nx, float* ny) {
    *nx = (g_scene(x + EPSILON, y).sd - g_scene(x - EPSILON, y).sd) * (0.5f / EPSILON);
    *ny = (g_scene(x, y + EPSILON).sd - g_scene(x, y - EPSILON).sd) * (0.5f / EPSILON);
}

/**
 * \brief 求反射向量
 * \param ix 入射光线X坐标
 * \param iy 入射光线Y坐标
 * \param nx 法向量X坐标
 * \param ny 法向量Y坐标
 * \param rx 反射光线X坐标
 * \param ry 反射光线Y坐标
 */
extern void reflect(float ix, float iy, float nx, float ny, float* rx, float* ry) {
    const auto idotn2 = (ix * nx + iy * ny) * 2.0f;
    *rx = ix - idotn2 * nx;
    *ry = iy - idotn2 * ny;
}

/**
 * \brief 递归光线追踪
 * \param ox 起点X坐标
 * \param oy 起点Y坐标
 * \param dx 终点X坐标
 * \param dy 终点T坐标
 * \param depth 层数
 * \return 采样
 */
extern color trace(float ox, float oy, float dx, float dy, int depth) {
    static color black;
    auto t = 0.0f;
    for (auto i = 0; i < MAX_STEP && t < MAX_DISTANCE; i++) {
        const auto x = ox + dx * t, y = oy + dy * t;
        auto r = g_scene(x, y);
        if (r.sd < EPSILON) {                                   // 如果线与图形有交点
            auto sum = r.emissive;                              // 用于累计采样，此处值为除去反射的正常接收光线
            if (depth < MAX_DEPTH && r.reflectivity.Valid()) {  // 在反射深度内，且允许反射
                float nx, ny, rx, ry;
                gradient(x, y, &nx, &ny);                       // 求该交点处法向量
                reflect(dx, dy, nx, ny, &rx, &ry);              // 求出反射光线
                // BIAS为偏差
                // 加上反射光线 = 反射光线追踪采样值 * 反射系数
                sum.Add(r.reflectivity * trace(x + nx * BIAS, y + ny * BIAS, rx, ry, depth + 1));
            }
            return sum;
        }
        t += r.sd;
    }
    return black;
}

/**
* \brief 采样(x,y)位置处的光线
* \param x X坐标
* \param y Y坐标
* \return 采样
*/
extern color sample(float x, float y) {
    color sum;
    for (auto i = 0; i < N; i++) {
        // const auto a = PI2 * rand() / RAND_MAX;                  // 均匀采样
        // const auto a = PI2 * i / N;                              // 分层采样
        const auto a = PI2 * (i + float(rand()) / RAND_MAX) / N;    // 抖动采样
        sum.Add(trace(x, y, cosf(a), sinf(a), 0)); // 追踪 (x,y) 从 随机方向(cos(a),sin(a)) 收集到的光
    }
    return sum * (1.0f / N);
}

extern DrawSceneBag bag;

extern void DrawSceneGlobal(int part)
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
                const auto color = sample(float(x) / m, float(y) / m);
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

void PhysicsEngine::RenderSceneIntern(CComPtr<ID2D1RenderTarget> rt, CRect bounds, BYTE* buffer)
{
    auto d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
    if (painted || buf.get())
    {
        if (buf.get())
        {
            bitmap->CopyFromMemory(&d2dRect, buf.get(), rect.Width * 4);
            if (painted)
                buf.release();
        }
        // 画渲染好的位图
        rt->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top,
            (FLOAT)bounds.left + fminf((FLOAT)bounds.Width(), bitmap->GetSize().width),
                (FLOAT)bounds.top + fminf((FLOAT)bounds.Height(), bitmap->GetSize().height)),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
        return;
    }

    auto _rt = d2drt.lock();
    auto _w = bounds.Width(), _h = bounds.Height();
    auto wic = _rt->CreateBitmap(_w, _h);
    rect.X = 0;
    rect.Y = 0;
    rect.Width = _w;
    rect.Height = _h;
    if (!buf.get())
    {
        buf.reset(new BYTE[rect.Width * rect.Height * 4]);
        if (buffer)
            CopyMemory(buf.get(), buffer, rect.Width * rect.Height * 4);
    }

    bitmap = _rt->GetBitmapFromWIC(wic);
    bitmap->CopyFromMemory(&d2dRect, buf.get(), rect.Width * 4);
    rt->DrawBitmap(
        bitmap,
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );

    bag.g_painted = &painted;
    bag.g_buf = buf.get();
    bag.g_width = _w;
    bag.g_height = _h;
    bag.g_cnt = 0;
    // ----------------------------------------------------
    // 位图渲染开始
    std::thread th0(scene, 0);
    th0.detach();
    std::thread th1(scene, 1);
    th1.detach();
    std::thread th2(scene, 2);
    th2.detach();
    std::thread th3(scene, 3);
    th3.detach();
    // ----------------------------------------------------
}

void PhysicsEngine::Render2DReflect(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    g_scene = ::scene_ref;
    scene = DrawSceneGlobal;
    RenderSceneIntern(rt, bounds);
}
