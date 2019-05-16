//
// Project: clibparser
// Created by bajdcc
//

// 图形用户界面
// 屏幕坐标系

int gui_enable(int flag) {
    flag;
    interrupt 301;
}

void gui_power_on() {
    gui_enable(1);
}
void gui_power_off() {
    gui_enable(0);
}

int gui_width() {
    return gui_enable(2);
}
int gui_height() {
    return gui_enable(3);
}

int gui_move_to(int x, int y) {
    x << 16 | y;
    interrupt 302;
}

int gui_line_to(int x, int y) {
    x << 16 | y;
    interrupt 303;
}

int gui_point(int x, int y) {
    x << 16 | y;
    interrupt 304;
}

int gui_rgb(int r, int g, int b) {
    0xff << 24 | r << 16 | g << 8 | b;
    interrupt 305;
}

int gui_rgba(int r, int g, int b, int a) {
    a << 24 | r << 16 | g << 8 | b;
    interrupt 305;
}
