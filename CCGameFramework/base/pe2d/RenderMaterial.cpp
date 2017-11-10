#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "Geometries.h"

void PhysicsEngine::RenderMaterialIntern(BYTE * buffer, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 10, 10),   // 摄影机眼睛的位置
        vector3(0, 0, -1),    // 视角中向前方向的单位向量
        vector3(0, 1, 0),     // 视角中向上方向的单位向量
        90.0f);               // FOV

    auto maxDepth = 20;       // 最大深度

    // -------------------------------------
    // 几何体集合
    World world;

    // -------------------------------------
    // 球体
    world.Add(std::make_shared<Sphere>(
        vector3(0.0f, 10.0f, -10.0f), // 球心坐标
        10.0f                         // 半径
    ));

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
                auto depth = 255 - min((result.distance / maxDepth) * 255.0f, 255.0f);
                // 输出灰阶
                buffer[0] = (BYTE)depth;
                buffer[1] = (BYTE)depth;
                buffer[2] = (BYTE)depth;
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
