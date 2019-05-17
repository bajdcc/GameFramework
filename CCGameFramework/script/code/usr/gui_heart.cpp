#include "/include/gui"
#include "/include/math"
#include "/include/proc"
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
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    int x, y;
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = ((sx - 0.5) * fovScale) * 2.0;
            double yy = ((sy - 0.5) * fovScale) * 2.0;
            
            double a = xx * xx + yy * yy - 1.0;
            if (a * a * a - xx * xx * yy * yy * yy <= 0.0) {
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
    sleep(5000);
    gui_power_off();
    return 0;
}