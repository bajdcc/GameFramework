#include "/include/gui"
#include "/include/math"
#include "/include/proc"
struct v3 {
    double x, y, z;
};
v3 v3_new(double x, double y, double z){
    v3 v;
    v.x = x; v.y = y; v.z = z;
    return v;
}
v3 v3_cross(v3 v1, v3 v2) {
    v3 v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}
double v3_dot(v3 v1, v3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
v3 v3_add(v3 v1, v3 v2) {
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}
v3 v3_sub(v3 v1, v3 v2) {
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    return v1;
}
v3 v3_mul(v3 v, double d) {
    v.x *= d;
    v.y *= d;
    v.z *= d;
    return v;
}
double v3_sq(v3 v) {
    return v.x* v.x + v.y * v.y + v.z * v.z;
}
v3 v3_normalize(v3 v) {
    double m = 1 / sqrt(v3_sq(v));
    v.x *= m;
    v.y *= m;
    v.z *= m;
    return v;
}
double min(double a, double b) {
    return a > b ? b : a;
}
int main(int argc, char **argv) {
    gui_power_on();
    v3 eye   = v3_new(0, 10, 10);         // 摄影机眼睛的位置
    v3 front = v3_new(0, 0, -1);          // 视角中向前方向的单位向量
    v3 up    = v3_new(0, 1, 0);           // 视角中向上方向的单位向量
    v3 right = v3_cross(front, up);       // 视角中向右方向的单位向量
    double fovScale = 2.0;                // 视角缩放
    double maxDepth = 20.0;               // 最大深度

    v3 center = v3_new(0, 10, -10);       // 球的中心点
    double r = 10.0;                      // 球的半径
    double rsq = r * r;                   // 球的半径平方

    int width = gui_width(), height = gui_height();
    int border = 10;
    int start_x = border, start_y = border;
    int stop_x = width - border, stop_y = height - border;
    if (width > height) {
        start_x += (width - height) / 2;
        stop_x -= (width - height) / 2;
    }
    else {
        start_y += (height - width) / 2;
        stop_y -= (height - width) / 2;
        height = width;
    }
    width = stop_x - start_x;
    height = stop_y - start_y;
    gui_clear(0, 0, 0);
    gui_rgb(255, 0, 0);
    gui_font_size(32);
    gui_font_refresh();
    gui_move_to(start_x, start_y);
    gui_draw_text("【三维球体-灰度】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    int x, y, suc;
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            suc = 0;
            double sx = 1.0 * (x - start_x) / width;
            v3 r = v3_mul(right, ((sx - 0.5) * fovScale));
            v3 u = v3_mul(up, ((sy - 0.5) * fovScale));

            // 求出起点到取样位置3D实际位置的距离单位向量
            v3 direction = v3_normalize(v3_add(v3_add(front, r),u));

            // 测试光线与球是否相交
            // 球面上点x满足： || 点x - 球心center || = 球半径radius
            // 光线方程 r(t) = o + t.d (t>=0)
            // 代入得 || o + t.d - c || = r
            // 令 v = o - c，则 || v + t.d || = r

            // 化简求 t = - d.v - sqrt( (d.v)^2 + (v^2 - r^2) )  (求最近点)

            // 令 v = origin - center
            v3 v = v3_sub(eye, center);

            // a0 = (v^2 - r^2)
            double a0 = v3_sq(v) - rsq;

            // DdotV = d.v
            double DdotV = v3_dot(direction, v);

            if (DdotV <= 0) {
                // 点乘测试相交，为负则同方向

                double discr = (DdotV * DdotV) - a0; // 平方根中的算式

                if (discr >= 0) {
                    // 非负则方程有解，相交成立

                    // r(t) = o + t.d
                    // 得出t，即摄影机发出的光线到其与球面的交点距离
                    double distance = -DdotV - sqrt(discr);
                    // 代入直线方程，得出交点位置
                    v3 position = v3_add(eye, v3_mul(direction, distance));
                    // 法向量 = 光线终点(球面交点) - 球心坐标
                    v3 normal = v3_normalize(v3_sub(position, center));

                    suc = 1;

                    double depth = 255 - min((distance / maxDepth) * 255.0, 255.0);

                    // 输出灰阶
                    gui_rgb((int)depth, (int)depth, (int)depth);
                    gui_point(x, y);
                }
            }

            if (!suc) {
                // 没有接触，就是背景色
                gui_rgb(0, 0, 0);
                gui_point(x, y);
            }
        }
        //sleep(1);
    }
    sleep(5000);
    gui_clear(0, 0, 0);
    gui_rgb(255, 0, 0);
    gui_font_size(32);
    gui_font_refresh();
    gui_move_to(start_x, start_y);
    gui_draw_text("【三维球体-法向量】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            suc = 0;
            double sx = 1.0 * (x - start_x) / width;
            v3 r = v3_mul(right, ((sx - 0.5) * fovScale));
            v3 u = v3_mul(up, ((sy - 0.5) * fovScale));

            // 求出起点到取样位置3D实际位置的距离单位向量
            v3 direction = v3_normalize(v3_add(v3_add(front, r), u));

            // 测试光线与球是否相交
            // 球面上点x满足： || 点x - 球心center || = 球半径radius
            // 光线方程 r(t) = o + t.d (t>=0)
            // 代入得 || o + t.d - c || = r
            // 令 v = o - c，则 || v + t.d || = r

            // 化简求 t = - d.v - sqrt( (d.v)^2 + (v^2 - r^2) )  (求最近点)

            // 令 v = origin - center
            v3 v = v3_sub(eye, center);

            // a0 = (v^2 - r^2)
            double a0 = v3_sq(v) - rsq;

            // DdotV = d.v
            double DdotV = v3_dot(direction, v);

            if (DdotV <= 0) {
                // 点乘测试相交，为负则同方向

                double discr = (DdotV * DdotV) - a0; // 平方根中的算式

                if (discr >= 0) {
                    // 非负则方程有解，相交成立

                    // r(t) = o + t.d
                    // 得出t，即摄影机发出的光线到其与球面的交点距离
                    double distance = -DdotV - sqrt(discr);
                    // 代入直线方程，得出交点位置
                    v3 position = v3_add(eye, v3_mul(direction, distance));
                    // 法向量 = 光线终点(球面交点) - 球心坐标
                    v3 normal = v3_normalize(v3_sub(position, center));

                    suc = 1;

                    double depth = 255 - min((distance / maxDepth) * 255.0, 255.0);

                    // 输出法向量
                    gui_rgb(
                        (int)((normal.x + 1) * 128),
                        (int)((normal.y + 1) * 128),
                        (int)((normal.z + 1) * 128));
                    gui_point(x, y);
                }
            }

            if (!suc) {
                // 没有接触，就是背景色
                gui_rgb(0, 0, 0);
                gui_point(x, y);
            }
        }
        //sleep(1);
    }
    sleep(5000);
    gui_power_off();
    return 0;
}