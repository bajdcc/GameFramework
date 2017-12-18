#include "stdafx.h"
#include "Geometries2D.h"

Geo2DResult::Geo2DResult()
{
}

Geo2DResult::Geo2DResult(const Geo2DShape* body, float distance,
    const vector2& position, const vector2& normal)
    : body(body), distance(distance), position(position), normal(normal)
{
}

Geo2DObject::Geo2DObject()
{
}

Geo2DObject::~Geo2DObject()
{
}

Geo2DOper::Geo2DOper(OpType op, std::shared_ptr<Geo2DObject> o1, std::shared_ptr<Geo2DObject> o2) : op(op), obj1(o1), obj2(o2)
{
}

Geo2DOper::~Geo2DOper()
{
}

Geo2DResult Geo2DOper::sample(vector2 ori, vector2 dst) const
{
    return Geo2DResult();
}

Geo2DShape::Geo2DShape(ShapeType shape, color L) : shape(shape), L(L)
{
}

Geo2DShape::~Geo2DShape()
{
}

Geo2DCircle::Geo2DCircle(float cx, float cy, float r, color L)
    : Geo2DShape(t_circle, L), center(cx, cy), r(r)
{
}

Geo2DCircle::~Geo2DCircle()
{
}

Geo2DResult Geo2DCircle::sample(vector2 ori, vector2 dir) const
{
    // 圆上点x满足： || 点x - 圆心center || = 圆半径radius
    // 光线方程 r(t) = o + t.d (t>=0)
    // 代入得 || o + t.d - c || = r
    // 令 v = o - c，则 || v + t.d || = r

    // 化简求 t = - d.v - sqrt( (d.v)^2 + (v^2 - r^2) )  (求最近点)

    // 令 v = origin - center
    auto v = ori - center;

    // a0 = (v^2 - r^2)
    auto a0 = SquareMagnitude(v) - r;

    // DdotV = d.v
    auto DdotV = DotProduct(dir, v);

    //if (DdotV <= 0)
    {
        // 点乘测试相交，为负则同方向

        auto discr = (DdotV * DdotV) - a0; // 平方根中的算式

        if (discr >= 0)
        {
            // 非负则方程有解，相交成立
            // r(t) = o + t.d
            auto distance = -DdotV - sqrtf(discr); // 得出t，即摄影机发出的光线到其与圆的交点距离
            auto position = ori + dir * distance; // 代入直线方程，得出交点位置
            auto normal = Normalize(position - center); // 法向量 = 光线终点(球面交点) - 球心坐标
            return Geo2DResult(this, distance, position, normal);
        }
    }

    return Geo2DResult(); // 失败，不相交
}

vector2 Geo2DCircle::get_center() const
{
    return center;
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::intersect(Geo2DObjPtr s1, Geo2DObjPtr s2)
{
    return std::make_shared<Geo2DOper>(Geo2DOper::t_intersect, s1, s2);
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::new_circle(float cx, float cy, float r, color L)
{
    return std::make_shared<Geo2DCircle>(cx, cy, r, L);
}
