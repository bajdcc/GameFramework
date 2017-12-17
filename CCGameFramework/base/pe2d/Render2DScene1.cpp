#include "stdafx.h"
#include "PhysicsEngine2D.h"
#include "render/Direct2DRenderTarget.h"
#include "Geometries2D.h"

void PhysicsEngine::Render2DScene1(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderSingleBitmap2D(rt, bounds, Render2DScene1Intern);
}

void PhysicsEngine::Render2DScene1Intern(BYTE* buffer, cint width, cint height)
{
    // -------------------------------------
    // 摄影机
    const Camera2D camera(
        vector3(0.0f, 0.0f, 0.0f),
        vector3(0.0f, 0.0f, -1.0f),
        vector3(0.0f, 1.0f, 0.0f),
        40.0f);

    // -------------------------------------
    // 几何体集合
    World2D world;

    // -------------------------------------
    // 几何体
    const auto circle1 = std::make_shared<Circle2D>(
        vector3(0.0f, 0.0f, 0.0f),
        6.0f
        );
    circle1->material = std::make_shared<ColorMaterial2D>(color(Gdiplus::Color::Red));
    world.AddGeometries(circle1);
    const auto circle2 = std::make_shared<Circle2D>(
        vector3(25.0f, -10.0f, 0.0f),
        6.0f
        );
    circle2->material = std::make_shared<ColorMaterial2D>(color(Gdiplus::Color::Blue));
    world.AddGeometries(circle2);

    // -------------------------------------
    // 光源
    const auto light1 = std::make_shared<PointLight2D>(
        color(0.8f, 0.9f, 0.8f) * 2000.0f,
        vector3(60.0f, 40.0f, 0.0f)
        );
    world.AddLight(light1);
    const auto light2 = std::make_shared<PointLight2D>(
        color(1.0f, 1.0f, 1.0f) * 2000.0f,
        vector3(-80.0f, 0.0f, 0.0f)
        );
    world.AddLight(light2);

    // -------------------------------------
    // 光线追踪
    const auto m = min(width, height);
    for (auto y = 0; y < height; y++)
    {
        const auto sy = 1.0f - (1.0f * y / m);
        for (auto x = 0; x < width; x++)
        {
            const auto sx = 1.0f * x / m;
            const auto ray = camera.GenerateRay(sx, sy);
            auto result = world.Intersect(ray);
            color color;
            if (result.body)
            {
                for (auto & k : world.lights) {
                    auto lightSample = k->Sample(world, result.position);

                    if (!lightSample.empty()) {
                        auto NdotL = DotProduct(result.normal, lightSample.L);

                        if (NdotL >= 0)
                        {
                            auto meter = result.body->material->Sample(ray, result.position, result.normal);;
                            color = color + (lightSample.EL * NdotL * meter);
                        }
                    }
                }
            }
            else
            {
                auto pos = ray.origin;
                pos.z = 0;
                for (auto & k : world.lights) {
                    auto lightSample = k->Sample(world, pos);

                    if (!lightSample.empty()) {
                        color = color + lightSample.EL;
                    }
                }
            }
            buffer[0] = BYTE(fminf(color.b, 1.0f) * 255.0f);
            buffer[1] = BYTE(fminf(color.g, 1.0f) * 255.0f);
            buffer[2] = BYTE(fminf(color.r, 1.0f) * 255.0f);
            buffer[3] = 255;
            buffer += 4;
        }
    }
}
