//
// Project: mice2d
// Created by bajdcc
//

#ifndef MICE2D_MICEATOM_H
#define MICE2D_MICEATOM_H

#include <ui/gdi/Gdi.h>
#include <base\pe2d\math\vector2.h>

using decimal = float;

namespace mice2d {

    struct DrawBag {
        CComPtr<ID2D1SolidColorBrush> brush;
        CComPtr<ID2D1PathGeometry> tail;
    };

    class MiceAtom {
    public:
        MiceAtom();

        void tick();
        void draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, DrawBag& bag);

        vector2 pt;
        decimal angle;
        CColor bodyF;
        CComPtr<ID2D1SolidColorBrush> body;
        bool collide{ false };
    };
}

#endif //MICE2D_MICEATOM_H
