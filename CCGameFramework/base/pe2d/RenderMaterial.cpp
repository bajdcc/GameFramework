#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "Geometries.h"

void PhysicsEngine::RenderMaterialIntern(BYTE * buffer, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 5, 15),    // 摄影机眼睛的位置
        vector3(0, 0, -1),    // 视角中向前方向的单位向量
        vector3(0, 1, 0),     // 视角中向上方向的单位向量
        90.0f);               // FOV

    auto maxDepth = 20;       // 最大深度

    // -------------------------------------
    // 几何体集合
    World world;

    // -------------------------------------
    // 平面
    auto plane = std::make_shared<Plane>(
        vector3(0.0f, 1.0f, 0.0f),     // 球心坐标
        0.0f                           // 半径
        );
    plane->material = std::make_shared<CheckerMaterial>(0.1f, 0.0f);
    world.Add(plane);

    // -------------------------------------
    // 球体
    auto sphere1 = std::make_shared<Sphere>(
        vector3(-10.0f, 10.0f, -10.0f), // 球心坐标
        10.0f                           // 半径
    );
    sphere1->material = std::make_shared<PhongMaterial>(color(Gdiplus::Color::Red), color(Gdiplus::Color::White), 16.0f, 0.0f);
    world.Add(sphere1);
    auto sphere2 = std::make_shared<Sphere>(
        vector3(10.0f, 10.0f, -10.0f),  // 球心坐标
        10.0f                           // 半径
    );
    sphere2->material = std::make_shared<PhongMaterial>(color(Gdiplus::Color::Blue), color(Gdiplus::Color::White), 16.0f, 0.0f);
    world.Add(sphere2);

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
            auto result = world.Intersect(ray);
            if (result.body)
            {
                // 采样
                auto color = result.body->material->Sample(ray, result.position, result.normal);
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
