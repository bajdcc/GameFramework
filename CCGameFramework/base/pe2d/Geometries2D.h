#ifndef GEOMETRIES2D_H
#define GEOMETRIES2D_H
#include "math/vector3.h"
#include "Geometries.h"

// 光线
class Ray2D
{
public:
    Ray2D(const vector3& origin, const vector3& direction);

    vector3 Eval(float t) const; // 依方程 r(t)=o+t.d 求出解

    vector3 origin;    // 起点
    vector3 direction; // 方向
};

// 透视摄影机
class Camera2D
{
public:
    Camera2D(vector3 eye, vector3 front, vector3 up, float scale);

    Ray2D GenerateRay(float x, float y) const; // 产生追踪光线

    vector3 eye;     // 摄影机眼睛的位置
    vector3 front;   // 视角中向前方向的单位向量
    vector3 up;      // 视角中向上方向的单位向量
    vector3 right;   // 视角中向右方向的单位向量
    float scale{ 0.0f };
};

class Geometries2D;

// 相交测试结果
class IntersectResult2D
{
public:
    IntersectResult2D();
    IntersectResult2D(Geometries2D* body, float distance, const vector3& position, const vector3& normal);

    Geometries2D* body{ nullptr }; // 相交的几何体
    float distance{ 0.0f };        // 距离
    vector3 position;              // 位置
    vector3 normal;                // 法向量
};

// 材质接口
class Material2D
{
public:
    Material2D();
    virtual ~Material2D();
    virtual color Sample(Ray2D ray, vector3 position, vector3 normal) = 0;
};

// 纯色材质
class ColorMaterial2D : public Material2D
{
public:
    ColorMaterial2D(color L);

    color Sample(Ray2D ray, vector3 position, vector3 normal) override;

    color L;
    float scale;
};

// 光源采样
class LightSample2D
{
public:
    LightSample2D();
    LightSample2D(vector3 L, color EL);

    bool empty() const;

    vector3 L;
    color EL;
};

class World2D;

// 光源采样接口
class Light2D
{
public:
    virtual ~Light2D();
    virtual LightSample2D Sample(World2D& world, vector3 position) = 0;

    bool shadow{ true }; // 默认有阴影
};

// 点光源
class PointLight2D : public Light2D
{
public:
    PointLight2D(color intensity, vector3 position);

    LightSample2D Sample(World2D& world, vector3 position) override;

    color intensity;     // 幅射强度
    vector3 position;    // 光源位置
};

// 几何体接口
class Geometries2D
{
public:
    virtual ~Geometries2D();
    virtual IntersectResult2D Intersect(Ray2D ray) = 0;

    std::shared_ptr<Material2D> material;
};

// 几何体集合
class World2D
{
public:
    void AddGeometries(std::shared_ptr<Geometries2D> body);
    void AddLight(std::shared_ptr<Light2D> light);

    IntersectResult2D Intersect(Ray2D ray); // 相交测试
    
    std::vector<std::shared_ptr<Geometries2D>> geometries;
    std::vector<std::shared_ptr<Light2D>> lights;
};

// 圆形
class Circle2D : public Geometries2D
{
public:
    Circle2D(const vector3& center, float radius);

    IntersectResult2D Intersect(Ray2D ray) override; // 相交测试

    vector3 center;     // 球心坐标
    float radius;       // 半径
    float radiusSquare; // 缓存半径平方
};

#endif // GEOMETRIES2D_H
