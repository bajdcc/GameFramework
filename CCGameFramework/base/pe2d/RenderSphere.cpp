#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "math/vector3.h"

// 光线
class Ray
{
public:
    Ray(const vector3& origin, const vector3& direction)
        : origin(origin),
          direction(direction)
    {
    }

    vector3 origin;    // 起点
    vector3 direction; // 方向

    // 依方程 r(t)=o+t.d 求出解
    vector3 Eval(float t)
    {
        return origin + (direction * t);
    }
};

// 透视摄影机
class PerspectiveCamera
{
public:
    vector3 eye;     // 摄影机眼睛的位置
    vector3 front;   // 视角中向前方向的单位向量
    vector3 up;      // 视角中向上方向的单位向量
    vector3 right;   // 视角中向右方向的单位向量
    float fov;       // 视角
    float fovScale;  // 视角缩放

    PerspectiveCamera(vector3 eye, vector3 front, vector3 up, float fov): eye(eye), front(front), up(up), fov(fov)
    {
        right = CrossProduct(front, up);
        up = CrossProduct(right, front);
        fovScale = tanf(fov * (float)M_PI / 360.0f) * 2;
    }

    // 产生追踪光线
    Ray GenerateRay(float x, float y)
    {
        // 取样坐标(sx,sy)，投影到[-1,1]
        auto r = right * ((x - 0.5f) * fovScale);
        auto u = up * ((y - 0.5f) * fovScale);

        // 单位化距离向量
        return Ray(eye, Normalize(front + r + u)); // 求出起点到取样位置3D实际位置的距离单位向量
    }
};

// 相交测试结果
class IntersectResult
{
public:
    IntersectResult(float distance, const vector3& position, const vector3& normal)
        : hit(true),
        distance(distance),
        position(position),
        normal(normal)
    {
    }

    IntersectResult(bool hit)
        : hit(hit)
    {
    }

    bool hit;          // 是否相交
    float distance;    // 距离
    vector3 position;  // 位置
    vector3 normal;    // 法向量
};

// 球体
class Sphere
{
public:
    Sphere(const vector3& center, float radius)
        : center(center),
          radius(radius)
    {
        radiusSquare = radius * radius;
    }

    vector3 center;     // 球心坐标
    float radius;       // 半径
    float radiusSquare; // 缓存半径平方

    // 相交测试
    IntersectResult Intersect(Ray ray)
    {
        // 球面上点x满足： || 点x - 球心center || = 球半径radius
        // 光线方程 r(t) = o + t.d (t>=0)
        // 代入得 || o + t.d - c || = r
        // 令 v = o - c，则 || v + t.d || = r

        // 化简求 t = - d.v - sqrt( (d.v)^2 + (v^2 - r^2) )  (求最近点)

        // 令 v = origin - center
        auto v = ray.origin - center;

        // a0 = (v^2 - r^2)
        auto a0 = SquareMagnitude(v) - radiusSquare;

        // DdotV = d.v
        auto DdotV = DotProduct(ray.direction, v);

        if (DdotV <= 0) { // 点乘测试相交，为负则同方向

            auto discr = (DdotV * DdotV) - a0; // 平方根中的算式
            ATLTRACE("%f\n", discr);

            if (discr >= 0) { // 非负则方程有解，相交成立

                // r(t) = o + t.d
                auto distance = -DdotV - sqrtf(discr);      // 得出t，即摄影机发出的光线到其与球面的交点距离
                auto position = ray.Eval(distance);         // 代入直线方程，得出交点位置
                auto normal = Normalize(position - center); // 法向量 = 起点(摄影机) - 终点(球面交点)
                return IntersectResult(distance, position, normal);
            }
        }

        return IntersectResult(false); // 失败，不相交
    }
};

void PhysicsEngine::RenderSphereIntern(BYTE * buffer, BYTE * buffer2, cint width, cint height)
{
    // 注意：RELEASE下速度更快，DEBUG比较慢

    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 10, 10),   // 摄影机眼睛的位置
        vector3(0, 0, -1),    // 视角中向前方向的单位向量
        vector3(0, 1, 0),     // 视角中向上方向的单位向量
        90.0f);               // FOV

    auto maxDepth = 20;       // 最大深度

    // -------------------------------------
    // 球体
    Sphere sphere(
        vector3(0.0f, 10.0f, -10.0f), // 球心坐标
        10.0f                         // 半径
    );

    // -------------------------------------
    // 光线追踪
    for (auto y = 0; y < height; y++)
    {
        auto sy = 1.0f - (1.0f * y / height);

        for (auto x = 0; x < width; x++)
        {
            auto sx = 1.0f * x / width;

            // sx和sy将屏幕投影到[0,1]区间

            // 产生光线
            auto ray = camera.GenerateRay(sx, sy);

            // 测试光线与球是否相交
            auto result = sphere.Intersect(ray);
            if (result.hit)
            {
                auto depth = 255 - min((result.distance / maxDepth) * 255.0f, 255.0f);
                // 输出灰阶
                buffer[0] = (BYTE)depth;
                buffer[1] = (BYTE)depth;
                buffer[2] = (BYTE)depth;
                buffer[3] = 255;
                buffer2[0] = (BYTE)((result.normal.x + 1) * 128); // 法向[-1,1]映射至[0,255]
                buffer2[1] = (BYTE)((result.normal.y + 1) * 128);
                buffer2[2] = (BYTE)((result.normal.z + 1) * 128);
                buffer2[3] = 255;
            }
            else
            {
                // 没有接触，就是背景色
                buffer[0] = 0;
                buffer[1] = 0;
                buffer[2] = 0;
                buffer[3] = 255;
                buffer2[0] = 0;
                buffer2[1] = 0;
                buffer2[2] = 0;
                buffer2[3] = 255;
            }

            buffer += 4;
            buffer2 += 4;
        }
    }
}
