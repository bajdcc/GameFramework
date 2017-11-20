#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"
#include "ui/window/Window.h"
#include <queue>

#define SCAN_N 10

#define N 64
#define MAX_STEP 64
#define MAX_DISTANCE 5.0f
#define EPSILON 1e-6f
#define BIAS 1e-4f
#define MAX_DEPTH 3

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

static std::vector<int> g_buf;
static int g_width;
static int g_height;
static int g_max;

static float fontSDF(float x, float y) {
    auto idx = int(y * g_height) * g_width + int(x * g_height);
    if (idx >= 0 && idx < g_max)
        return float(g_buf[idx]) / g_height;
    return float(g_buf[0]) / g_height;
}

extern Result unionOp(Result a, Result b);
extern Result intersectOp(Result a, Result b);
extern Result subtractOp(Result a, Result b);

static Result scene_ref(float x, float y)
{
    Result c = { circleSDF(x, y, 0.7f, -0.3f, 0.05f), color(10.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f), 0.0f };
    Result i = { fontSDF(x, y), color(0.1f, 0.1f, 0.1f), color(0.0f, 0.0f, 0.0f), 1.5f };
    return unionOp(c, i);
}

extern Result(*g_scene)(float x, float y);

extern void gradient(float x, float y, float* nx, float* ny);
extern void reflect(float ix, float iy, float nx, float ny, float* rx, float* ry);

extern color trace_ref(float ox, float oy, float dx, float dy, int depth);

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
                //const auto color = sample(float(x) / m, float(y) / m);
                //auto idx = y * g_width + x;
                //const auto color = (idx >= 0 && idx < g_max) ? (g_buf[idx] % 256) : 0;
                //buffer[0] = color;
                //buffer[1] = color;
                //buffer[2] = color;
                //buffer[3] = 255;
                //buffer += 4;
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

void PhysicsEngine::Render2DFont(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    // ---------------------------------------------
    // 渲染字体SDF
    auto _rt = d2drt.lock();
    auto _w = bounds.Width(), _h = bounds.Height();
    g_buf.resize(_w * _h);
    g_width = _w;
    g_height = _h;
    g_max = _w * _h;

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
        190/*高度*/, 0/*宽度*/, 0/*不用管*/, 0/*不用管*/, FW_NORMAL/*一般这个值设为400*/,
        FALSE/*不带斜体*/, FALSE/*不带下划线*/, FALSE/*不带删除线*/,
        DEFAULT_CHARSET,  //这里我们使用默认字符集，还有其他以 _CHARSET 结尾的常量可用
        OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,  //这行参数不用管
        DEFAULT_QUALITY,  //默认输出质量
        FF_DONTCARE,  //不指定字体族*/
        _T("微软雅黑") //字体名
    );
    ::SelectObject(hMem, hFont);

    CString s(_T("bajdcc"));
    CRect rect(bounds);
    rect.DeflateRect(100, 50);
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
                WORD x, y;
                WORD dist;
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
                        Q.push(font_bag{ WORD(nHeight - i), WORD(j), WORD(0) });
                        g_buf[(nHeight - i)*nWidth + j] = 0;
                    }
                    else
                    {
                        g_buf[(nHeight - i)*nWidth + j] = -1;
                    }
                }
            }
            for (auto i = 0; i < bm.bmHeight; i++)
            {
                for (auto j = 0; j < bm.bmWidth; j++)
                {
                    if (g_buf[i*nWidth + j] == 0)
                        buf[i*nWidth + j] = 1;
                    else
                        buf[i*nWidth + j] = 0;
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
                const auto x = front.x;
                const auto y = front.y;
                const auto dist = front.dist + 1;
                for (auto i = 0; i < 4; i++)
                {
                    const auto x1 = x + direction[i][0];
                    const auto y1 = y + direction[i][1];
                    if (x1 >= 0 && y1 >= 0 && x1 < _h && y1 < _w)
                    {
                        if (buf[x1 * nWidth + y1] == 0)
                        {
                            Q.push(font_bag{ WORD(x1), WORD(y1), WORD(dist) });
                            g_buf[x1 * nWidth + y1] = dist;
                            buf[x1 * nWidth + y1] = 1;
                        }
                    }
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
    RenderSceneIntern(rt, bounds);
}
