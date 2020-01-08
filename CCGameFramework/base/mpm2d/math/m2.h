//
// Project: clib2d
// Created by bajdcc
//

#ifndef MPM_M2_H
#define MPM_M2_H

#include "v2.h"

namespace clib {
    namespace mpm {

        static const auto inf = std::numeric_limits<decimal>::infinity();

        // 二维矩阵
        template<class T = decimal>
        struct m2 {
            T x1{ 1 }, y1{ 0 }, x2{ 0 }, y2{ 1 };

            m2() = default;

            m2(T _x1, T _y1, T _x2, T _y2) : x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}

            m2(const m2& m) = default;

            m2& operator=(const m2& m) = default;

            m2(T d) : x1(d), y1(0), x2(0), y2(d) {}

            m2(const v2<T>& v1, const v2<T>& v2) : x1(v1.x * v2.x), y1(v1.x * v2.y), x2(v1.y * v2.x), y2(v1.y * v2.y) {}

            m2 operator+(const m2& m) const {
                return { x1 + m.x1, y1 + m.y1, x2 + m.x2, y2 + m.y2 };
            }

            m2 operator*(T d) const {
                return { x1 * d, y1 * d, x2 * d, y2 * d };
            }

            v2<T> operator*(const v2<T>& v) const {
                return { x1 * v.x + y1 * v.y, x2 * v.x + y2 * v.y };
            }

            friend m2 operator*(T d, const m2& m) {
                return m * d;
            }

            m2& operator+=(const m2& m) {
                x1 += m.x1;
                y1 += m.y1;
                x2 += m.x2;
                y2 += m.y2;
                return *this;
            }

            const m2& rotate(T theta) {
                const auto _sin = std::sin(theta);
                const auto _cos = std::cos(theta);
                *this = m2{ _cos, -_sin, _sin, _cos };
                return *this;
            }

            v2<T> rotate(const v2<T>& v) const {
                return { x1 * v.x + y1 * v.y, x2 * v.x + y2 * v.y };
            }

            T det() const {
                return x1 * y2 - x2 * y1;
            }

            m2 inv() const {
                auto _det = det();
                return _det == 0 ? m2(inf, inf, inf, inf) : ((1 / _det) * m2(y2, -x2, -y1, x1));
            }
        };
    }
}

#endif //MPM_M2_H
