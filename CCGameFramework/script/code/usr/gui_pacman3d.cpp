#include "/include/gui"
#include "/include/math"
#include "/include/proc"
struct V {
    double x, y, z;
};
V make(double x, double y, double z){
    V v;
    v.x = x; v.y = y; v.z = z;
    return v;
}
V cross(V v1, V v2) {
    V v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    return v;
}
double dot(V v1, V v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
V add(V v1, V v2) {
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}
V sub(V v1, V v2) {
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    return v1;
}
V mul(V v, double d) {
    v.x *= d;
    v.y *= d;
    v.z *= d;
    return v;
}
double sq(V v) {
    return v.x* v.x + v.y * v.y + v.z * v.z;
}
V norm(V v) {
    double m = 1 / sqrt(sq(v));
    v.x *= m;
    v.y *= m;
    v.z *= m;
    return v;
}
double len(V v) {
    return sqrt(dot(v, v));
}
double min(double a, double b) {
    return a > b ? b : a;
}
double max(double a, double b) {
    return a > b ? a : b;
}
double map(V p) {
    V m1 = make(cos(1.0), 0.0, sin(1.0));
    V m2 = make(cos(1.0), 0.0, -sin(1.0));
    return min(max(len(p) - 1.0, min(dot(p, m1), dot(p, m2))), p.z + 1.0);
}
V normal(V p) {
    double e = 1e-6;
    V dx = make(e, 0.0, 0.0);
    V dy = make(0.0, e, 0.0);
    V dz = make(0.0, 0.0, e);
    return norm(make(
        map(add(p, dx)) - map(sub(p, dx)),
        map(add(p, dy)) - map(sub(p, dy)),
        map(add(p, dz)) - map(sub(p, dz))));
}
V albedo(V p) {
    if (p.z + 1.0 < 1e-4)
        return make(1.0, 1.0, 1.0);
    else if (p.x * p.x + pow(p.z - 0.5, 2.0) < 0.02)
        return make(0.0, 0.0, 0.0);
    else
        return make(1.0, 1.0, 0.0);
}
int main(int argc, char **argv) {
    gui_power_on();
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
    V eye = make(2.0, 3.0, 1.0), at = make(0.0, 0.0, 0.0), up = make(0.0, 0.0, 1.0);
    V vz = norm(sub(at, eye)); V vx = cross(up, vz); V vy = cross(vz, vx);
    double fovScale = 3.0;
    int x, y, i;
    int step = 4;
    for (y = start_y; y < stop_y; y += step) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x += step) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = (sx - 0.5) * fovScale;
            double yy = (sy - 0.5) * fovScale;

            V o = add(add(eye, mul(vx, xx)), mul(vy, yy));
            double t = 0.0;
            do {
                V p = add(o, mul(vz, t));
                double sd = map(p);
                if (sd < 1e-4) {
                    V n = normal(p), alb = albedo(p);
                    double ao = 1.0, s = 0.5;
                    for (i = 1; i < 5; i++) {
                        double d = i * 0.1;
                        ao = max(ao - (d - map(add(p, mul(n, d)))) * s, 0.1);
                        s *= 0.95;
                    }
                    V r = mul(alb, (dot(n, norm(make(0.0, 1.0, 1.0))) * 0.5 + 0.5) * ao * 255.0);
                    gui_rgb((int)r.x, (int)r.y, (int)r.z);
                    gui_move_to(x, y);
                    gui_rect(min(x + step, stop_x), min(y + step, stop_y));
                    break;
                }
                t += sd;
            } while (t < 100.0);
        }
        //sleep(1);
    }
    gui_rgb(255, 0, 0);
    gui_font_size(32);
    gui_font_refresh();
    gui_move_to(start_x, start_y);
    gui_draw_text("【吃豆人-3D】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    sleep(5000);
    gui_power_off();
    return 0;
}