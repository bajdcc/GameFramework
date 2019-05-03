//
// Project: clib2d
// Created by bajdcc
//

#include "stdafx.h"
#include "c2dpolygon.h"
#include "c2dworld.h"

namespace clib {

    c2d_polygon::c2d_polygon(uint16_t _id, decimal _mass, const std::vector<v2>& _vertices)
        : c2d_body(_id, _mass), vertices(_vertices), verticesWorld(_vertices) {
        init();
    }

    decimal c2d_polygon::calc_polygon_area(const std::vector<v2>& vertices) {
        decimal area = 0;
        auto size = vertices.size();
        // 求所有三角形的面积之和
        for (size_t i = 0; i < size; ++i) {
            auto j = (i + 1) % size;
            // 叉乘求两相邻向量所成平行四边形面积
            // 所以要求三角形面积就要除以2
            area += vertices[i].cross(vertices[j]);
        }
        return area / 2;
    }

    v2 c2d_polygon::calc_polygon_centroid(const std::vector<v2> & vertices) {
        v2 gc;
        auto size = vertices.size();
        // 重心 = (各三角形重心 * 其面积) / 总面积
        // 三角形重心 = 两向量之和 / 3
        for (size_t i = 0; i < size; ++i) {
            auto j = (i + 1) % size;
            gc += (vertices[i] + vertices[j]) * vertices[i].cross(vertices[j]);
        }
        return gc / 6.0 / calc_polygon_area(vertices);
    }

    decimal c2d_polygon::calc_polygon_inertia(decimal mass, const std::vector<v2> & vertices) {
        if (std::isinf(mass))
            return mass;
        decimal acc0 = 0, acc1 = 0;
        auto size = vertices.size();
        // 转动惯量 = m / 6 * (各三角形面积 * 其(a*a+a*b+b*b)) / (总面积)
        for (size_t i = 0; i < size; ++i) {
            auto a = vertices[i], b = vertices[(i + 1) % size];
            auto _cross = std::abs(a.cross(b));
            acc0 += _cross * (a.dot(a) + b.dot(b) + a.dot(b));
            acc1 += _cross;
        }
        return mass * acc0 / 6 / acc1;
    }

    void c2d_polygon::calc_bounds() {
        boundMin = boundMax = vertex(0);
        for (size_t i = 1; i < verticesWorld.size(); ++i) {
            boundMin.x = min(boundMin.x, vertex(i).x);
            boundMin.y = min(boundMin.y, vertex(i).y);
            boundMax.x = max(boundMax.x, vertex(i).x);
            boundMax.y = max(boundMax.y, vertex(i).y);
        }
    }

    bool c2d_polygon::contains_in_bound(const v2 & pt) {
        return boundMin.x < pt.x&&
            boundMax.x > pt.x&&
            boundMin.y < pt.y&&
            boundMax.y > pt.y;
    }

    bool c2d_polygon::contains_in_polygon(const v2 & pt) {
        const auto size = verticesWorld.size();
        if (size < 3) return false;
        if ((pt - vertex(0)).cross(vertex(1) - vertex(0)) > 0)
            return false;
        if ((pt - vertex(0)).cross(vertex(size - 1) - vertex(0)) < 0)
            return false;

        // 判断剩下的连线方向的一致性
        size_t i = 2, j = size - 1;
        auto line = SIZE_MAX;

        // 二分法
        while (i <= j) {
            auto mid = (i + j) >> 1;
            if ((pt - vertex(0)).cross(vertex(mid) - vertex(0)) > 0) {
                line = mid;
                j = mid - 1;
            }
            else i = mid + 1;
        }
        return (pt - vertex(line - 1)).cross(vertex(line) - vertex(line - 1)) < 0;
    }

    bool c2d_polygon::contains(const v2 & pt) {
        // 先快速判断是否在包围框（外包矩形）中
        // 是则具体判断在多边形中（这步计算大）
        return contains_in_bound(pt) && contains_in_polygon(pt);
    }

    void c2d_polygon::init() {
        inertia.set(calc_polygon_inertia(mass.value, vertices));
        center = calc_polygon_centroid(vertices);
        refresh();
    }

    void c2d_polygon::refresh() {
        R.rotate(angle);
        for (size_t i = 0; i < edges(); ++i) {
            auto v = R.rotate(vertices[i] - center) + center;
            vertex(i) = pos + v; // 本地坐标转换为世界坐标
        }
        calc_bounds();
    }

    void c2d_polygon::impulse(const v2 & p, const v2 & r) {
        if (statics) return;
        auto _p = p * c2d_world::dt_inv;
        F += _p;
        Fa += _p;
        M += r.cross(_p);
    }

    v2 c2d_polygon::world() const {
        return pos + center;
    }

    c2d_body_t c2d_polygon::type() const {
        return C2D_POLYGON;
    }

    v2 c2d_polygon::_min() const {
        return boundMin;
    }

    v2 c2d_polygon::_max() const {
        return boundMax;
    }

    void c2d_polygon::update(v2 gravity, int n) {
        if (statics) return;
#if ENABLE_SLEEP
        if (sleep) return;
#endif
        switch (n) {
        case 0:
            pass0();
            break;
        case 1:
            pass1();
            break;
        case 2:
            pass2();
            break;
        case 3:
            pass3(gravity);
            break;
        case 4:
            pass4();
            break;
        case 5:
            pass5();
            break;
        default:
            break;
        }
    }

    void c2d_polygon::pass0() {
        F.x = F.y = 0;
        M = 0;
    }

    void c2d_polygon::pass1() {
        V += F * mass.inv * c2d_world::dt;
        angleV += M * inertia.inv * c2d_world::dt;
    }

    void c2d_polygon::pass2() {
        pos += V * c2d_world::dt;
        angle += angleV * c2d_world::dt;
        R.rotate(angle);
        for (size_t i = 0; i < edges(); ++i) {
            auto v = R.rotate(vertices[i] - center) + center;
            vertex(i) = pos + v; // 本地坐标转换为世界坐标
        }
        calc_bounds();
    }

    void c2d_polygon::pass3(const v2 & gravity) {
        F += gravity * mass.value * c2d_world::dt;
        Fa += F;
    }

    void c2d_polygon::pass4() {
        Fa.x = Fa.y = 0;
    }

    void c2d_polygon::pass5() {
#if ENABLE_SLEEP
        // 当合外力和速度为零时，判定休眠
        if (Fa.zero(EPSILON_FORCE) && V.zero(EPSILON_V) && std::abs(angleV) < EPSILON_ANGLE_V) {
            V.x = 0;
            V.y = 0;
            angleV = 0;
            pass0();
            pass4();
            collision = 0;
            sleep = true;
        }
#endif
    }

    void c2d_polygon::drag(const v2 & pt, const v2 & offset) {
        V += mass.inv * offset;
        angleV += inertia.inv * (pt - pos - center).cross(offset);
    }

    void c2d_polygon::draw(CComPtr<ID2D1RenderTarget> & rt, const CRect & bounds, const clib::BrushBag & brushes) {
        if (statics) { // 画静态物体
            auto k = verticesWorld.back();
            for (auto& v : verticesWorld) {
                rt->DrawLine(c2d_world::transform(bounds, k), c2d_world::transform(bounds, v), brushes.brushes[b_static]);
                k = v;
            }
            return;
        }
#if ENABLE_SLEEP
        if (sleep) { // 画休眠物体
            auto k = verticesWorld.back();
            for (auto& v : verticesWorld) {
                rt->DrawLine(c2d_world::transform(bounds, k), c2d_world::transform(bounds, v), brushes.brushes[b_sleep]);
                k = v;
            }
            return;
        }
#endif
        rt->DrawRectangle(D2D1::RectF(
            c2d_world::transform_pt_x(bounds, boundMin.x),
            c2d_world::transform_pt_y(bounds, boundMax.y),
            c2d_world::transform_pt_x(bounds, boundMax.x),
            c2d_world::transform_pt_y(bounds, boundMin.y)), brushes.brushes[b_bounds]);
        auto k = verticesWorld.back();
        for (auto& v : verticesWorld) {
            rt->DrawLine(c2d_world::transform(bounds, k), c2d_world::transform(bounds, v), brushes.brushes[b_normal]);
            k = v;
        }
        auto p = pos + center;
        auto v = p + V * 0.2;
        rt->DrawLine(c2d_world::transform(bounds, p), c2d_world::transform(bounds, v2(
            p.x + (Fa.x >= 0 ? 0.2 : -0.2) * std::log10(1 + std::abs(Fa.x) * 5),
            p.y + (Fa.y >= 0 ? 0.2 : -0.2) * std::log10(1 + std::abs(Fa.y) * 5))), brushes.brushes[b_F]);
        rt->DrawLine(c2d_world::transform(bounds, p), c2d_world::transform(bounds, v), brushes.brushes[b_V]);
        rt->DrawLine(c2d_world::transform(bounds, p), c2d_world::transform(bounds, v2(
            p.x + R.x1 * 0.2, p.y + R.x2 * 0.2)), brushes.brushes[b_D]);
        rt->FillEllipse(D2D1::Ellipse(c2d_world::transform(bounds, p), 1.0f, 1.0f), brushes.brushes[b_center]);
    }

    v2 c2d_polygon::edge(size_t idx) const {
        return verticesWorld[index(idx + 1)] - verticesWorld[index(idx)];
    }

    v2& c2d_polygon::vertex(size_t idx) {
        return verticesWorld[index(idx)];
    }

    size_t c2d_polygon::index(size_t idx) const {
        return idx % verticesWorld.size();
    }

    size_t c2d_polygon::edges() const {
        return verticesWorld.size();
    }
}