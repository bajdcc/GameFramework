#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

#define N 256
#define MAX_STEP 10
#define MAX_DISTANCE 2.0f
#define EPSILON 1e-6f

#define CIRCLE_X 1.0f
#define CIRCLE_Y 0.5f
#define CIRCLE_R 0.1f

extern auto PI2 = 2.0f * float(M_PI);

/**
 * \brief 圆的带符号距离场 SDF=||x-c||-r
 * \param x 目标X坐标
 * \param y 目标Y坐标
 * \param cx 圆心X坐标
 * \param cy 圆心Y坐标
 * \param r 圆半径
 * \return 到圆的距离
 */
extern float circleSDF(float x, float y, float cx, float cy, float r) {
    const auto ux = x - cx, uy = y - cy;
    return sqrtf(ux * ux + uy * uy) - r;
}

/**
 * \brief 光线步行法 收集dest向origin发射的光线
 * \param ox origin X坐标
 * \param oy origin Y坐标
 * \param dx d X坐标
 * \param dy dest Y坐标
 * \return 亮度
 */
static float trace(float ox, float oy, float dx, float dy) {
    auto t = 0.0f;
    for (auto i = 0; i < MAX_STEP && t < MAX_DISTANCE; i++) {
        const auto sd = circleSDF(ox + dx * t, oy + dy * t, CIRCLE_X, CIRCLE_Y, CIRCLE_R); // 圆在(0.5,0.5) r=0.1
        if (sd < EPSILON) // 已经很接近圆了
            return 2.0f;
        t += sd; // 向圆靠近
    }
    return 0.0f;
}

/**
 * \brief 采样(x,y)位置处的光线
 * \param x X坐标
 * \param y Y坐标
 * \return 亮度
 */
static float sample(float x, float y) {
    auto sum = 0.0f;
    for (auto i = 0; i < N; i++) {
        // const auto a = PI2 * rand() / RAND_MAX;                  // 均匀采样
        // const auto a = PI2 * i / N;                              // 分层采样
        const auto a = PI2 * (i + float(rand()) / RAND_MAX) / N;    // 抖动采样
        sum += trace(x, y, cosf(a), sinf(a)); // 追踪 (x,y) 从 随机方向(cos(a),sin(a)) 收集到的光
    }
    return sum / N;
}

static volatile bool* g_painted = nullptr;
static volatile BYTE* g_buf = nullptr;
static volatile int g_width, g_height;
static volatile int g_cnt = 0;
static std::mutex mtx;

static void DrawScene(int part)
{
    auto buffer = g_buf;
    auto width = g_width;
    auto height = g_height;
    auto m = min(width, height);
    for (auto y = 0; y < height; y++)
    {
        if (y % 4 == part)
        {
            for (auto x = 0; x < width; x++)
            {
                const auto color = BYTE(fminf(sample(float(x) / m, float(y) / m) * 255.0f, 255.0f));
                buffer[0] = color;
                buffer[1] = color;
                buffer[2] = color;
                buffer[3] = 255;
                buffer += 4;
            }
        }
        else
            buffer += 4 * width;
    }
    mtx.lock();
    g_cnt++;
    if (g_cnt == 4)
        *g_painted = true;
    mtx.unlock();
}

void PhysicsEngine::Render2DLight(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    if (painted || buf.get())
    {
        if (buf.get())
        {
            auto d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
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
        wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buf.get());
    }

    bitmap = _rt->GetBitmapFromWIC(wic);
    rt->DrawBitmap(
        bitmap,
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );

    g_painted = &painted;
    g_buf = buf.get();
    g_width = _w;
    g_height = _h;
    g_cnt = 0;
    // ----------------------------------------------------
    // 位图渲染开始
    std::thread th0(DrawScene, 0);
    th0.detach();
    std::thread th1(DrawScene, 1);
    th1.detach();
    std::thread th2(DrawScene, 2);
    th2.detach();
    std::thread th3(DrawScene, 3);
    th3.detach();
    // ----------------------------------------------------
}
