#include "stdafx.h"
#include "Geometries2D.h"

#define EPSILON 1.0e-6f

Geo2DPoint::Geo2DPoint()
{
}

Geo2DPoint::Geo2DPoint(float distance, const vector2& position, const vector2& normal)
    : distance(distance), position(position), normal(normal)
{
}

const Geo2DPoint& Geo2DPoint::operator=(const Geo2DPoint& r)
{
    distance = r.distance;
    position = r.position;
    normal = r.normal;
    return *this;
}

Geo2DResult::Geo2DResult()
{
}

Geo2DResult::Geo2DResult(const Geo2DShape* body, bool inside, Geo2DPoint min_pt, Geo2DPoint max_pt)
    : body(body), inside(inside), min_pt(min_pt), max_pt(max_pt)
{
}

Geo2DResult::Geo2DResult(const Geo2DResult& r)
    : body(r.body), inside(r.inside), min_pt(r.min_pt), max_pt(r.max_pt)
{
}

const Geo2DResult& Geo2DResult::operator=(const Geo2DResult& r)
{
    body = r.body;
    inside = r.inside;
    min_pt = r.min_pt;
    max_pt = r.max_pt;
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
        return r1.min_pt.distance < r2.min_pt.distance ? r1 : r2;
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
                    if (r1.min_pt.distance < r2.min_pt.distance)
                    {
                        if (r2.min_pt.distance > r1.max_pt.distance) // AABB
                            break;
                        if (r2.max_pt.distance < r1.max_pt.distance) // ABBA
                            return r2;
                        auto r(r2); // ABAB
                        r.max_pt = r1.max_pt;
                        return r;

                    }
                    if (r2.min_pt.distance < r1.min_pt.distance)
                    {
                        if (r1.min_pt.distance > r2.max_pt.distance) // BBAA
                            break;
                        if (r1.max_pt.distance < r2.max_pt.distance) // BAAB
                            return r1;
                        auto r(r1); // BABA
                        r.max_pt = r2.max_pt;
                        return r;
                    }
                    break;
                case 1: // B
                    if (r1.min_pt.distance < r2.max_pt.distance)
                    {
                        if (r1.max_pt.distance > r2.max_pt.distance) // ABA
                        {
                            auto r(r1);
                            r.max_pt = r2.max_pt;
                            return r;
                        }
                        else // AAB
                        {
                            auto r(r1);
                            r.max_pt = r1.max_pt;
                            return r;
                        }
                    }
                    break;
                case 2: // A
                    if (r2.min_pt.distance < r1.max_pt.distance)
                    {
                        if (r2.max_pt.distance > r1.max_pt.distance) // BAB
                        {
                            auto r(r2);
                            r.max_pt = r1.max_pt;
                            return r;
                        }
                        else // BBA
                        {
                            auto r(r2);
                            r.max_pt = r2.max_pt;
                            return r;
                        }
                    }
                    break;
                case 3: // A and B
                    if (r1.min_pt.distance > r2.min_pt.distance)
                    {
                        if (r1.max_pt.distance > r2.max_pt.distance) // BA
                        {
                            auto r(r2);
                            r.min_pt = r1.min_pt;
                            return r;
                        }
                        else // AB
                        {
                            return r1;
                        }
                    }
                    else
                    {
                        if (r2.max_pt.distance > r1.max_pt.distance) // AB
                        {
                            auto r(r1);
                            r.min_pt = r2.min_pt;
                            return r;
                        }
                        else // AB
                        {
                            return r2;
                        }
                    }
                default:
                    break;
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
            break;
        case 1: // B
            break;
        case 2: // A
            if (r1.inside) // AA
            {
                if (r2.max_pt.distance == FLT_MAX)
                    return r1;
                if (r1.min_pt.distance > r2.max_pt.distance)
                    return r1;
                auto r(r1);
                r.min_pt = r2.max_pt;
                r.min_pt.normal = -r.min_pt.normal;
                return r;
            }
            else
                return r1;
        case 3: // A and B
            if (r1.inside && r2.inside)
            {
                if (r2.max_pt.distance < r1.max_pt.distance) // BA
                {
                    auto r(r1);
                    r.min_pt = r2.max_pt;
                    r.min_pt.normal = -r.min_pt.normal;
                    r.inside = false;
                    return r;
                }
                else // AB
                {
                    break;
                }
            }
            else if (r2.inside)
            {
                if (r1.max_pt.distance > r2.max_pt.distance)
                {
                    if (r2.max_pt.distance > r1.min_pt.distance) // ABA
                    {
                        auto r(r1);
                        r.min_pt = r2.max_pt;
                        r.min_pt.normal = -r.min_pt.normal;
                        r.inside = false;
                        return r;
                    }
                    else // BAA
                    {
                        return r1;
                    }
                }
                else // AAB
                    break;
            }
            else if (r1.inside) // BAB
            {
                auto r(r1);
                r.max_pt = r2.min_pt;
                return r;
            }
            else
            {
                if (r1.min_pt.distance < r2.min_pt.distance)
                {
                    if (r2.min_pt.distance > r1.max_pt.distance) // AABB
                        return r1;
                    if (r2.max_pt.distance < r1.max_pt.distance) // ABBA
                        return r1;
                    auto r(r1); // ABAB
                    r.max_pt = r2.min_pt;
                    r.max_pt.normal = -r.max_pt.normal;
                    return r;
                }
                else
                {
                    if (r1.min_pt.distance > r2.max_pt.distance) // BBAA
                        return r1;
                    if (r1.max_pt.distance < r2.max_pt.distance) // BAAB
                        break;
                    auto r(r1); // BABA
                    r.min_pt = r2.max_pt;
                    r.min_pt.normal = -r.min_pt.normal;
                    return r;
                }
            }
        default:
            break;
        }
    }
    return Geo2DResult();
}

Geo2DShape::Geo2DShape(ShapeType shape, color L, color R, float eta, color S) : shape(shape), L(L), R(R), eta(eta), S(S)
{
}

Geo2DShape::~Geo2DShape()
{
}

Geo2DCircle::Geo2DCircle(float cx, float cy, float r, color L, color R, float eta, color S)
    : Geo2DShape(t_circle, L, R, eta, S), center(cx, cy), r(r), rsq(r*r)
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
            auto position2 = ori + dir * distance2;
            auto normal = Normalize(position - center); // 法向量 = 光线终点(球面交点) - 球心坐标
            auto normal2 = Normalize(position2 - center);
            return Geo2DResult((a0 <= 0 || distance >= 0) ? this : nullptr, a0 <= 0,
                Geo2DPoint(distance, position, normal),
                Geo2DPoint(distance2, position2, normal2));
        }
    }

    return Geo2DResult(); // 失败，不相交
}

vector2 Geo2DCircle::get_center() const
{
    return center;
}

Geo2DBox::Geo2DBox(float cx, float cy, float sx, float sy, float theta, color L, color R, float eta, color S)
    : Geo2DShape(t_circle, L, R, eta, S), center(cx, cy), s(sx, sy), theta(theta), costheta(cosf(theta)), sintheta(sinf(theta))
{
}

Geo2DBox::~Geo2DBox()
{
}

static int SignBit(const float& a)
{
    if (fabs(a) < EPSILON)
    {
        return 0;
    }
    return a > 0 ? 1 : -1;
}

static bool IntersectWithLineAB(const vector2& ori, const vector2& dir, const vector2& p1, const vector2& p2, float& t, vector2& p)
{
    const auto tAB1 = dir.y * (p2.x - p1.x) - dir.x * (p2.y - p1.y);
    if (fabs(tAB1) > EPSILON)
    {
        t = ((ori.x - p1.x) * (p2.y - p1.y) - (ori.y - p1.y) * (p2.x - p1.x)) / tAB1;
        p = ori + dir * t;
        return (SignBit(p1.x - p.x) == SignBit(p.x - p2.x)) && (SignBit(p1.y - p.y) == SignBit(p.y - p2.y));
    }
    return false;
}

Geo2DResult Geo2DBox::sample(vector2 ori, vector2 dir) const
{
    const auto _A = vector2(costheta * -s.x + sintheta * -s.y, costheta * -s.y - sintheta * -s.x);
    const auto _B = vector2(costheta * s.x + sintheta * -s.y, costheta * -s.y - sintheta * s.x);
    const auto A = center + _A;
    const auto B = center + _B;
    const auto C = center - _A;
    const auto D = center - _B;
    const vector2 pts[4] = { A,B,C,D };

    static int m[4][2] = { {0,1},{1,2},{2,3},{3,0} };
    float t[2];
    vector2 p[2];
    int ids[2];
    int cnt = 0;
    for (int i = 0; i < 4 && cnt < 2; i++)
    {
        if (IntersectWithLineAB(ori, dir, pts[m[i][0]], pts[m[i][1]], t[cnt], p[cnt]))
        {
            ids[cnt++] = i;
        }
    }
    if (cnt == 2)
    {
        const auto td = ((t[0] >= 0 ? 1 : 0) << 1) | (t[1] >= 0 ? 1 : 0);
        switch (td)
        {
        case 0: // 双反，无交点，在外
            break;
        case 1: // t[1]，有交点，在内
            return Geo2DResult(this, true,
                Geo2DPoint(t[0], p[0], Normalize(pts[m[ids[0]][0]] + pts[m[ids[0]][1]] - center - center)),
                Geo2DPoint(t[1], p[1], Normalize(pts[m[ids[1]][0]] + pts[m[ids[1]][1]] - center - center)));
        case 2: // t[0]，有交点，在内
            return Geo2DResult(this, true,
                Geo2DPoint(t[1], p[1], Normalize(pts[m[ids[1]][0]] + pts[m[ids[1]][1]] - center - center)),
                Geo2DPoint(t[0], p[0], Normalize(pts[m[ids[0]][0]] + pts[m[ids[0]][1]] - center - center)));
        case 3: // 双正，有交点，在外
            if (t[0] > t[1])
            {
                return Geo2DResult(this, false,
                    Geo2DPoint(t[1], p[1], Normalize(pts[m[ids[1]][0]] + pts[m[ids[1]][1]] - center - center)),
                    Geo2DPoint(t[0], p[0], Normalize(pts[m[ids[0]][0]] + pts[m[ids[0]][1]] - center - center)));
            }
            else
            {
                return Geo2DResult(this, false,
                    Geo2DPoint(t[0], p[0], Normalize(pts[m[ids[0]][0]] + pts[m[ids[0]][1]] - center - center)),
                    Geo2DPoint(t[1], p[1], Normalize(pts[m[ids[1]][0]] + pts[m[ids[1]][1]] - center - center)));
            }
        default:
            break;
        }
    }
    return Geo2DResult();
}

vector2 Geo2DBox::get_center() const
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

Geo2DFactory::Geo2DObjPtr Geo2DFactory::new_circle(float cx, float cy, float r, color L, color R, float eta, color S)
{
    return std::make_shared<Geo2DCircle>(cx, cy, r, L, R, eta, S);
}

Geo2DFactory::Geo2DObjPtr Geo2DFactory::new_box(float cx, float cy, float sx, float sy, float theta, color L, color R, float eta, color S)
{
    return std::make_shared<Geo2DBox>(cx, cy, sx, sy, theta, L, R, eta, S);
}
