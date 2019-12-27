#include "/include/gui"
#include "/include/math"
#include "/include/proc"
int main(int argc, char** argv) {
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
    gui_rgb(255, 255, 255);
    gui_move_to(start_x, start_y);
    gui_line_to(start_x, stop_y);
    gui_line_to(stop_x, stop_y);
    gui_line_to(stop_x, start_y);
    gui_line_to(start_x, start_y);
    int x, y, c, i;
    fovScale = 1.3;
    gui_rgb(255, 0, 0);
    int max_iter = 100;
    for (y = start_y; y < stop_y; y++) {
        double sy = 1.0 - (1.0 * (y - start_y) / height);
        for (x = start_x; x < stop_x; x++) {
            double sx = 1.0 * (x - start_x) / width;
            double xx = ((sx - 0.5) * fovScale) * 5.0;
            double yy = ((sy - 0.5) * fovScale) * 5.0;

            double sq = 0.0;
            double zx = 0.0, zy = 0.0;
            for (i = 0; sq < 4.0 && i < max_iter; ++i) {
                double temp = zx * zx - zy * zy + xx;
                zy = 2.0 * zx * zy + yy;
                zx = temp;
                sq = zx * zx + zy * zy;
            }
            if (max_iter == i) {
                gui_color(0);
            }
            else {
                double n = 1.0 * i / max_iter * 240.0 + 210.0;
                if (n >= 240.0) n -= 240.0;
                gui_color(gui_hsl2rgb((int)n, 240, 140));
            }
            gui_point(x, y);
        }
        sleep(1);
    }
    gui_rgb(255, 0, 0);
    gui_font_size(32);
    gui_font_refresh();
    gui_move_to(start_x, start_y);
    gui_draw_text("曼德勃罗集");
    sleep(5000);
    gui_power_off();
    return 0;
}