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

void gui_width() {
    gui_enable(2);
}
void gui_height() {
    gui_enable(3);
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
