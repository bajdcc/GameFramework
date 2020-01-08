//
// Project: clib2d
// Created by bajdcc
//

#ifndef MPM_V2_H
#define MPM_V2_H

#include <cmath>

namespace clib {
    namespace mpm {

        using decimal = float;

        // 二维向量
        template<class T = decimal>
        struct v2 {
            T x{ 0 }, y{ 0 }; // X、Y坐标

            // 构造函数
            v2() = default;

            v2(T _x, T _y) : x(_x), y(_y) {}

            v2(const v2& v) = default;

            v2& operator=(const v2& v) = default;

            v2 operator*(const T& d) const {
                return { x * d, y * d };
            }

            v2 operator/(const T& d) const {
                return { x / d, y / d };
            }

            v2 operator+(const v2& v) const {
                return { x + v.x, y + v.y };
            }

            v2 operator-(const v2& v) const {
                return { x - v.x, y - v.y };
            }

            v2 operator+(const T& d) const {
                return { x + d, y + d };
            }

            v2 operator-(const T& d) const {
                return { x - d, y - d };
            }

            v2& operator*=(const T& d) {
                x *= d;
                y *= d;
                return *this;
            }

            v2& operator+=(const v2& v) {
                x += v.x;
                y += v.y;
                return *this;
            }

            v2& operator-=(const v2& v) {
                x -= v.x;
                y -= v.y;
                return *this;
            }

            friend v2 operator*(const T& d, const v2& v) {
                return { d * v.x, d * v.y };
            }

            v2 operator-() const {
                return { -x, -y };
            }

            // 叉乘
            T cross(const v2& v) const {
                return x * v.y - y * v.x;
            }

            // 点乘
            T dot(const v2& v) const {
                return x * v.x + y * v.y;
            }

            T magnitude() const {
                return std::sqrt(x * x + y * y);
            }

            T magnitude_square() const {
                return x * x + y * y;
            }

            v2 normalize() const {
                return *this / magnitude();
            }

            // 法线向量
            v2 normal() const {
                return N().normalize();
            }

            v2 N() const {
                return v2{ y, -x };
            }

            bool zero(T d) const {
                return std::abs(x) < d && std::abs(y) < d;
            }

            v2 to_int() const {
                return v2(T((int)x), T((int)y));
            }
        };
    }
}

#endif //MPM_V2_H
