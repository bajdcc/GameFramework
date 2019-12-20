#include "stdafx.h"
#include "Geometries.h"
#define EPSILON 1e-6f
#define TRIANGLE_CULLING 1

Ray::Ray(const vector3& origin, const vector3& direction)
    : origin(origin), direction(direction)
{
}

vector3 Ray::Eval(float t) const
{
    return origin + (direction * t);
}

PerspectiveCamera::PerspectiveCamera(vector3 eye, vector3 front, vector3 u, float fov, float asp)
    : eye(eye), front(front), up(u), fov(fov), asp(asp)
{
    right = Normalize(CrossProduct(front, u));
    up = Normalize(CrossProduct(right, front));
    fovScale = tanf(fov * float(M_PI) / 360.0f) * 2;
}

Ray PerspectiveCamera::GenerateRay(float x, float y) const
{
    // 取样坐标(sx,sy)，投影到[-1,1]
    const auto r = right * ((x - 0.5f) * asp * fovScale);
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

color::color()
    : r(0.0f), g(0.0f), b(0.0f)
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

void color::Add(const color& c)
{
    r += c.r; g += c.g; b += c.b;
}

const bool color::Valid() const
{
    return r > 0.0f && g > 0.0f && b > 0.0f;
}

void color::Set(float s)
{
    r = s; g = s; b = s;
}

color color::Negative(float s)
{
    return color(s - r, s - g, s - b);
}

void color::Normalize()
{
    r = fmax(0.0f, fmin(r, 1.0f)); g = fmax(0.0f, fmin(g, 1.0f)); b = fmax(0.0f, fmin(b, 1.0f));
}

color color::make_color(float s)
{
    return color(s, s, s);
}

color color::make_color(float r, float g, float b)
{
    return color(r, g, b);
}

Material::Material(float reflectiveness) : reflectiveness(reflectiveness)
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

SolidMaterial::SolidMaterial(color diffuse, float reflectiveness)
    : Material(reflectiveness)
    , diffuse(diffuse)
{
}

color SolidMaterial::Sample(Ray ray, vector3 position, vector3 normal)
{
    return diffuse * -DotProduct(ray.direction, normal);
}

LightSample::LightSample()
{
}

LightSample::LightSample(vector3 L, color EL)
    : L(L), EL(EL)
{
}

bool LightSample::empty() const
{
    return L.x == 0.0f && L.y == 0.0f && L.z == 0.0f && EL.r == 0.0f && EL.g == 0.0f && EL.b == 0.0f;
}

Light::~Light()
{
}

DirectionalLight::DirectionalLight(color irradiance, vector3 direction)
    : irradiance(irradiance), direction(direction)
{
    L = -Normalize(direction);
}

LightSample DirectionalLight::Sample(World& world, vector3 position)
{
    static LightSample zero;

    if (shadow) {
        const Ray shadowRay(position, L);
        const auto shadowResult = world.Intersect(shadowRay);
        if (shadowResult.body)
            return zero;
    }

    return LightSample(L, irradiance); // 就返回光源颜色
}

PointLight::PointLight(color intensity, vector3 position)
    : intensity(intensity), position(position)
{
}

static LightSample zero;

LightSample PointLight::Sample(World& world, vector3 pos)
{
    // 计算L，但保留r和r^2，供之后使用
    const auto delta = position - pos; // 距离向量
    const auto rr = SquareMagnitude(delta);
    const auto r = sqrtf(rr); // 算出光源到pos的距离
    const auto L = delta / r; // 距离单位向量

    if (shadow) {
        const Ray shadowRay(pos, L);
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
    return LightSample(L, intensity * attenuation);
}

SpotLight::SpotLight(color intensity, vector3 position, vector3 direction, float theta, float phi, float falloff)
    : intensity(intensity), position(position), direction(direction), theta(theta), phi(phi), falloff(falloff)
{
    S = -Normalize(direction);
    cosTheta = cosf(theta * float(M_PI) / 360.0f);
    cosPhi = cosf(phi * float(M_PI) / 360.0f);
    baseMultiplier = 1.0f / (cosTheta - cosPhi);
}

LightSample SpotLight::Sample(World& world, vector3 pos)
{
    // 计算L，但保留r和r^2，供之后使用
    const auto delta = position - pos; // 距离向量
    const auto rr = SquareMagnitude(delta);
    const auto r = sqrtf(rr); // 算出光源到pos的距离
    const auto L = delta / r; // 距离单位向量

    /*
     * spot(alpha) =
     *
     *     1
     *         where cos(alpha) >= cos(theta/2)
     *
     *     pow( (cos(alpha) - cos(phi/2)) / (cos(theta/2) - cos(phi/2)) , p)
     *         where cos(phi/2) < cos(alpha) < cos(theta/2)
     *
     *     0
     *         where cos(alpha) <= cos(phi/2)
     */

     // 计算spot
    auto spot = 0.0f;
    const auto SdotL = DotProduct(S, L);
    if (SdotL >= cosTheta)
        spot = 1.0f;
    else if (SdotL <= cosPhi)
        spot = 0.0f;
    else
        spot = powf((SdotL - cosPhi) * baseMultiplier, falloff);

    if (shadow) {
        const Ray shadowRay(pos, L);
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
    return LightSample(L, intensity * (attenuation * spot));
}

Geometries::~Geometries()
{
}

void World::AddGeometries(std::shared_ptr<Geometries> body)
{
    geometries.push_back(body);
}

void World::AddLight(std::shared_ptr<Light> light)
{
    lights.push_back(light);
}

IntersectResult World::Intersect(Ray ray)
{
    auto minDistance = FLT_MAX;
    IntersectResult minResult;
    for (auto& body : geometries) {
        const auto result = body->Intersect(ray);
        if (result.body && result.distance < minDistance) {
            minDistance = result.distance;
            minResult = result;
        }
    }
    return minResult;
}

Sphere::Sphere(const vector3& center, float radius) : center(center),
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

// Ray-Triangle交叉检测算法的Tomas Moller算法实现
// http://www.graphics.cornell.edu/pubs/1997/MT97.html
IntersectResult RayIntersectWithTriangle(
    const Ray& ray,
    const vector3& v0,
    const vector3& v1,
    const vector3& v2,
    Geometries* geo
) {
    const auto& ori = ray.origin;
    const auto& dir = ray.direction;
    // 假设点012为逆时针排列，视角在三角形上方往下看
    // 找到边1:点0到点1
    auto edge1 = v1 - v0;
    // 找到边2:点0到点2
    auto edge2 = v2 - v0;
    // 计算行列式，等于与射线方向和边2所在平面垂直的平面的法向量
    // 如果三角形012为逆时针，且射线为从上往下
    // 那么dir叉乘边2，右手定则，结果为点1点2射线方向
    auto pvec = CrossProduct(dir, edge2);
    // 什么时候点乘结果为零呢？
    // 即射线与边2所在平面法向量与边1垂直的时候
    // 只有两种可能：射线方向与三角形平面平行
    auto det = DotProduct(edge1, pvec);

    // TRIANGLE_CULLING为1：表示只接受正面的相交
    // TRIANGLE_CULLING为0：表示正反面相交都可以
#if TRIANGLE_CULLING
    if (det < EPSILON) // 不相交，此时视角为从上到下
        return IntersectResult();

    // 如果视角为从下到上，则可能相交
    // 计算射线起点到点0的向量
    auto tvec = ori - v0;
    // 法向量与反向射线的点乘，判断是否同向
    auto u = DotProduct(tvec, pvec);
    // 如果小于零，那么点0到射线起点与法向量不同向
    // 意味着射线起点在边2远离点1的空间内，不相交
    // 如果大于det，tvec在法向量上的射影大于det在法向量上的射影
    // 意味着射线起点到边2的距离比点1到边2的距离还远，不相交
    // 这样就排除了两种可能，使得射线在三角形平面上的交点缩小至三角形012
    // 以及01左边空间、12右边空间内
    if (u < 0.0f || u > det)
        return IntersectResult();

    // 射线在三角形平面上的交点必须在起点与边1所在平面的右侧
    auto qvec = CrossProduct(tvec, edge1);
    auto v = DotProduct(dir, qvec);
    // 如果v小于零，则排除01左边空间
    // 如果u+v大于det，则排除12右边空间
    if (v < 0.0f || u + v > det)
        return IntersectResult();

    // 此时交点限于三角形内部
    // 计算射线的t
    auto t = DotProduct(edge2, qvec);
    auto inv_det = 1.0f / det;
    t *= inv_det;
    auto distance = t; // 得出t，即摄影机发出的光线到其与三角形的交点距离
    auto position = ray.Eval(distance); // 代入直线方程，得出交点位置
    auto normal = Normalize(CrossProduct(edge1, edge2)); // 法向量 = 边1和边2叉乘
    return IntersectResult(geo, distance, position, normal);
#else
    if (det > -EPSILON && det < EPSILON) // 视角与所在面平行
        return IntersectResult();
    auto inv_det = 1.0f / det;
    auto tvec = ori - v0;
    auto u = DotProduct(tvec, pvec);
    u *= inv_det;
    if (u < 0.0f || u > 1.0f)
        return IntersectResult();
    auto qvec = CrossProduct(tvec, edge1);
    auto v = DotProduct(dir, qvec);
    v *= inv_det;
    if (v < 0.0f || u + v > 1.0f)
        return IntersectResult();
    auto t = DotProduct(edge2, qvec);
    t *= inv_det;
    auto distance = t; // 得出t，即摄影机发出的光线到其与三角形的交点距离
    auto position = ray.Eval(distance); // 代入直线方程，得出交点位置
    auto normal = Normalize(CrossProduct(edge1, edge2)); // 法向量 = 边1和边2叉乘
    if (DotProduct(dir, normal) > 0) {
        normal = -normal;
    }
    return IntersectResult(geo, distance, position, normal);
#endif
}

IntersectResult TriCollection::Intersect(Ray ray)
{
    auto v = ray.origin - center;
    auto a0 = SquareMagnitude(v) - radius;
    auto DdotV = DotProduct(ray.direction, v);
    if (DdotV <= 0)
    {
        IntersectResult result;
        // 射线和包围盒有交点
        for (const auto& tri : tridx) {
            auto r2 = RayIntersectWithTriangle(ray,
                vertices[std::get<0>(tri)], vertices[std::get<1>(tri)], vertices[std::get<2>(tri)], this);
            if (r2.body) {
                if (result.body) {
                    if (result.distance > r2.distance)
                        result = r2;
                }
                else {
                    result = r2;
                }
            }
        }
        return result;
    }

    return IntersectResult(); // 失败，不相交
}

Cube::Cube(const vector3& center, const vector3& scale, float a, float b)
{
    // a 逆时针绕y轴
    // b 往y轴上面外侧旋转
    /*
        --== 使用右手坐标系 ==--
        [0] : (-1,-1,-1) 远左下
        [1] : (-1,+1,-1) 远左上
        [2] : (+1,-1,-1) 远右下
        [3] : (+1,+1,-1) 远右上
        [4] : (-1,-1,+1) 近左下
        [5] : (-1,+1,+1) 近左上
        [6] : (+1,-1,+1) 近右下
        [7] : (+1,+1,+1) 近右上
    */
    auto s = scale * 0.5f;
    for (int i = 0; i < 8; i++) {
        vertices.emplace_back(
            i & 1 ? 1.0f : -1.0f,
            i & 2 ? 1.0f : -1.0f,
            i & 4 ? 1.0f : -1.0f);
        vertices[i] *= s;
    }
    if (a != 0.0f || b != 0.0f) {
        auto r1 = vector3(0, 1, 0);
        auto r2 = Rotate(vector3(0, 0, 1), r1, a);
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i] = Rotate(r1, vertices[i], a);
            vertices[i] = Rotate(r2, vertices[i], b);
        }
    }
    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i] += center;
    }
    radius = sqrtf(SquareMagnitude(scale));
    static const int idx[][4] = {
        {0,1,3,2},
        {6,7,5,4},
        {4,5,1,0},
        {2,3,7,6},
        {7,3,1,5},
        {2,6,4,0}
    };
    static const int idx2[][3] = {
        {2,1,0},
        {0,3,2}
    };
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 2; j++) {
            tridx.emplace_back(idx[i][idx2[j][0]], idx[i][idx2[j][1]], idx[i][idx2[j][2]]);
        }
    }
}
