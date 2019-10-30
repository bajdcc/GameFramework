#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "Geometries.h"

void PhysicsEngine::RenderMaterialIntern(BYTE * buffer, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 5, 15) + bag3d.camera_pos,   // 摄影机眼睛的位置
        bag3d.rotate_front,                     // 视角中向前方向的单位向量
        bag3d.rotate_up,                        // 视角中向上方向的单位向量
        bag3d.fov,                              // FOV
        1.0f * width / height);                 // 屏幕长宽比

    const auto maxDepth = 20;       // 最大深度

    // -------------------------------------
    // 几何体集合
    World world;

    // -------------------------------------
    // 平面
    auto plane = std::make_shared<Plane>(
        vector3(0.0f, 1.0f, 0.0f),      // 距原点垂足坐标
        0.0f                            // 到原点最短距离
        );
    plane->material = std::make_shared<CheckerMaterial>(0.1f, 0.0f);
    world.AddGeometries(plane);

    // -------------------------------------
    // 球体
    auto sphere1 = std::make_shared<Sphere>(
        vector3(-10.0f, 10.0f, -10.0f), // 球心坐标
        10.0f                           // 半径
    );
    sphere1->material = std::make_shared<PhongMaterial>(color(Gdiplus::Color::Red), color(Gdiplus::Color::White), 16.0f, 0.0f);
    world.AddGeometries(sphere1);
    auto sphere2 = std::make_shared<Sphere>(
        vector3(10.0f, 10.0f, -10.0f),  // 球心坐标
        10.0f                           // 半径
    );
    sphere2->material = std::make_shared<PhongMaterial>(color(Gdiplus::Color::Blue), color(Gdiplus::Color::White), 16.0f, 0.0f);
    world.AddGeometries(sphere2);

    // -------------------------------------
    // 光线追踪
    for (auto y = 0; y < height; y++)
    {
        const auto sy = 1.0f - (1.0f * y / height);

        for (auto x = 0; x < width; x++)
        {
            const auto sx = (1.0f * x / width);

            // sx和sy将屏幕投影到[0,1]区间

            // 产生光线
            const auto ray = camera.GenerateRay(sx, sy);

            // 测试光线与球是否相交
            auto result = world.Intersect(ray);
            if (result.body)
            {
                // 采样
                const auto color = result.body->material->Sample(ray, result.position, result.normal);
                buffer[0] = BYTE(color.b * 255);
                buffer[1] = BYTE(color.g * 255);
                buffer[2] = BYTE(color.r * 255);
                buffer[3] = 255;
            }
            else
            {
                // 没有接触，就是背景色
                buffer[0] = 0;
                buffer[1] = 0;
                buffer[2] = 0;
                buffer[3] = 255;
            }

            buffer += 4;
        }
    }
}

void PhysicsEngine::RenderReflectIntern(BYTE* buffer, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 5, 15) + bag3d.camera_pos,   // 摄影机眼睛的位置
        bag3d.rotate_front,                     // 视角中向前方向的单位向量
        bag3d.rotate_up,                        // 视角中向上方向的单位向量
        bag3d.fov,                              // FOV
        1.0f * width / height);                 // 屏幕长宽比

    const auto maxDepth = 20;       // 最大深度
    const auto maxReflect = 3;      // 最大反射次数

    // -------------------------------------
    // 几何体集合
    World world;

    // -------------------------------------
    // 平面
    auto plane = std::make_shared<Plane>(
        vector3(0.0f, 1.0f, 0.0f),     // 球心坐标
        0.0f                           // 半径
        );
    plane->material = std::make_shared<CheckerMaterial>(0.1f, 0.5f);
    world.AddGeometries(plane);

    // -------------------------------------
    // 球体
    auto sphere1 = std::make_shared<Sphere>(
        vector3(-10.0f, 10.0f, -10.0f), // 球心坐标
        10.0f                           // 半径
        );
    sphere1->material = std::make_shared<PhongMaterial>(color(Gdiplus::Color::Red), color(Gdiplus::Color::White), 16.0f, 0.25f);
    world.AddGeometries(sphere1);
    auto sphere2 = std::make_shared<Sphere>(
        vector3(10.0f, 10.0f, -10.0f),  // 球心坐标
        10.0f                           // 半径
        );
    sphere2->material = std::make_shared<PhongMaterial>(color(Gdiplus::Color::Blue), color(Gdiplus::Color::White), 16.0f, 0.25f);
    world.AddGeometries(sphere2);

    // -------------------------------------
    // 光线追踪
    for (auto y = 0; y < height; y++)
    {
        const auto sy = 1.0f - (1.0f * y / height);

        for (auto x = 0; x < width; x++)
        {
            const auto sx = (1.0f * x / width);

            // sx和sy将屏幕投影到[0,1]区间

            // 产生光线
            const auto ray = camera.GenerateRay(sx, sy);

            // 测试光线与球是否相交
            const auto color = RenderReflectRecursive(world, ray, maxReflect);
            buffer[0] = BYTE(color.b * 255);
            buffer[1] = BYTE(color.g * 255);
            buffer[2] = BYTE(color.r * 255);
            buffer[3] = 255;

            buffer += 4;
        }
    }
}

color PhysicsEngine::RenderReflectRecursive(World& world, const Ray& ray, int maxReflect)
{
    static color black(Gdiplus::Color::Black);

    auto result = world.Intersect(ray);

    if (result.body) {
        // 参见 https://www.cnblogs.com/bluebean/p/5299358.html

        // 取得反射系数
        const auto reflectiveness = result.body->material->reflectiveness;

        // 先采样（取物体自身的颜色）
        auto color = result.body->material->Sample(ray, result.position, result.normal);

        // 加上物体自身的颜色成份（与反射的颜色相区分）
        color = color * (1.0f - reflectiveness);

        if (reflectiveness > 0 && maxReflect > 0) {

            // 公式 R = I - 2 * N * (N . I) ，求出反射光线
            const auto r = result.normal * (-2.0f * DotProduct(result.normal, ray.direction)) + ray.direction;

            // 以反射光线作为新的光线追踪射线
            const auto reflectedColor = RenderReflectRecursive(world, Ray(result.position, r), maxReflect - 1);

            // 加上反射光的成份
            color = color + (reflectedColor * reflectiveness);
        }
        return color;
    }
    return black;
}
