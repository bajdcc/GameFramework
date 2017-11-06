#include "stdafx.h"
#include "PhysicsEngine2D.h"

void PhysicsEngine::RenderSphereIntern(BYTE * buffer, cint width, cint height)
{
    // µ»Œ“–¥ÕÍ
    for (auto y = 0; y < height; y++)
    {
        for (auto x = 0; x < width; x++)
        {
            buffer[0] = BYTE(x * 255 / width);
            buffer[1] = 0;
            buffer[2] = BYTE(y * 255 / height);
            buffer[3] = 255;
            buffer += 4;
        }
    }
}