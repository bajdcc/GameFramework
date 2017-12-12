#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

#define SCAN_N 10

#define N 256
#define MAX_STEP 64
#define MAX_DISTANCE 5.0f
#define EPSILON 1e-6f
#define BIAS 1e-4f
#define MAX_DEPTH 5

extern float PI2;

extern float circleSDF(float x, float y, float cx, float cy, float r);
extern float boxSDF(float x, float y, float cx, float cy, float theta, float sx, float sy);
extern float planeSDF(float x, float y, float px, float py, float nx, float ny);

/**
 * \brief 正多边形的SDF
 * \param x 目标X坐标
 * \param y 目标Y坐标
 * \param cx 中心点X坐标
 * \param cy 中心点Y坐标
 * \param r 半径
 * \param n 边的数量
 * \return SD
 */
extern float ngonSDF(float x, float y, float cx, float cy, float r, float n) {
	const auto ux = x - cx, uy = y - cy, a = PI2 / n;
	const auto t = fmodf(atan2f(uy, ux) + PI2, a), s = sqrtf(ux * ux + uy * uy);
	return planeSDF(s * cosf(t), s * sinf(t), r, 0.0f, cosf(a * 0.5f), sinf(a * 0.5f));
}

/**
 * \brief 比尔-朗伯定律
 * \param a 物质的吸光系数
 * \param d 光程距离
 * \return 透射率
 */
static color beerLambert(color a, float d) {
	return color(expf(-a.r * d), expf(-a.g * d), expf(-a.b * d));
}

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
    Result b = { ngonSDF(x, y, 0.9f, 0.5f, 0.15f, 5.0f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(4.0f, 1.0f, 4.0f) };
    Result c = { circleSDF(x, y, 0.7f, -0.3f, 0.05f), color(20.0f, 5.0f, 10.0f), color(0.0f, 0.0f, 0.0f), 0.0f, color(0.0f, 0.0f, 0.0f) };
    Result c2 = { circleSDF(x, y, 1.5f, -0.3f, 0.05f), color(10.0f, 5.0f, 20.0f), color(0.0f, 0.0f, 0.0f), 0.0f, color(0.0f, 0.0f, 0.0f) };
    Result d = { circleSDF(x, y, 1.3f, 0.2f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(3.0f, 0.0f, 3.0f) };
    Result e = { circleSDF(x, y, 1.3f, 0.8f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(3.0f, 0.0f, 3.0f) };
    Result f = { boxSDF(x, y, 1.8f, 0.5f, 0.0f, 0.2f, 0.1f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(1.0f, 1.0f, 1.0f) };
    Result g = { circleSDF(x, y, 1.8f, 0.12f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(1.0f, 1.0f, 1.0f) };
    Result h = { circleSDF(x, y, 1.8f, 0.87f, 0.35f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(1.0f, 1.0f, 1.0f) };
    Result i = { circleSDF(x, y, 0.4f, 0.5f, 0.2f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(2.0f, 1.0f, 1.0f) };
    Result j = { planeSDF(x, y, 0.4f, 0.5f, 0.0f, -1.0f), color(0.0f, 0.0f, 0.0f), color(0.2f, 0.2f, 0.2f), 1.5f, color(2.0f, 1.0f, 1.0f) };
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
extern int refract(float ix, float iy, float nx, float ny, float eta, float* rx, float* ry) {
    const auto idotn = ix * nx + iy * ny;
    const auto k = 1.0f - eta * eta * (1.0f - idotn * idotn);
    if (k < 0.0f)
        return 0; // Total internal reflection
    const auto a = eta * idotn + sqrtf(k);
    *rx = eta * ix - a * nx;
    *ry = eta * iy - a * ny;
    return 1;
}

/**
 * \brief 菲涅耳方程（描述了光线经过两个介质的界面时，反射和透射的光强比重）
 * \param cosi 入射角余弦
 * \param cost 折射角余弦
 * \param etai 外界折射率
 * \param etat 介质折射率
 * \return 比重
 */
static float fresnel(float cosi, float cost, float etai, float etat) {
    const auto rs = (etat * cosi - etai * cost) / (etat * cosi + etai * cost);
    const auto rp = (etai * cosi - etat * cost) / (etai * cosi + etat * cost);
    return (rs * rs + rp * rp) * 0.5f;
}

/**
 * \brief 菲涅耳方程的近似
 * \param cosi 入射角余弦
 * \param cost 折射角余弦
 * \param etai 外界折射率
 * \param etat 介质折射率
 * \return 比重
 */
static float schlick(float cosi, float cost, float etai, float etat) {
    float r0 = (etai - etat) / (etai + etat);
    r0 *= r0;
    const float a = 1.0f - (etai < etat ? cosi : cost);
    const float aa = a * a;
    return r0 + (1.0f - r0) * aa * aa * a;
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
static color trace(float ox, float oy, float dx, float dy, int depth) {
    static color black;
    auto t = 1e-3f;
    const auto sign = g_scene(ox, oy).sd > 0.0f;                      // 判断光线自物体内部还是外部，正为外，负为内
    for (auto i = 0; i < MAX_STEP && t < MAX_DISTANCE; i++) {
        const auto x = ox + dx * t, y = oy + dy * t;
        auto r = g_scene(x, y);
        if (sign ? (r.sd < EPSILON) : (r.sd > -EPSILON)) {      // 如果线与图形有交点
            auto sum = r.emissive;                              // 用于累计采样，此处值为除去反射的正常接收光线
            if (depth < MAX_DEPTH &&
                (r.reflectivity.Valid() || r.eta > 0.0f)) {     // 在反射深度内，且允许反射
                float nx, ny, rx, ry;
                auto refl = r.reflectivity;
                gradient(x, y, &nx, &ny);                       // 求该交点处法向量
                auto s = 1.0f / sqrtf(nx * nx + ny * ny);            // 法向量单位化，这点很重要，之前代码中没有
                nx = s * (sign ? nx : -nx);                     // 当光线从形状内往外发射时，要反转法线方向
                ny = s * (sign ? ny : -ny);
                if (r.eta > 0.0f) {
                    if (refract(dx, dy, nx, ny, sign ? (1.0f / r.eta) : r.eta, &rx, &ry))
                    {
                        const auto cosi = -(dx * nx + dy * ny);
                        const auto cost = -(rx * nx + ry * ny);
                        refl = refl * (sign ? fresnel(cosi, cost, 1.0f, r.eta) : fresnel(cosi, cost, r.eta, 1.0f));
                        refl.Normalize();
                        // refl = !sign ? schlick(cosi, cost, r.eta, 1.0f) : schlick(cosi, cost, 1.0f, r.eta);
                        sum.Add((refl.Negative(1.0f)) * trace(x - nx * BIAS, y - ny * BIAS, rx, ry, depth + 1));
                    }
                    else
                        refl.Set(1.0f);                         // 不折射则为全内反射
                }
                if (refl.Valid()) {
                    reflect(dx, dy, nx, ny, &rx, &ry);          // 求出反射光线
                    // BIAS为偏差
                    // 加上反射光线 = 反射光线追踪采样值 * 反射系数
                    sum.Add(refl * trace(x + nx * BIAS, y + ny * BIAS, rx, ry, depth + 1));
                }
            }
			return sum * beerLambert(r.absorption, t);
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
        sum.Add(trace(x, y, cosf(a), sinf(a), 0)); // 追踪 (x,y) 从 随机方向(cos(a),sin(a)) 收集到的光
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
