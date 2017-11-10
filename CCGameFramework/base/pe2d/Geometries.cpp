#include "stdafx.h"
#include "Geometries.h"

Ray::Ray(const vector3& origin, const vector3& direction)
    : origin(origin), direction(direction)
{
}

vector3 Ray::Eval(float t) const
{
    return origin + (direction * t);
}

PerspectiveCamera::PerspectiveCamera(vector3 eye, vector3 front, vector3 up, float fov)
    : eye(eye), front(front), up(up), fov(fov)
{
    right = CrossProduct(front, up);
    up = CrossProduct(right, front);
    fovScale = tanf(fov * float(M_PI) / 360.0f) * 2;
}

Ray PerspectiveCamera::GenerateRay(float x, float y) const
{
    // 取样坐标(sx,sy)，投影到[-1,1]
    const auto r = right * ((x - 0.5f) * fovScale);
    const auto u = up * ((y - 0.5f) * fovScale);

    // 单位化距离向量
    return Ray(eye, Normalize(front + r + u)); // 求出起点到取样位置3D实际位置的距离单位向量
}

IntersectResult::IntersectResult()
{
}

IntersectResult::IntersectResult(Geometries* body, float distance, const vector3& position, const vector3& normal)
    : body(body), distance(distance), position(position), normal(normal)
{
}

void World::Add(std::shared_ptr<Geometries> body)
{
    collections.push_back(body);
}

IntersectResult World::Intersect(Ray ray)
{
    auto minDistance = FLT_MAX;
    IntersectResult minResult;
    for (auto & body : collections) {
        const auto result = body->Intersect(ray);
        if (result.body && result.distance < minDistance) {
            minDistance = result.distance;
            minResult = result;
        }
    }
    return minResult;
}

Sphere::Sphere(const vector3& center, float radius): center(center),
                                                     radius(radius)
{
    radiusSquare = radius * radius;
}

IntersectResult Sphere::Intersect(Ray ray)
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

    if (DdotV <= 0)
    {
        // 点乘测试相交，为负则同方向

        auto discr = (DdotV * DdotV) - a0; // 平方根中的算式

        if (discr >= 0)
        {
            // 非负则方程有解，相交成立

            // r(t) = o + t.d
            auto distance = -DdotV - sqrtf(discr); // 得出t，即摄影机发出的光线到其与球面的交点距离
            auto position = ray.Eval(distance); // 代入直线方程，得出交点位置
            auto normal = Normalize(position - center); // 法向量 = 光线终点(球面交点) - 球心坐标
            return IntersectResult(this, distance, position, normal);
        }
    }

    return IntersectResult(); // 失败，不相交
}
