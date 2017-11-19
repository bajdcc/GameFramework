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
};

extern Result unionOp(Result a, Result b);
extern Result intersectOp(Result a, Result b);
extern Result subtractOp(Result a, Result b);

static Result scene_ref(float x, float y)
{
    Result b = { boxSDF(x, y, 0.9f, 0.5f, 0.0f, 0.15f, 0.08f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result c = { circleSDF(x, y, 0.7f, -0.3f, 0.05f), color(20.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f), 0.0f };
    Result c2 = { circleSDF(x, y, 1.5f, -0.3f, 0.05f), color(0.0f, 0.0f, 20.0f), color(0.0f, 0.0f, 0.0f), 0.0f };
    Result d = { circleSDF(x, y, 1.3f, 0.2f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result e = { circleSDF(x, y, 1.3f, 0.8f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result f = { boxSDF(x, y, 1.8f, 0.5f, 0.0f, 0.2f, 0.1f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result g = { circleSDF(x, y, 1.8f, 0.12f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result h = { circleSDF(x, y, 1.8f, 0.87f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result i = { circleSDF(x, y, 0.4f, 0.5f, 0.2f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    Result j = { planeSDF(x, y, 0.4f, 0.5f, 0.0f, -1.0f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f };
    return unionOp(unionOp(c2, subtractOp(f, unionOp(g, h))), unionOp(unionOp(c, intersectOp(d, e)), unionOp(b, intersectOp(i, j))));
}

extern Result (*g_scene)(float x, float y);

extern void gradient(float x, float y, float* nx, float* ny);
extern void reflect(float ix, float iy, float nx, float ny, float* rx, float* ry);

/**
 * \brief 计算折射向量
 * \param ix 入射光线X坐标
 * \param iy 入射光线Y坐标
 * \param nx 法向量X坐标
 * \param ny 法向量Y坐标
 * \param eta 折射率
 * \param rx 折射光线X坐标
 * \param ry 折射光线Y坐标
 * \return 是否产生折射
 */
int refract(float ix, float iy, float nx, float ny, float eta, float* rx, float* ry) {
    auto idotn = ix * nx + iy * ny;
    auto k = 1.0f - eta * eta * (1.0f - idotn * idotn);
    if (k < 0.0f)
        return 0; // Total internal reflection
    auto a = eta * idotn + sqrtf(k);
    *rx = eta * ix - a * nx;
    *ry = eta * iy - a * ny;
    return 1;
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
static color trace_ref(float ox, float oy, float dx, float dy, int depth) {
    static color black;
    auto t = 1e-3f;
    auto sign = g_scene(ox, oy).sd > 0.0f;                      // 判断光线自物体内部还是外部，正为外，负为内
    for (auto i = 0; i < MAX_STEP && t < MAX_DISTANCE; i++) {
        const auto x = ox + dx * t, y = oy + dy * t;
        auto r = g_scene(x, y);
        if (sign ? (r.sd < EPSILON) : (r.sd > -EPSILON)) {          // 如果线与图形有交点
            auto sum = r.emissive;                              // 用于累计采样，此处值为除去反射的正常接收光线
            if (depth < MAX_DEPTH &&
                (r.reflectivity.Valid() || r.eta > 0.0f)) {     // 在反射深度内，且允许反射
                float nx, ny, rx, ry;
                auto refl = r.reflectivity;
                gradient(x, y, &nx, &ny);                       // 求该交点处法向量
                nx = sign ? nx : -nx, ny = sign ? ny : -ny;     // 当光线从形状内往外发射时，要反转法线方向
                if (r.eta > 0.0f) {
                    if (refract(dx, dy, nx, ny, !sign ? r.eta : 1.0f / r.eta, &rx, &ry))
                        sum.Add((refl.Negative(1.0f)) * trace_ref(x - nx * BIAS, y - ny * BIAS, rx, ry, depth + 1));
                    else
                        refl.Set(1.0f);                         // 不折射则为全内反射
                }
                if (refl.Valid()) {
                    reflect(dx, dy, nx, ny, &rx, &ry);          // 求出反射光线
                    // BIAS为偏差
                    // 加上反射光线 = 反射光线追踪采样值 * 反射系数
                    sum.Add(refl * trace_ref(x + nx * BIAS, y + ny * BIAS, rx, ry, depth + 1));
                }
            }
            return sum;
        }
        sign ? t += r.sd : t -= r.sd;
    }
    return black;
}

/**
* \brief 采样(x,y)位置处的光线
* \param x X坐标
* \param y Y坐标
* \return 采样
*/
static color sample(float x, float y) {
    color sum;
    for (auto i = 0; i < N; i++) {
        // const auto a = PI2 * rand() / RAND_MAX;                  // 均匀采样
        // const auto a = PI2 * i / N;                              // 分层采样
        const auto a = PI2 * (i + float(rand()) / RAND_MAX) / N;    // 抖动采样
        sum.Add(trace_ref(x, y, cosf(a), sinf(a), 0)); // 追踪 (x,y) 从 随机方向(cos(a),sin(a)) 收集到的光
    }
    return sum * (1.0f / N);
}

extern DrawSceneBag bag;

static void DrawSceneRefraction(int part)
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

void PhysicsEngine::Render2DRefraction(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    g_scene = ::scene_ref;
    scene = DrawSceneRefraction;
    RenderSceneIntern(rt, bounds);
}
