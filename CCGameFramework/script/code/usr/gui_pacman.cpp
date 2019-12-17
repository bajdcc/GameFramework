#include "/include/gui"
#include "/include/math"
#include "/include/proc"
int min(int a, int b) {
    return a > b ? b : a;
}
int f(double x, double y, int d) {
    return
        pow(x - 0.5, 2.0) + pow(y - 0.5, 2.0) < 0.25 &&
        fabs(atan2(y - 0.5, x - 0.5)) > 0.5 &&
        pow(x - 0.5, 2.0) + pow(y - 0.75, 2.0) > 0.005 &&
        (d == 0 || f(fmod(x * 8, 1), fmod(y * 8, 1), d - 1));
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
    gui_draw_text("【吃豆人-原始版】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    int x, y, c;
    fovScale = 1.3;
    gui_rgb(255, 0, 0);
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = ((sx - 0.5) * fovScale) * 2.0;
            double yy = ((sy - 0.5) * fovScale) * 2.0;
            
            if (xx * xx + yy * yy < 1.0 &&
                fabs(atan2(yy, xx)) > 0.5 &&
                xx * xx + pow(yy - 0.5, 2.0) > 0.02) {
                gui_point(x, y);
            }
            //else {
                //gui_rgb(0, 0, 0);
                //gui_point(x, y);
            //}

        }
        //sleep(1);
    }
    sleep(3000);
    gui_clear(0, 0, 0);
    gui_rgb(255, 0, 0);
    gui_font_size(32);
    gui_font_refresh();
    gui_move_to(start_x, start_y);
    gui_draw_text("【吃豆人-分形版】");
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    fovScale = 1.2;
    gui_rgb(255, 0, 0);
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = -0.1 + sx * fovScale;
            double yy = -0.1 + sy * fovScale;

            if (f(xx, yy, 1.0)) {
                gui_point(x, y);
            }
            //else {
                //gui_rgb(0, 0, 0);
                //gui_point(x, y);
            //}

        }
        //sleep(1);
    }
    sleep(5000);
    gui_power_off();
    return 0;
}