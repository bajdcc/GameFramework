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

color::color(BYTE r, BYTE g, BYTE b)
    : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f)
{
}

color::color(float r, float g, float b)
    : r(r), g(g), b(b)
{
}

color::color(Gdiplus::Color clr)
    : r(clr.GetR() / 255.0f), g(clr.GetG() / 255.0f), b(clr.GetB() / 255.0f)
{
}

color color::operator+(const color& c) const
{
    // 注意溢出，区间[0,1]
    return color(fmin(r + c.r, 1.0f), fmin(g + c.g, 1.0f), fmin(b + c.b, 1.0f));
}

color color::operator*(float s) const
{
    return color(r * s, g * s, b * s);
}

color color::operator*(const color& c) const
{
    return color(r * c.r, g * c.g, b * c.b);
}

Material::Material(float reflectiveness): reflectiveness(reflectiveness)
{
}

Material::~Material()
{
}

CheckerMaterial::CheckerMaterial(float scale, float reflectiveness)
    : Material(reflectiveness)
    , scale(scale)
{
}

color CheckerMaterial::Sample(Ray ray, vector3 position, vector3 normal)
{
    static color black(Gdiplus::Color::Black);
    static color white(Gdiplus::Color::White);
    return fabs(int(floorf(position.x * 0.1f) + floorf(position.z * scale)) % 2) < 1 ? black : white;
}

// global temp
auto lightDir = Normalize(vector3(1.0f, 1.0f, 1.0f));
color lightColor(Gdiplus::Color::White);

PhongMaterial::PhongMaterial(color diffuse, color specular, float shininess, float reflectiveness)
    : Material(reflectiveness)
    , diffuse(diffuse)
    , specular(specular)
    , shininess(shininess)
{
}

color PhongMaterial::Sample(Ray ray, vector3 position, vector3 normal)
{
    /*
      参考 https://www.cnblogs.com/bluebean/p/5299358.html Blinn-Phong模型
        Ks：物体对于反射光线的衰减系数
        N：表面法向量
        H：光入射方向L和视点方向V的中间向量
        Shininess：高光系数

        Specular = Ks * lightColor * pow(dot(N, H), shininess)

        当视点方向和反射光线方向一致时，计算得到的H与N平行，dot(N,H)取得最大；当视点方向V偏离反射方向时，H也偏离N。
        简单来说，入射光与视线的差越接近法向量，镜面反射越明显
     */

    const auto NdotL = DotProduct(normal, lightDir);
    const auto H = Normalize(lightDir - ray.direction);
    const auto NdotH = DotProduct(normal, H);
    const auto diffuseTerm = diffuse * fmax(NdotL, 0.0f); // N * L 入射光在镜面法向上的投影 = 漫反射
    const auto specularTerm = specular * powf(fmax(NdotH, 0.0f), shininess);
    return lightColor * (diffuseTerm + specularTerm);
}

Geometries::~Geometries()
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

Plane::Plane(const vector3& normal, float d)
    : normal(normal)
    , d(d)
{
    position = normal * d;
}

IntersectResult Plane::Intersect(Ray ray)
{
    const auto a = DotProduct(ray.direction, normal);

    if (a >= 0)
        // 反方向看不到平面，负数代表角度为钝角
        // 举例，平面法向量n=(0,1,0)，距离d=0，
        // 我从上面往下看，光线方向为y轴负向，而平面法向为y轴正向
        // 所以两者夹角为钝角，上面的a为cos(夹角)=负数，不满足条件
        // 当a为0，即视线与平面平行时，自然看不到平面
        // a为正时，视线从平面下方向上看，看到平面的反面，因此也看不到平面
        return IntersectResult();

    // 参考 http://blog.sina.com.cn/s/blog_8f050d6b0101crwb.html
    /* 将直线方程写成参数方程形式，即有：
       L(x,y,z) = ray.origin + ray.direction * t(t 就是距离 dist)
       将平面方程写成点法式方程形式，即有：
       plane.normal . (P(x,y,z) - plane.position) = 0
       解得 t = {(plane.position - ray.origin) . normal} / (ray.direction . plane.normal )
    */
    const auto b = DotProduct(normal, ray.origin - position);

    const auto dist = -b / a;
    return IntersectResult(this, dist, ray.Eval(dist), normal);
}
