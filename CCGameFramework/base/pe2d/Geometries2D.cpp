#include "stdafx.h"
#include "Geometries2D.h"

Ray2D::Ray2D(const vector3& origin, const vector3& direction)
    : origin(origin), direction(direction)
{
}

vector3 Ray2D::Eval(float t) const
{
    return origin + (direction * t);
}

Camera2D::Camera2D(vector3 eye, vector3 front, vector3 u, float scale)
    : eye(eye), front(front), up(u), scale(scale)
{
    right = Normalize(CrossProduct(front, u));
    up = Normalize(CrossProduct(right, front));
    this->front = Normalize(front);
}

Ray2D Camera2D::GenerateRay(float x, float y) const
{
    // 取样坐标(sx,sy)，投影到[-1,1]
    const auto r = right * (x - 0.5f) * scale;
    const auto u = up * (y - 0.5f) * scale;

    // 单位化距离向量
    return Ray2D(eye + r + u, front); // 求出起点到取样位置3D实际位置的距离单位向量
}

IntersectResult2D::IntersectResult2D()
{
}

IntersectResult2D::IntersectResult2D(Geometries2D* body, float distance, const vector3& position, const vector3& normal)
    : body(body), distance(distance), position(position), normal(normal)
{
}

Material2D::Material2D()
{
}

Material2D::~Material2D()
{
}

ColorMaterial2D::ColorMaterial2D(color L)
    : Material2D()
    , L(L)
{
}

color ColorMaterial2D::Sample(Ray2D ray, vector3 position, vector3 normal)
{
    return L;
}

LightSample2D::LightSample2D()
{
}

LightSample2D::LightSample2D(vector3 L, color EL)
    : L(L), EL(EL)
{
}

bool LightSample2D::empty() const
{
    return L.x == 0.0f && L.y == 0.0f && L.z == 0.0f && EL.r == 0.0f && EL.g == 0.0f && EL.b == 0.0f;
}

Light2D::~Light2D()
{
}

PointLight2D::PointLight2D(color intensity, vector3 position)
    : intensity(intensity), position(position)
{
}

static LightSample2D zero;

LightSample2D PointLight2D::Sample(World2D& world, vector3 pos)
{
    // 计算L，但保留r和r^2，供之后使用
    const auto delta = position - pos; // 距离向量
    const auto rr = SquareMagnitude(delta);
    const auto r = sqrtf(rr); // 算出光源到pos的距离
    const auto L = delta / r; // 距离单位向量

    if (shadow) {
        const Ray2D shadowRay(pos, L);
        const auto shadowResult = world.Intersect(shadowRay);
        // 在r以内的相交点才会遮蔽光源
        // shadowResult.distance <= r 表示：
        //   以pos交点 -> 光源位置 发出一条阴影测试光线
        //   如果阴影测试光线与其他物体有交点，那么相交距离 <= r
        //   说明pos位置无法直接看到光源
        if (shadowResult.body && shadowResult.distance <= r)
            return zero;
    }

    // 平方反比衰减
    const auto attenuation = 1 / rr;

    // 返回衰减后的光源颜色
    return LightSample2D(L, intensity * attenuation);
}

Geometries2D::~Geometries2D()
{
}

void World2D::AddGeometries(std::shared_ptr<Geometries2D> body)
{
    geometries.push_back(body);
}

void World2D::AddLight(std::shared_ptr<Light2D> light)
{
    lights.push_back(light);
}

IntersectResult2D World2D::Intersect(Ray2D ray)
{
    auto minDistance = FLT_MAX;
    IntersectResult2D minResult;
    for (auto & body : geometries) {
        const auto result = body->Intersect(ray);
        if (result.body && result.distance < minDistance) {
            minDistance = result.distance;
            minResult = result;
        }
    }
    return minResult;
}

Circle2D::Circle2D(const vector3& center, float radius): center(center),
                                                     radius(radius)
{
    radiusSquare = radius * radius;
}

IntersectResult2D Circle2D::Intersect(Ray2D ray)
{
    // 圆上点x满足： || 点x - 圆心center || = 圆半径radius
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
            return IntersectResult2D(this, distance, position, normal);
        }
    }

    return IntersectResult2D(); // 失败，不相交
}
