﻿#include "stdafx.h"
#include "vector3.h"

// REFER TO: D3DX9MATH.H

vector3::vector3()
{
    x = y = z = 0.0f;
}

vector3::vector3(FLOAT f)
{
    x = y = z = f;
}

vector3::vector3(const _vector3& v)
{
    x = v.x; y = v.y; z = v.z;
}

vector3::vector3(FLOAT _x, FLOAT _y, FLOAT _z)
{
    x = _x; y = _y; z = _z;
}

vector3& vector3::operator += (const vector3& v)
{
    x += v.x; y += v.y; z += v.z;
    return *this;
}

vector3& vector3::operator -= (const vector3& v)
{
    x -= v.x; y -= v.y; z -= v.z;
    return *this;
}

vector3& vector3::operator *= (const vector3& v)
{
    x *= v.x; y *= v.y; z *= v.z;
    return *this;
}

vector3& vector3::operator /= (const vector3& v)
{
    x /= v.x; y /= v.y; z /= v.z;
    return *this;
}

vector3& vector3::operator += (FLOAT s)
{
    x += s; y += s; z += s;
    return *this;
}

vector3& vector3::operator -= (FLOAT s)
{
    x -= s; y -= s; z -= s;
    return *this;
}

vector3& vector3::operator *= (FLOAT s)
{
    x *= s; y *= s; z *= s;
    return *this;
}

vector3& vector3::operator /= (FLOAT s)
{
    x /= s; y /= s; z /= s;
    return *this;
}

vector3 vector3::operator + () const
{
    return *this;
}

vector3 vector3::operator - () const
{
    return vector3(-x, -y, -z);
}

vector3 vector3::operator + (const vector3& v) const
{
    return vector3(x + v.x, y + v.y, z + v.z);
}

vector3 vector3::operator - (const vector3& v) const
{
    return vector3(x - v.x, y - v.y, z - v.z);
}

vector3 vector3::operator * (FLOAT s) const
{
    return vector3(x * s, y * s, z * s);
}

vector3 vector3::operator / (FLOAT s) const
{
    return vector3(x / s, y / s, z / s);
}

BOOL vector3::operator == (const vector3& v) const
{
    return x == v.x && y == v.y && z == v.z; // 有精度问题，这里先不管
}

BOOL vector3::operator != (const vector3& v) const
{
    return x != v.x && y != v.y && z != v.z;
}

vector3 operator * (FLOAT s, const vector3& v)
{
    return vector3(v.x * s, v.y * s, v.z * s);
}

FLOAT SquareMagnitude(const vector3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z; /* DotProduct(v, v) */
}

FLOAT Magnitude(const vector3& v)
{
    return sqrt(SquareMagnitude(v));
}

vector3 Normalize(const vector3& v)
{
    return v / Magnitude(v);
}

FLOAT DotProduct(const vector3& v1, const vector3& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vector3 CrossProduct(const vector3& v1, const vector3& v2)
{
    return vector3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

vector3 Rotate(const vector3& v1, const vector3& v2, const float& delta)
{
    const auto c = cosf(delta);
    const auto s = sinf(delta);
    const auto c_ = 1 - c;
    const auto s_ = 1 - s;
    const auto& x = v1.x;
    const auto& y = v1.y;
    const auto& z = v1.z;
    const auto xx = x * x;
    const auto xy = x * y;
    const auto xz = x * z;
    const auto yy = y * y;
    const auto yz = y * z;
    const auto zz = z * z;
    const auto& x_ = v2.x;
    const auto& y_ = v2.y;
    const auto& z_ = v2.z;
    return vector3(
        (xx * c_ + c) * x_ + (xy * c_ + z * s) * y_ + (xz * c_ - y * s) * z_,
        (xy * c_ - z * s) * x_ + (yy * c_ + c) * y_ + (yz * c_ + x * s) * z_,
        (xz * c_ + y * s) * x_ + (yz * c_ - x * s) * y_ + (zz * c_ + c) * z_
    );
}
