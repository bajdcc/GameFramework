#include "stdafx.h"
#include "Geometries2D.h"

Geo2DResult::Geo2DResult()
{
}

Geo2DResult::Geo2DResult(const Geo2DShape* body, bool inside, float distance, float distance2,
    const vector2& position, const vector2& normal)
    : body(body), inside(inside), distance(distance), distance2(distance2), position(position), normal(normal)
{
}

Geo2DResult::Geo2DResult(const Geo2DResult& r)
    : body(r.body), inside(r.inside), distance(r.distance), distance2(r.distance2), position(r.position), normal(r.normal)

{
}

const Geo2DResult& Geo2DResult::operator=(const Geo2DResult& r)
{
    body = r.body;
    inside = r.inside;
    distance = r.distance;
    distance2 = r.distance2;
    position = r.position;
    normal = r.normal;
    return *this;
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
    if (op == t_union)
    {
        const auto r1 = obj1->sample(ori, dst);
        const auto r2 = obj2->sample(ori, dst);
        return r1.distance < r2.distance ? r1 : r2;
    }
    else if (op == t_intersect)
    {
        const auto r1 = obj1->sample(ori, dst);
        if (r1.body)
        {
            const auto r2 = obj2->sample(ori, dst);
            if (r2.body)
            {
                const auto rd = ((r1.inside ? 1 : 0) << 1) | (r2.inside ? 1 : 0);
                switch (rd)
                {
                case 0: // not(A or B)
                    if (r1.distance2 > r2.distance || r2.distance2 < r1.distance)
                        return r1.distance2 > r2.distance ? r2 : r1;
                    if (r1.distance < r2.distance && r2.distance2 < r1.distance2)
                        return r2;
                    if (r2.distance < r1.distance || r1.distance2 < r2.distance2)
                        return r1;
                    return Geo2DResult();
                case 1: // B
                    if (r1.distance < r2.distance2)
                        return r1;
                    return Geo2DResult();
                case 2: // A
                    if (r2.distance < r1.distance2)
                        return r1;
                    return Geo2DResult();
                case 3: // A and B
                    return r1.distance > r2.distance ? r1 : r2;
                }
            }
        }
    }
    else if (op == t_subtract)
    {
        const auto r1 = obj1->sample(ori, dst);
        const auto r2 = obj2->sample(ori, dst);
        const auto rd = ((r1.body ? 1 : 0) << 1) | (r2.body ? 1 : 0);
        switch (rd)
        {
        case 0: // not(A or B)
            return Geo2DResult();
        case 1: // B
            return Geo2DResult();
        case 2: // A
            return r1;
        case 3: // A and B
            if (r2.inside)
            {
                if (r1.distance2 > r2.distance2)
                {
                    auto r(r2);
                    r.body = r1.body;
                    r.inside = false;
                    r.distance = r.distance2;
                    return r;
                }
                return Geo2DResult();
            }
            if (r1.inside)
            {
                return r1;
            }
            if (r2.distance < r1.distance)
            {
                if (r1.distance2 < r2.distance2)
                {
                    return Geo2DResult();
                }
                auto r(r2);
                r.body = r1.body;
                r.inside = false;
                r.distance = r.distance2;
                return r;
            }
            return r1;
        }
    }
    return Geo2DResult();
}

Geo2DShape::Geo2DShape(ShapeType shape, color L) : shape(shape), L(L)
{
}

Geo2DShape::~Geo2DShape()
{
}

Geo2DCircle::Geo2DCircle(float cx, float cy, float r, color L)
    : Geo2DShape(t_circle, L), center(cx, cy), r(r), rsq(r*r)
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
    auto a0 = SquareMagnitude(v) - rsq;

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
            auto distance2 = -DdotV + sqrtf(discr);
            auto position = ori + dir * distance; // 代入直线方程，得出交点位置
            auto normal = Normalize(position - center); // 法向量 = 光线终点(球面交点) - 球心坐标
            if (a0 <= 0 || distance >= 0)
                return Geo2DResult(this, a0 <= 0, distance, distance2, position, normal);
        }
    }

    return Geo2DResult(); // 失败，不相交
}

vector2 Geo2DCircle::get_center() const
{
    return center;
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::and (Geo2DObjPtr s1, Geo2DObjPtr s2)
{
    return std::make_shared<Geo2DOper>(Geo2DOper::t_intersect, s1, s2);
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::or (Geo2DObjPtr s1, Geo2DObjPtr s2)
{
    return std::make_shared<Geo2DOper>(Geo2DOper::t_union, s1, s2);
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::sub(Geo2DObjPtr s1, Geo2DObjPtr s2)
{
    return std::make_shared<Geo2DOper>(Geo2DOper::t_subtract, s1, s2);
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::new_circle(float cx, float cy, float r, color L)
{
    return std::make_shared<Geo2DCircle>(cx, cy, r, L);
}
