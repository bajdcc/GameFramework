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

int gui_move_to(int x, int y) {
    x << 16 | y;
    interrupt 302;
}

int gui_line_to(int x, int y) {
    x << 16 | y;
    interrupt 303;
}