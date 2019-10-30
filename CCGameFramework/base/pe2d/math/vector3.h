#ifndef VECTOR3_H
#define VECTOR3_H
#include <stdafx.h>

// REFER TO: D3DX9MATH.H

typedef struct {
    float x;
    float y;
    float z;
} _vector3;

typedef struct vector3 : public _vector3
{
    vector3();
    vector3(FLOAT pf);
    vector3(const _vector3& v);
    vector3(FLOAT fx, FLOAT fy, FLOAT fz);

    vector3& operator += (const vector3&);
    vector3& operator -= (const vector3&);
    vector3& operator *= (const vector3&);
    vector3& operator /= (const vector3&);
    vector3& operator += (FLOAT);
    vector3& operator -= (FLOAT);
    vector3& operator *= (FLOAT);
    vector3& operator /= (FLOAT);

    vector3 operator + () const;
    vector3 operator - () const;

    vector3 operator + (const vector3&) const;
    vector3 operator - (const vector3&) const;
    vector3 operator * (FLOAT) const;
    vector3 operator / (FLOAT) const;

    friend vector3 operator * (FLOAT, const struct vector3&);

    BOOL operator == (const vector3&) const;
    BOOL operator != (const vector3&) const;
} vector3;

FLOAT SquareMagnitude(const vector3& v);
FLOAT Magnitude(const vector3& v);
vector3 Normalize(const vector3& v);
FLOAT DotProduct(const vector3& v1, const vector3& v2);
vector3 CrossProduct(const vector3& v1, const vector3& v2);
vector3 Rotate(const vector3& v1, const vector3& v2, const float& delta);

#endif // VECTOR3_H
