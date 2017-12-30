#ifndef GEOMETRIES2D_H
#define GEOMETRIES2D_H
#include "Geometries.h"
#include "math/vector2.h"

class Geo2DShape;

// 点信息
struct Geo2DPoint
{
    Geo2DPoint() = default;
    Geo2DPoint(float distance, const vector2& position, const vector2& normal);

    const Geo2DPoint& operator = (const Geo2DPoint& r);

    float distance{ FLT_MAX };
    vector2 position;
    vector2 normal;
};

// 相交测试
struct Geo2DResult
{
    Geo2DResult() = default;
    Geo2DResult(const Geo2DShape* body, bool inside, Geo2DPoint min_pt, Geo2DPoint max_pt);

    Geo2DResult(const Geo2DResult& r);
    const Geo2DResult& operator = (const Geo2DResult& r);

    const Geo2DShape* body{ nullptr };
    bool inside{ false };
    Geo2DPoint min_pt, max_pt;
};

// 2D平面对象基类
class Geo2DObject
{
public:
    Geo2DObject() = default;
    virtual ~Geo2DObject() = default;

    virtual Geo2DResult sample(vector2 ori, vector2 dst) const = 0;
};

// 2D平面对象操作符
class Geo2DOper : public Geo2DObject
{
public:
    enum OpType
    {
        t_none,
        t_intersect,
        t_union,
        t_subtract,
        t_complement,
    };

    Geo2DOper(OpType op, std::shared_ptr<Geo2DObject> o1, std::shared_ptr<Geo2DObject> o2);
    ~Geo2DOper() = default;

    /**
     * \brief 采样求交点
     * \param ori 直线起点
     * \param dir 直线方向
     * \return 相交情况
     */
    Geo2DResult sample(vector2 ori, vector2 dir) const override;

    OpType op{ t_none };

    std::shared_ptr<Geo2DObject> obj1, obj2;
};

// 2D形状
class Geo2DShape : public Geo2DObject
{
public:
    enum ShapeType
    {
        t_none,
        t_circle,
        t_box,
        t_triangle,
        t_ngon,
        t_plane,
        t_capsule,
    };

    Geo2DShape(ShapeType shape, color L, color R, float eta, color S);
    ~Geo2DShape() = default;

    ShapeType shape{ t_none };
    color L, R, S;
    float eta;
    bool angle{ false }; // 角度限制
    vector2 A1, A2;

    virtual vector2 get_center() const = 0;
};

// 圆
class Geo2DCircle : public Geo2DShape
{
public:
    Geo2DCircle(float cx, float cy, float r, color L, color R, float eta, color S);
    ~Geo2DCircle() = default;

    Geo2DResult sample(vector2 ori, vector2 dir) const override;

    vector2 get_center() const override;

    vector2 center;
    float r, rsq;
};

// 矩形
class Geo2DBox : public Geo2DShape
{
public:
    Geo2DBox(float cx, float cy, float sx, float sy, float theta, color L, color R, float eta, color S);
    ~Geo2DBox() =default;

    Geo2DResult sample(vector2 ori, vector2 dir) const override;

    vector2 get_center() const override;

    vector2 center;
    vector2 s;
    float theta, costheta, sintheta;
};

// 三角形
class Geo2DTriangle : public Geo2DShape
{
public:
    Geo2DTriangle(vector2 p1, vector2 p2, vector2 p3, color L, color R, float eta, color S);
    ~Geo2DTriangle() = default;

    Geo2DResult sample(vector2 ori, vector2 dir) const override;

    vector2 get_center() const override;

    vector2 center, p1, p2, p3;
    vector2 n[3];
};

#define Geo2DAngle Geo2DFactory::angle

#define Geo2DSub Geo2DFactory::sub
#define Geo2DOr Geo2DFactory::or
#define Geo2DAnd Geo2DFactory::and

#define Geo2DNewCircle Geo2DFactory::new_circle
#define Geo2DNewBox Geo2DFactory::new_box
#define Geo2DNewTriangle Geo2DFactory::new_triangle


class Geo2DFactory
{
public:
    using Geo2DObjPtr = std::shared_ptr<Geo2DObject>;
    using Geo2DShapePtr = std::shared_ptr<Geo2DShape>;

    static Geo2DObjPtr angle(Geo2DShapePtr s, float a1, float a2);

    static Geo2DObjPtr and(Geo2DObjPtr s1, Geo2DObjPtr s2);
    static Geo2DObjPtr or(Geo2DObjPtr s1, Geo2DObjPtr s2);
    static Geo2DObjPtr sub(Geo2DObjPtr s1, Geo2DObjPtr s2);

    static Geo2DShapePtr new_circle(float cx, float cy, float r, color L, color R = color(), float eta = 1.0f, color S = color());
    static Geo2DShapePtr new_box(float cx, float cy, float sx, float sy, float theta, color L, color R = color(), float eta = 1.0f, color S = color());
    static Geo2DShapePtr new_triangle(vector2 p1, vector2 p2, vector2 p3, color L, color R = color(), float eta = 1.0f, color S = color());
};

#endif // GEOMETRIES2D_H
