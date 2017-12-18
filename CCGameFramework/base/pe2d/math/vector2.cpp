#include "stdafx.h"
#include "vector2.h"

vector2::vector2()
{
    x = y = 0.0f;
}

vector2::vector2(FLOAT f)
{
    x = y = f;
}

vector2::vector2(const _vector2& v)
{
    x = v.x; y = v.y;
}

vector2::vector2(FLOAT _x, FLOAT _y)
{
    x = _x; y = _y;
}

vector2& vector2::operator += (const vector2& v)
{
    x += v.x; y += v.y;
    return *this;
}

vector2& vector2::operator -= (const vector2& v)
{
    x -= v.x; y -= v.y;
    return *this;
}

vector2& vector2::operator *= (const vector2& v)
{
    x *= v.x; y *= v.y;
    return *this;
}

vector2& vector2::operator /= (const vector2& v)
{
    x /= v.x; y /= v.y;
    return *this;
}

vector2& vector2::operator += (FLOAT s)
{
    x += s; y += s;
    return *this;
}

vector2& vector2::operator -= (FLOAT s)
{
    x -= s; y -= s;
    return *this;
}

vector2& vector2::operator *= (FLOAT s)
{
    x *= s; y *= s;
    return *this;
}

vector2& vector2::operator /= (FLOAT s)
{
    x /= s; y /= s;
    return *this;
}

vector2 vector2::operator + () const
{
    return *this;
}

vector2 vector2::operator - () const
{
    return vector2(-x, -y);
}

vector2 vector2::operator + (const vector2& v) const
{
    return vector2(x + v.x, y + v.y);
}

vector2 vector2::operator - (const vector2& v) const
{
    return vector2(x - v.x, y - v.y);
}

vector2 vector2::operator * (FLOAT s) const
{
    return vector2(x * s, y * s);
}

vector2 vector2::operator / (FLOAT s) const
{
    return vector2(x / s, y / s);
}

BOOL vector2::operator == (const vector2& v) const
{
    return x == v.x && y == v.y; // 有精度问题，这里先不管
}

BOOL vector2::operator != (const vector2& v) const
{
    return x != v.x && y != v.y;
}

vector2 operator * (FLOAT s, const vector2& v)
{
    return vector2(v.x*s, v.y*s);
}

FLOAT SquareMagnitude(const vector2& v)
{
    return v.x*v.x + v.y*v.y; /* DotProduct(v, v) */
}

FLOAT Magnitude(const vector2& v)
{
    return sqrt(SquareMagnitude(v));
}

vector2 Normalize(const vector2& v)
{
    return v / Magnitude(v);
}

FLOAT DotProduct(const vector2& v1, const vector2& v2)
{
    return v1.x*v2.x + v1.y*v2.y;
}
