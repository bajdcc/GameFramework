#ifndef GEOMETRIES2D_H
#define GEOMETRIES2D_H
#include "Geometries.h"
#include "math/vector2.h"

class Geo2DShape;

// 相交测试
struct Geo2DResult
{
    Geo2DResult();
    Geo2DResult(const Geo2DShape* body, float distance, const vector2& position, const vector2& normal);

    const Geo2DShape* body{ nullptr };
    float distance{ 0.0f };
    vector2 position;
    vector2 normal;
};

// 2D平面对象基类
class Geo2DObject
{
public:
    Geo2DObject();
    virtual ~Geo2DObject();

    virtual Geo2DResult sample(vector2 ori, vector2 dst) const = 0;
};

// 2D平面对象操作符
class Geo2DOper : public Geo2DObject
{
public:
    Geo2DOper();
    ~Geo2DOper();

    /**
     * \brief 采样求交点
     * \param ori 直线起点
     * \param dir 直线方向
     * \return 相交情况
     */
    Geo2DResult sample(vector2 ori, vector2 dir) const override;

    enum OpType
    {
        t_none,
        t_intersect,
        t_union,
        t_subtract,
        t_complement,
    };

    OpType op{ t_none };
};

// 2D形状
class Geo2DShape : public Geo2DObject
{
public:
    enum ShapeType
    {
        t_none,
        t_circle,
        t_ngon,
        t_plane,
        t_capsule,
    };

    Geo2DShape(ShapeType shape, color L);
    ~Geo2DShape();

    ShapeType shape{ t_none };
    color L;

    virtual vector2 get_center() const = 0;
};

// 圆
class Geo2DCircle : public Geo2DShape
{
public:
    Geo2DCircle(float cx, float cy, float r, color L);
    ~Geo2DCircle();

    Geo2DResult sample(vector2 ori, vector2 dir) const override;

    vector2 get_center() const override;

    vector2 center;
    float r;
};

#endif // GEOMETRIES2D_H
