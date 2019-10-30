#ifndef GEOMETRIES_H
#define GEOMETRIES_H
#include "math/vector3.h"

// 光线
class Ray
{
public:
    Ray(const vector3& origin, const vector3& direction);

    vector3 Eval(float t) const; // 依方程 r(t)=o+t.d 求出解

    vector3 origin;    // 起点
    vector3 direction; // 方向
};

// 透视摄影机
class PerspectiveCamera
{
public:
    PerspectiveCamera(vector3 eye, vector3 front, vector3 up, float fov, float asp);

    Ray GenerateRay(float x, float y) const; // 产生追踪光线

    vector3 eye;     // 摄影机眼睛的位置
    vector3 front;   // 视角中向前方向的单位向量
    vector3 up;      // 视角中向上方向的单位向量
    vector3 right;   // 视角中向右方向的单位向量
    float fov;       // 视角
    float fovScale;  // 视角缩放
    float asp;       // 长宽比
};

class Geometries;

// 相交测试结果
class IntersectResult
{
public:
    IntersectResult();
    IntersectResult(Geometries* body, float distance, const vector3& position, const vector3& normal);

    Geometries* body{ nullptr };   // 相交的几何体
    float distance{ 0.0f };        // 距离
    vector3 position;              // 位置
    vector3 normal;                // 法向量
};

// 颜色
class color
{
public:
    color();
    color(BYTE r, BYTE g, BYTE b);
    color(float r, float g, float b);
    color(Gdiplus::Color clr);

    color operator + (const color& c) const;
    color operator * (float s) const;
    color operator * (const color& c) const;

    void Add(const color& c);
    const bool Valid() const;
    void Set(float s);
    color Negative(float s);
    void Normalize();
    static color make_color(float s);
    static color make_color(float r, float g, float b);

    float r, g, b;
};

// 材质接口
class Material
{
public:
    Material(float reflectiveness);
    virtual ~Material();
    virtual color Sample(Ray ray, vector3 position, vector3 normal) = 0;

    float reflectiveness;
};

// 棋盘材质
class CheckerMaterial : public Material
{
public:
    CheckerMaterial(float scale, float reflectiveness);

    color Sample(Ray ray, vector3 position, vector3 normal) override;

    float scale;
};

// Phong材质
class PhongMaterial : public Material
{
public:
    PhongMaterial(color diffuse, color specular, float shininess, float reflectiveness);

    color Sample(Ray ray, vector3 position, vector3 normal) override;

    color diffuse;
    color specular;
    float shininess;
};

// 光源采样
class LightSample
{
public:
    LightSample();
    LightSample(vector3 L, color EL);

    bool empty() const;

    vector3 L;
    color EL;
};

class World;

// 光源采样接口
class Light
{
public:
    virtual ~Light();
    virtual LightSample Sample(World& world, vector3 position) = 0;

    bool shadow{ true }; // 默认有阴影
};

// 平行光
class DirectionalLight : public Light
{
public:
    DirectionalLight(color irradiance, vector3 direction);

    LightSample Sample(World& world, vector3 position) override;

    color irradiance;    // 幅照度
    vector3 direction;   // 光照方向
    vector3 L;           // 光源方向
};

// 点光源
class PointLight : public Light
{
public:
    PointLight(color intensity, vector3 position);

    LightSample Sample(World& world, vector3 position) override;

    color intensity;     // 幅射强度
    vector3 position;    // 光源位置
};

// 聚光灯
class SpotLight : public Light
{
public:
    SpotLight(color intensity, vector3 position, vector3 direction, float theta, float phi, float falloff);

    LightSample Sample(World& world, vector3 position) override;

    color intensity;     // 幅射强度
    vector3 position;    // 光源位置
    vector3 direction;   // 光照方向
    float theta;         // 内圆锥的内角
    float phi;           // 外圆锥的内角
    float falloff;       // 衰减

    /* 以下为预计算常量 */
    vector3 S;           // 光源方向
    float cosTheta;      // cos(内圆锥角)
    float cosPhi;        // cos(外圆锥角)
    float baseMultiplier;// 1/(cosTheta-cosPhi)
};

// 几何体接口
class Geometries
{
public:
    virtual ~Geometries();
    virtual IntersectResult Intersect(Ray ray) = 0;

    std::shared_ptr<Material> material;
};

// 几何体集合
class World
{
public:
    void AddGeometries(std::shared_ptr<Geometries> body);
    void AddLight(std::shared_ptr<Light> light);

    IntersectResult Intersect(Ray ray); // 相交测试
    
    std::vector<std::shared_ptr<Geometries>> geometries;
    std::vector<std::shared_ptr<Light>> lights;
};

// 球体
class Sphere : public Geometries
{
public:
    Sphere(const vector3& center, float radius);

    IntersectResult Intersect(Ray ray) override; // 相交测试

    vector3 center;     // 球心坐标
    float radius;       // 半径
    float radiusSquare; // 缓存半径平方
};

// 平面
class Plane : public Geometries
{
public:
    Plane(const vector3& normal, float d);

    IntersectResult Intersect(Ray ray) override; // 相交测试

    vector3 normal;     // 单位法向量
    vector3 position;   // 原点到平面最短距离之交点坐标
    float d;            // 原点到平面最短距离 normal.x = d
};

#endif // GEOMETRIES_H
