#include "/include/gui"
#include "/include/math"
#include "/include/proc"
double fun2_f(double x, double y, double z) {
    double a = x * x + 9.0 / 4.0 * y * y + z * z - 1.0;
    return a * a* a - x * x * z * z * z - 9.0 / 80.0 * y * y * z * z * z;
}
double fun2_h(double x, double z) {
    double y;
    for (y = 1.0; y >= 0.0; y -= 0.001)
        if (fun2_f(x, y, z) <= 0.0)
            return y;
    return 0.0;
}
int min(int a, int b) {
    return a > b ? b : a;
}
int main(int argc, char **argv) {
    gui_power_on();
    double fovScale = 2.0;
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
    gui_draw_text("【画爱心-普通版】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    int x, y, c;
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = ((sx - 0.5) * fovScale) * 2.0;
            double yy = ((sy - 0.5) * fovScale) * 2.0;
            
            double a = xx * xx + yy * yy - 1.0;
            double f = a * a * a - xx * xx * yy * yy * yy;
            if (f <= 0.0) {
                gui_rgb(255, 0, 0);
                gui_point(x, y);
            }
            else {
                gui_rgb(0, 0, 0);
                gui_point(x, y);
            }

        }
        //sleep(1);
    }
    sleep(2000);
    gui_clear(0, 0, 0);
    gui_rgb(255, 0, 0);
    gui_font_size(32);
    gui_font_refresh();
    gui_move_to(start_x, start_y);
    gui_draw_text("【画爱心-高级版】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    int step = 4;
    for (y = start_y; y < stop_y; y += step) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x += step) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = ((sx - 0.5) * fovScale) * 2.0;
            double yy = ((sy - 0.5) * fovScale) * 2.0;

            double a = xx * xx + yy * yy - 1.0;
            double f = fun2_f(xx, 0.0, yy);
            if (f <= 0.0) {
                double y0 = fun2_h(xx, yy);
                double ny = 0.01;
                double nx = fun2_h(xx + ny, yy) - y0;
                double nz = fun2_h(xx, yy + ny) - y0;
                double nd = 1.0 / sqrt(nx * nx + ny * ny + nz * nz);
                double d = (nx + ny - nz) * nd * 0.5 + 0.5;
                c = (int)(d * 240);
                if (c > 255) c = 255;
                if (c < 0) c = 0;
                gui_rgb(c, 0, 0);
                gui_move_to(x, y);
                gui_rect(min(x + step, stop_x), min(y + step, stop_y));
            }
            /*else {
                gui_rgb(0, 0, 0);
                gui_move_to(x, y);
                gui_rect(min(x + step, stop_x), min(y + step, stop_y));
            }*/
        }
        //sleep(1);
    }
    sleep(5000);
    gui_power_off();
    return 0;
}