//
// Project: mice2d
// Created by bajdcc
//

#ifndef MICE2D_MICEATOM_H
#define MICE2D_MICEATOM_H

#include <random>
#include <ui/gdi/Gdi.h>
#include <base\pe2d\math\vector2.h>

using decimal = float;

namespace mice2d {

    struct DrawBag {
        CComPtr<ID2D1SolidColorBrush> brush;
        CComPtr<ID2D1PathGeometry> tail;
        std::default_random_engine random;
    };

    class MiceAtom {
    public:
        MiceAtom();

        void tick(decimal dt, const CRect& bounds, DrawBag& bag);
        void draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, DrawBag& bag);

        int id{ 0 };
        vector2 pt;
        decimal angle{ 0 };
        decimal angleF{ 0 };
        decimal speed{ 0 };
        decimal speedF{ 0 };
        decimal angleToCenter{ 0 };
        CColor bodyF;
        CComPtr<ID2D1SolidColorBrush> body;
        bool collide{ false };
        int needReturn{ 0 };
        decimal mouseEyeDirection{ 0 };
    };
}

#endif //MICE2D_MICEATOM_H
