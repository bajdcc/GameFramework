#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"
#include "ui/window/Window.h"
#include <queue>

#define SCAN_N 10

#define N 1024
#define MAX_STEP 800
#define MAX_DISTANCE 2.0f
//#define EPSILON 1e-6f
#define BIAS 1e-4f
#define MAX_DEPTH 5
#define DES 0.7f

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

static std::vector<float> g_buf;
static int g_width;
static int g_height;
static int g_m;
static float g_pixel;
static int g_max;

static float fontSDF(float x, float y) {
    const auto j = int(round(y * g_m));
    const auto i = int(round(x * g_m));
    if (i < 0 || j < 0 || i >= g_width || j >= g_height)
        return 100.0f;
    const auto idx = j * g_width + i;
    return g_buf[idx];
}

extern Result unionOp(Result a, Result b);
extern Result intersectOp(Result a, Result b);
extern Result subtractOp(Result a, Result b);

static Result scene_ref(float x, float y)
{
    //Result c = { circleSDF(x, y, 0.7f, -0.3f, 0.05f), color(10.0f, 10.0f, 0.0f), color(0.0f, 0.0f, 0.0f), 0.0f };
    Result i = { fontSDF(x, y), color(0.9f, 0.9f, 0.9f), color(0.0f, 0.0f, 0.0f), 0.0f };
    return i;// unionOp(c, i);
}

extern Result(*g_scene)(float x, float y);

extern void gradient(float x, float y, float* nx, float* ny);
extern void reflect(float ix, float iy, float nx, float ny, float* rx, float* ry);
extern int refract(float ix, float iy, float nx, float ny, float eta, float* rx, float* ry);

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
    auto sign = g_scene(ox, oy).sd > 0.0f;                      // 判断光线自物体内部还是外部，正为外，负为内
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
                nx = sign ? nx : -nx, ny = sign ? ny : -ny;     // 当光线从形状内往外发射时，要反转法线方向
                if (r.eta > 0.0f) {
                    if (refract(dx, dy, nx, ny, !sign ? r.eta : 1.0f / r.eta, &rx, &ry))
                        sum.Add((refl.Negative(1.0f)) * trace(x - nx * BIAS, y - ny * BIAS, rx, ry, depth + 1));
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
        float a = float(PI2 * (i + double(rand()) / RAND_MAX) / N);    // 抖动采样
        sum.Add(trace(x, y, cosf(a), sinf(a), 0)); // 追踪 (x,y) 从 随机方向(cos(a),sin(a)) 收集到的光
    }
    return sum * (1.0f / N);
}

extern DrawSceneBag bag;

static void DrawSceneFont(int part)
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
#if 0
                auto idx = y * g_width + x;
                const auto color = BYTE(g_buf[idx] * 255);
                const auto b = color;
                buffer[0] = b;
                buffer[1] = b;
                buffer[2] = b;
                buffer[3] = 255;
                buffer += 4;
#else
                const auto color = sample(float(x) / m, float(y) / m);
                buffer[0] = BYTE(fminf(color.b, 1.0f) * 255.0f);
                buffer[1] = BYTE(fminf(color.g, 1.0f) * 255.0f);
                buffer[2] = BYTE(fminf(color.r, 1.0f) * 255.0f);
                buffer[3] = 255;
                buffer += 4;
#endif
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

void PhysicsEngine::Render2DFont(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    if (buf.get()) {
        RenderSceneIntern(rt, bounds);
        return;
    }
    // ---------------------------------------------
    // 渲染字体SDF
    auto _rt = d2drt.lock();
    auto _w = bounds.Width(), _h = bounds.Height();
    g_buf.resize(_w * _h);
    g_width = _w;
    g_height = _h;
    g_max = _w * _h;
    auto m = min(_w, _h);
    g_m = m;
    g_pixel = 1.0f / m;

    auto hwnd = window->GetWindowHandle();
    auto hDC = ::GetDC(hwnd);
    auto hBitmap = ::CreateCompatibleBitmap(hDC, _w, _h);
    auto hMem = ::CreateCompatibleDC(hDC);
    ::ReleaseDC(hwnd, hDC);
    ::SelectObject(hMem, hBitmap);

    BITMAP bm;
    BITMAPINFO bmpInf;
    memset(&bmpInf.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
    if (::GetObject(hBitmap, sizeof(bm), &bm) == 0)
    {
        ::ReleaseDC(hwnd, hDC);
        return;
    }

    ::SetBkMode(hMem, OPAQUE);
    ::FillRect(hMem, &bounds, (HBRUSH)::GetStockObject(BLACK_BRUSH));
    ::SetBkColor(hMem, RGB(0, 0, 0));
    ::SetTextColor(hMem, RGB(255, 255, 255));

    auto hFont = ::CreateFont(
        100/*高度*/, 0/*宽度*/, 0/*不用管*/, 0/*不用管*/, FW_NORMAL/*一般这个值设为400*/,
        FALSE/*不带斜体*/, FALSE/*不带下划线*/, FALSE/*不带删除线*/,
        DEFAULT_CHARSET,  //这里我们使用默认字符集，还有其他以 _CHARSET 结尾的常量可用
        OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,  //这行参数不用管
        ANTIALIASED_QUALITY,  //默认输出质量
        FF_DONTCARE,  //不指定字体族*/
        _T("楷体") //字体名
    );
    ::SelectObject(hMem, hFont);

    CString s(_T("文字透明效果\nby bajdcc"));
    CRect rect(bounds);
    rect.DeflateRect(100, 10);
    ::DrawText(hMem, s.GetBuffer(0), s.GetLength(), &rect, DT_EDITCONTROL | DT_WORDBREAK | DT_CENTER | DT_NOPREFIX);

    // 取像素
    bmpInf.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInf.bmiHeader.biWidth = _w;
    bmpInf.bmiHeader.biHeight = _h;
    bmpInf.bmiHeader.biPlanes = 1;
    bmpInf.bmiHeader.biBitCount = 32;
    bmpInf.bmiHeader.biCompression = BI_RGB;
    bmpInf.bmiHeader.biSizeImage = (bm.bmWidth + 7) / 8 * bm.bmHeight*bm.bmBitsPixel;
    auto buf = new UINT[_w * _h];
    if (::GetDIBits(hMem, hBitmap, 0, UINT(bm.bmHeight), buf, &bmpInf, DIB_RGB_COLORS))
    {
        if (bmpInf.bmiHeader.biBitCount == 32)
        {
            struct font_bag
            {
                WORD x0, y0;
                WORD x, y;
                float dist;
            };

            std::queue<font_bag> Q;
            const auto nWidth = bm.bmWidth;
            const auto nHeight = bm.bmHeight - 1;
            for (auto i = 0; i < bm.bmHeight; i++)
            {
                const auto nOffset = i*nWidth;
                for (auto j = 0; j < bm.bmWidth; j++)
                {
                    const auto c = UINT(buf[nOffset + j]);

                    if (c != 0) // 字
                    {
                        Q.push(font_bag{
                            WORD(nHeight - i), WORD(j), // 起始点
                            WORD(nHeight - i), WORD(j), // 上一次
                            float(0.0f) });
                        g_buf[(nHeight - i)*nWidth + j] = -1.0f;
                    }
                    else
                    {
                        g_buf[(nHeight - i)*nWidth + j] = 0.0f;
                    }
                }
            }
            for (auto i = 0; i < bm.bmHeight; i++)
            {
                for (auto j = 0; j < bm.bmWidth; j++)
                {
                    if (g_buf[i*nWidth + j] == 0.0f)
                        buf[i*nWidth + j] = 0; // 无字没访问
                    else
                        buf[i*nWidth + j] = 1; // 有字访问过
                }
            }

            static const int direction[4][2] = {
                { 0, 1 },
                { 0, -1 },
                { 1, 0 },
                { -1, 0 }
            };
            // BFS
            while (!Q.empty())
            {
                const auto front = Q.front();
                Q.pop();
                const auto x0 = front.x0;
                const auto y0 = front.y0;
                const auto x = front.x;
                const auto y = front.y;
                for (auto i = 0; i < 4; i++)
                {
                    const auto x1 = x + direction[i][0];
                    const auto y1 = y + direction[i][1];
                    if (x1 >= 0 && y1 >= 0 && x1 < _h && y1 < _w)
                    {
                        const auto idx = x1 * nWidth + y1;
                        const auto dist = (float)((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
                        if (buf[idx] == 0) // 没访问
                        {
                            Q.push(font_bag{
                                WORD(x0), WORD(y0),
                                WORD(x1), WORD(y1),
                                dist });
                            g_buf[idx] = dist;
                            buf[idx] = 1; // 访问过
                        }
                        else if (g_buf[idx] > 0.0f && g_buf[idx] > dist)
                        {
                            Q.push(font_bag{
                                WORD(x0), WORD(y0),
                                WORD(x1), WORD(y1),
                                dist });
                            g_buf[idx] = dist;
                        }
                    }
                }
            }
            const auto M = 1.0f / m;
            for (auto i = 0; i < bm.bmHeight; i++)
            {
                for (auto j = 0; j < bm.bmWidth; j++)
                {
                    if (g_buf[i * nWidth + j] > 0.0f)
                        g_buf[i * nWidth + j] = sqrtf(g_buf[i * nWidth + j]) * M * DES;
                }
            }
        }
    }
    delete[]buf;
    ::DeleteObject(hFont);
    ::DeleteObject(hMem);
    ::DeleteDC(hMem);
    ::ReleaseDC(hwnd, hDC);
    // ---------------------------------------------
    g_scene = ::scene_ref;
    scene = DrawSceneFont;
    std::auto_ptr<BYTE> b;
    {
        b.reset(new BYTE[_w * _h * 4]);
        const auto buffer = b.get();
        auto idx = 0;
        for (auto i = 0; i < _h; i++)
        {
            for (auto j = 0; j < _w; j++)
            {
                const int c = g_buf[idx] < 0.0f ? 255 : (int)min(255.0f, (1.0f * DES - g_buf[idx]) * 255.0f);
                buffer[(idx << 2) + 0] = c;
                buffer[(idx << 2) + 1] = c;
                buffer[(idx << 2) + 2] = c;
                buffer[(idx << 2) + 3] = 255;
                idx++;
            }
        }
    }
    RenderSceneIntern(rt, bounds, b.get());
}