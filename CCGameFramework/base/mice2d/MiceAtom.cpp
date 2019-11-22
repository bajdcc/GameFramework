//
// Project: mice2d
// Created by bajdcc
//

#include "stdafx.h"
#include "MiceAtom.h"

namespace mice2d {

    MiceAtom::MiceAtom()
    {

    }

    void MiceAtom::tick()
    {
    }

    void MiceAtom::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, DrawBag& bag)
    {
        auto center = bounds.CenterPoint();
        rt->SetTransform(
            D2D1::Matrix3x2F::Translation(pt.x + (decimal)center.x, pt.y + (decimal)center.y)
        );
        // 身体
        rt->FillEllipse({ { 0, 0 }, 10, 20 }, body);
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        rt->DrawEllipse({ { 0, 0 }, 10, 20 }, bag.brush);
        // 眼睛
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
        rt->FillEllipse({ {-6, -13}, 4, 4 }, bag.brush);
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        rt->DrawEllipse({ {-6, -13}, 4, 4 }, bag.brush);
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
        rt->FillEllipse({ {6, -13}, 4, 4 }, bag.brush);
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        rt->DrawEllipse({ {6, -13}, 4, 4 }, bag.brush);
        // 鼻子
        rt->FillEllipse({ {0, -20}, 2, 2 }, bag.brush);
        // 眼珠
        rt->FillEllipse({ {-6, -15}, 2, 2 }, bag.brush);
        rt->FillEllipse({ {6, -15}, 2, 2 }, bag.brush);
        // 耳朵
        bag.brush->SetColor(collide ? D2D1::ColorF(D2D1::ColorF::Red) : D2D1::ColorF(0.5f, 0.5f, 0));
        rt->FillEllipse({ {-9, -4}, 8, 8 }, bag.brush);
        rt->FillEllipse({ {9, -4}, 8, 8 }, bag.brush);
        // 尾巴
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        rt->DrawGeometry(bag.tail, bag.brush);
        rt->SetTransform(D2D1::Matrix3x2F::Identity());
    }
}
