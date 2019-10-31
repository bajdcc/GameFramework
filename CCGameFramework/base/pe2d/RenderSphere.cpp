#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "Geometries.h"

void PhysicsEngine::RenderSphereIntern(BYTE* buffer, BYTE* buffer2, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    PerspectiveCamera camera(
        vector3(0, 10, 10) + bag3d.camera_pos,  // 摄影机眼睛的位置
        bag3d.rotate_front,                     // 视角中向前方向的单位向量
        bag3d.rotate_up,                        // 视角中向上方向的单位向量
        bag3d.fov,                              // FOV
        0.5f * width / height);                 // 屏幕长宽比

    auto maxDepth = 20;       // 最大深度

    // -------------------------------------
    // 几何体集合
    World world;

    // -------------------------------------
    // 球体
    world.AddGeometries(std::make_shared<Sphere>(
        vector3(0.0f, 10.0f, -10.0f) + bag3d.sphere_pos, // 球心坐标
        10.0f                                            // 半径
        ));

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
            auto ray = camera.GenerateRay(sx, sy);

            // 测试光线与球是否相交
            auto result = world.Intersect(ray);
            if (result.body)
            {
                auto depth = 255 - fmin((result.distance / maxDepth) * 255.0f, 255.0f);
                // 输出灰阶
                buffer[0] = (BYTE)depth;
                buffer[1] = (BYTE)depth;
                buffer[2] = (BYTE)depth;
                buffer[3] = 255;
                buffer2[0] = (BYTE)((result.normal.x + 1) * 128); // 法向[-1,1]映射至[0,255]
                buffer2[1] = (BYTE)((result.normal.y + 1) * 128);
                buffer2[2] = (BYTE)((result.normal.z + 1) * 128);
                buffer2[3] = 255;
            }
            else
            {
                // 没有接触，就是背景色
                buffer[0] = 0;
                buffer[1] = 0;
                buffer[2] = 0;
                buffer[3] = 255;
                buffer2[0] = 0;
                buffer2[1] = 0;
                buffer2[2] = 0;
                buffer2[3] = 255;
            }

            buffer += 4;
            buffer2 += 4;
        }
    }
}
