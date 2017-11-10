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
    PerspectiveCamera(vector3 eye, vector3 front, vector3 up, float fov);

    Ray GenerateRay(float x, float y) const; // 产生追踪光线

    vector3 eye;     // 摄影机眼睛的位置
    vector3 front;   // 视角中向前方向的单位向量
    vector3 up;      // 视角中向上方向的单位向量
    vector3 right;   // 视角中向右方向的单位向量
    float fov;       // 视角
    float fovScale;  // 视角缩放
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

// 几何体接口
class Geometries
{
public:
    virtual ~Geometries() {}
    virtual IntersectResult Intersect(Ray ray) = 0;
};

// 几何体集合
class World
{
public:
    void Add(std::shared_ptr<Geometries> body);

    IntersectResult Intersect(Ray ray); // 相交测试

    std::vector<std::shared_ptr<Geometries>> collections;
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

#endif // GEOMETRIES_H
