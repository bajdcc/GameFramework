#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "Geometries.h"

void PhysicsEngine::RenderDirectionalLight(BYTE* buffer, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 10, 10),   // 摄影机眼睛的位置
        vector3(0, 0, -1),    // 视角中向前方向的单位向量
        vector3(0, 1, 0),     // 视角中向上方向的单位向量
        90.0f);               // FOV

    // 最大深度
    const auto maxDepth = 20;

    // -------------------------------------
    // 几何体集合
    World world;

    // -------------------------------------
    // 平面
    auto plane1 = std::make_shared<Plane>(
        vector3(0.0f, 1.0f, 0.0f),
        0.0f
        );
    world.AddGeometries(plane1);
    auto plane2 = std::make_shared<Plane>(
        vector3(0.0f, 0.0f, 1.0f),
        -50.0f
        );
    world.AddGeometries(plane2);
    auto plane3 = std::make_shared<Plane>(
        vector3(1.0f, 0.0f, 0.0f),
        -20.0f
        );
    world.AddGeometries(plane3);

    // -------------------------------------
    // 球体
    auto sphere1 = std::make_shared<Sphere>(
        vector3(0.0f, 10.0f, -10.0f),
        10.0f
        );
    world.AddGeometries(sphere1);

    // -------------------------------------
    // 平行光
    auto light = std::make_shared<DirectionalLight>(
        color(Gdiplus::Color::White),   // 颜色
        vector3(-1.75f, -2.0f, -1.5f)   // 朝向
        );
    world.AddLight(light);

    // -------------------------------------
    // 光线追踪
    for (auto y = 0; y < height; y++)
    {
        const auto sy = 1.0f - (1.0f * y / height);

        for (auto x = 0; x < width; x++)
        {
            const auto sx = 1.0f * x / width;

            // sx和sy将屏幕投影到[0,1]区间

            // 产生光线
            const auto ray = camera.GenerateRay(sx, sy);

            // 测试光线与球是否相交
            auto result = world.Intersect(ray);
            if (result.body)
            {
                color color;
                for (auto & k : world.lights) {
                    auto lightSample = k->Sample(world, result.position);

                    if (!lightSample.empty()) {
                        auto NdotL = DotProduct(result.normal, lightSample.L); // 计算角度

                        // 夹角为锐角，光源在平面前面
                        if (NdotL >= 0)
                            // 累计所有光线
                            // NdotL 就是光源方向在法向量上的投影
                            color = color + (lightSample.EL * NdotL);
                    }
                }
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

void PhysicsEngine::RenderPointLight(BYTE* buffer, cint width, cint height)
{
}

void PhysicsEngine::RenderSpotLight(BYTE* buffer, cint width, cint height)
{
}

void PhysicsEngine::RenderTriLight(BYTE* buffer, cint width, cint height)
{
}
