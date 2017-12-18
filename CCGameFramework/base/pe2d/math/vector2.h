#ifndef VECTOR2_H
#define VECTOR2_H
#include <stdafx.h>

// REFER TO: D3DX9MATH.H

typedef struct {
    float x;
    float y;
} _vector2;

typedef struct vector2 : public _vector2
{
    vector2();
    vector2(FLOAT pf);
    vector2(const _vector2& v);
    vector2(FLOAT fx, FLOAT fy);

    vector2& operator += (const vector2&);
    vector2& operator -= (const vector2&);
    vector2& operator *= (const vector2&);
    vector2& operator /= (const vector2&);
    vector2& operator += (FLOAT);
    vector2& operator -= (FLOAT);
    vector2& operator *= (FLOAT);
    vector2& operator /= (FLOAT);

    vector2 operator + () const;
    vector2 operator - () const;

    vector2 operator + (const vector2&) const;
    vector2 operator - (const vector2&) const;
    vector2 operator * (FLOAT) const;
    vector2 operator / (FLOAT) const;

    friend vector2 operator * (FLOAT, const struct vector2&);

    BOOL operator == (const vector2&) const;
    BOOL operator != (const vector2&) const;
} vector2;

FLOAT SquareMagnitude(const vector2& v);
FLOAT Magnitude(const vector2& v);
vector2 Normalize(const vector2& v);
FLOAT DotProduct(const vector2& v1, const vector2& v2);

#endif // VECTOR2_H
