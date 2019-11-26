//
// Project: mice2d
// Created by bajdcc
//

#include "stdafx.h"
#include "MiceAtom.h"
#include <random>

namespace mice2d {

    MiceAtom::MiceAtom()
    {

    }

    static decimal normalizeAngle(decimal angle)
    {
        while (angle < 0)
            angle += (decimal)(M_PI * 2);
        while (angle > (decimal)(M_PI * 2))
            angle -= (decimal)(M_PI * 2);
        return angle;
    }

    void MiceAtom::tick(decimal dt, const CRect& bounds, DrawBag& bag)
    {
        std::uniform_int_distribution<int> dk{ 0, 10 };
        std::uniform_int_distribution<int> di{ 0, 1 };
        std::uniform_real_distribution<decimal> da{ 0, 0.002f };
        std::uniform_real_distribution<decimal> ds{ -0.5f, 0.5f };
        auto& e = bag.random;
        auto border = bounds.Size();
        auto ang = normalizeAngle(angle);
        angleToCenter = std::atan2(pt.y, pt.x);
        if (angleToCenter < 0)
            angleToCenter += (decimal)M_PI * 2.0f;
        angleToCenter += (decimal)M_PI;
        angleToCenter = normalizeAngle(angleToCenter);
        if (pt.x < -border.cx * 0.3f || pt.x > border.cx * 0.3f || pt.y < -border.cy * 0.3f || pt.y > border.cy * 0.3f) {
            if (needReturn == 0) {
                if (ang < M_PI_2 || ang > M_PI_2 + M_PI) {
                    needReturn = 1;
                }
                else {
                    needReturn = 2;
                }
            }
            if (needReturn == 1) {
                angleF += (ds(e) + 0.5f) * 0.25f;
            }
            else if(needReturn == 2) {
                angleF += (ds(e) - 0.5f) * 0.25f;
            }
            if (needReturn != 0) {
                if (normalizeAngle((decimal)M_PI * 2.0f + angleToCenter - angle) < M_PI_4) {
                    angleF = 0;
                }
            }
        }
        else {
            needReturn = 0;
            if (dk(e) == 0) {
                angleF = 0;
            }
            if (std::sin(angleF) < 0) {
                angleF += ds(e) * 0.025f;
            }
            else if (std::sin(angleF) > 0) {
                angleF += (ds(e) - 0.5f) * 0.25f;
            }
        }

        if (needReturn == 0 && dk(e) == 0) {
            if (di(e) == 1)
                angleF += da(e);
            else
                angleF -= da(e);
        }

        if (angleF > 0.25f) {
            angleF = 0.25f;
        }
        else if (angleF < -0.25f) {
            angleF = -0.25f;
        }

        speed += ds(e);
        angle += angleF * 0.8f;
        angle = normalizeAngle(angle);

        mouseEyeDirection = angleF;

        speedF = 5.0f + std::sin(speed);
        pt.x += speedF * std::cos(angle);
        pt.y += speedF * std::sin(angle);
    }

    void MiceAtom::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, DrawBag& bag)
    {
        auto center = bounds.CenterPoint();
        rt->SetTransform(
            D2D1::Matrix3x2F::Rotation(450.0f - angle * (float)M_1_PI * 180.0f) *
            D2D1::Matrix3x2F::Translation(pt.x + (decimal)center.x, -pt.y + (decimal)center.y)
            
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
        rt->FillEllipse({ {-6 + mouseEyeDirection, -15}, 2, 2 }, bag.brush);
        rt->FillEllipse({ {6 + mouseEyeDirection, -15}, 2, 2 }, bag.brush);
        // 耳朵
        bag.brush->SetColor(collide ? D2D1::ColorF(D2D1::ColorF::Red) : D2D1::ColorF(0.5f, 0.5f, 0));
        rt->FillEllipse({ {-9, -4}, 8, 8 }, bag.brush);
        rt->FillEllipse({ {9, -4}, 8, 8 }, bag.brush);
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        rt->DrawEllipse({ {-9, -4}, 8, 8 }, bag.brush);
        rt->DrawEllipse({ {9, -4}, 8, 8 }, bag.brush);
        // 尾巴
        bag.brush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        rt->DrawGeometry(bag.tail, bag.brush);
        rt->SetTransform(D2D1::Matrix3x2F::Identity());
    }
}
