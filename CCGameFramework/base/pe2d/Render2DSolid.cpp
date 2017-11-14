#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"

#define SCAN_N 10

#define N 64
#define MAX_STEP 64
#define MAX_DISTANCE 2.0f
#define EPSILON 1e-6f

extern float PI2;

typedef struct
{
	float sd;           // 带符号距离（signed distance）
	float emissive;     // 自发光强度（emissive）
} Result;

/**
* \brief 求并集
* \param a
* \param b
* \return
*/
Result unionOp(Result a, Result b) {
	return a.sd < b.sd ? a : b; // 取SD最小的步进
}

/**
* \brief 求交集
* \param a
* \param b
* \return
*/
Result intersectOp(Result a, Result b) {
	Result r = a.sd > b.sd ? b : a;
	r.sd = a.sd > b.sd ? a.sd : b.sd;
	return r;
}

/**
* \brief 求差集
* \param a
* \param b
* \return
*/
Result subtractOp(Result a, Result b) {
	Result r = a;
	r.sd = (a.sd > -b.sd) ? a.sd : -b.sd;
	return r;
}

/**
* \brief 取反
* \param a
* \return
*/
Result complementOp(Result a) {
	a.sd = -a.sd;
	return a;
}

extern float circleSDF(float x, float y, float cx, float cy, float r);

/**
* \brief 平面的SDF
* \param x 目标X坐标
* \param y 目标Y坐标
* \param px 平面任意一点X坐标
* \param py 平面任意一点Y坐标
* \param nx 该点上法向量X坐标
* \param ny 该点上法向量Y坐标
* \return SD
*/
float planeSDF(float x, float y, float px, float py, float nx, float ny) {
	return (x - px) * nx + (y - py) * ny;
}

/**
 * \brief 线段的SDF
 * \param x 目标X坐标
 * \param y 目标Y坐标
 * \param ax 线段起点X坐标
 * \param ay 线段起点Y坐标
 * \param bx 线段终点X坐标
 * \param by 线段终点Y坐标
 * \return SD
 */
float segmentSDF(float x, float y, float ax, float ay, float bx, float by) {
	/*
	 * x' = a + max( min(v . u单位, u长度), 0 ) . u单位
	 * max和min是用来过滤的
	 * 
	 * x' 为 (x,y)在线段上的投影
	 * 情况一：x'在线段上，那么 0 <= v在u投影 <= u长度， x' = a + v在u投影
	 * 情况二：x'在b延长线上，那么v在u投影 > u长度，取u长度，x' = a + b = b点坐标
	 * 情况三：x'在a延长线上，那么v在u投影 为负，取0，x' = a = a点坐标
	 */

	float vx = x - ax, vy = y - ay, ux = bx - ax, uy = by - ay;
	float t = fmaxf(fminf((vx * ux + vy * uy) / (ux * ux + uy * uy), 1.0f), 0.0f);
	float dx = vx - ux * t, dy = vy - uy * t;
	return sqrtf(dx * dx + dy * dy);
}

/**
 * \brief 胶囊形的SDF
 * \param x 目标X坐标
 * \param y 目标Y坐标
 * \param ax 线段起点X坐标
 * \param ay 线段起点Y坐标
 * \param bx 线段终点X坐标
 * \param by 线段终点Y坐标
 * \param r 半径
 * \return SD
 */
float capsuleSDF(float x, float y, float ax, float ay, float bx, float by, float r) {
	// 胶囊 = { 离一段线段(ax,ay)(bx,by) 距离<=r 的点集 }
	return segmentSDF(x, y, ax, ay, bx, by) - r;
}

/**
 * \brief 矩形的SDF
 * \param x 目标X坐标
 * \param y 目标Y坐标
 * \param cx 中心点X坐标
 * \param cy 中心点Y坐标
 * \param theta 旋转角度
 * \param sx X轴半长
 * \param sy Y轴半长
 * \return SD
 */
float boxSDF(float x, float y, float cx, float cy, float theta, float sx, float sy) {
	float costheta = cosf(theta), sintheta = sinf(theta);
	// x' = [(cos,sin),(-sin,cos)] (x-c)
	float dx = fabs((x - cx) * costheta + (y - cy) * sintheta) - sx;
	float dy = fabs((y - cy) * costheta - (x - cx) * sintheta) - sy;
	float ax = fmaxf(dx, 0.0f), ay = fmaxf(dy, 0.0f);
	return fminf(fmaxf(dx, dy), 0.0f) + sqrtf(ax * ax + ay * ay);
}

/**
 * \brief 三角形的SDF
 * \param x 目标X坐标
 * \param y 目标Y坐标
 * \param ax 点A的X坐标
 * \param ay 点A的Y坐标
 * \param bx 点B的X坐标
 * \param by 点B的Y坐标
 * \param cx 点C的X坐标
 * \param cy 点C的Y坐标
 * \return SD
 */
float triangleSDF(float x, float y, float ax, float ay, float bx, float by, float cx, float cy) {
	// 到三条线段的最短距离
	// 注意：三点顺序为顺时针
	float d = fminf(fminf(
		segmentSDF(x, y, ax, ay, bx, by),
		segmentSDF(x, y, bx, by, cx, cy)),
		segmentSDF(x, y, cx, cy, ax, ay));
	return (bx - ax) * (y - ay) > (by - ay) * (x - ax) &&
		(cx - bx) * (y - by) > (cy - by) * (x - bx) &&
		(ax - cx) * (y - cy) > (ay - cy) * (x - cx) ? -d : d; // 判断内外
}

/**
* \brief 返回场景中多个几何物体的带符号距离和发光强度
* \param x 目标X坐标
* \param y 目标Y坐标
* \return 带符号距离和发光强度
*/
static Result scene(float x, float y) {
	Result r1 = { circleSDF(x, y, 0.4f, 0.3f, 0.20f), 1.0f };
	Result r2 = { circleSDF(x, y, 0.6f, 0.3f, 0.20f), 0.8f };
	Result r3 = { circleSDF(x, y, 0.8f, 0.7f, 0.15f), 0.8f };
	Result r4 = { circleSDF(x, y, 0.9f, 0.7f, 0.15f), 0.6f };
	Result r5 = { circleSDF(x, y, 1.1f, 0.5f, 0.10f), 0.0f };

	Result r10 = { circleSDF(x, y, 1.5f, 0.5f, 0.10f), 1.0f };
	Result r11 = { planeSDF(x, y, 1.0f, 0.5f, 0.0f, 1.0f), 0.8f };

	Result r20 = { capsuleSDF(x, y, 0.4f, 0.7f, 0.6f, 0.7f, 0.05f), 1.0f };
	Result r21 = { boxSDF(x, y, 1.5f, 0.2f, 0.0f, 0.1f, 0.05f), 1.0f };
	Result r22 = { triangleSDF(x, y, 1.0f, 0.2f, 1.2f, 0.2f, 1.1f, 0.3f), 1.0f };

	return unionOp(
		unionOp(
			unionOp(
				unionOp(
					subtractOp(r1, r2),
					intersectOp(r3, r4)),
				r5),
			intersectOp(r10, r11)),
		unionOp(unionOp(r20, r21), r22));
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
	auto t = 0.001f;
	for (auto i = 0; i < MAX_STEP && t < MAX_DISTANCE; i++) {
		auto r = scene(ox + dx * t, oy + dy * t);   // <- 返回场景中离目标最近的点和发光强度 
		if (r.sd < EPSILON)                         // <- 就在发光物体上
			return r.emissive;                      // <- 直接返回发光强度（自发光的亮度）
		t += r.sd;                                  // <- 步进
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

void PhysicsEngine::Render2DSolid(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
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
