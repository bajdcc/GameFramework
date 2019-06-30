//
// Project: clibparser
// Created by bajdcc
//

// 窗口

struct __window_create_struct__ {
    char* caption;
    int left, top, width, height;
};

int window_create(__window_create_struct__ *s) {
    s;
    interrupt 501;
}

int window_wait(int id) {
    id;
    interrupt 80;
}