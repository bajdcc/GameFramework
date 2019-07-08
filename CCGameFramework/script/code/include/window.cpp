//
// Project: clibparser
// Created by bajdcc
//

// 窗口
#include "/include/fs"
#include "/include/io"

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

struct __window_msg_struct__ {
    int code;
    int param1, param2;
};

int window_get_msg(int id, __window_msg_struct__* s) {
    int c, i;
    __window_msg_struct__ s0;
    char* p = (char*)& s0;
    for (i = 0; c < 0x1000 && i < sizeof(__window_msg_struct__); i++) {
        c = read(id);
        *p++ = (char)c;
    }
    *s = s0;
    return c;
}

struct __window_defmsg_struct__ {
    int handle;
    __window_msg_struct__ msg;
};

int window_default_msg(int handle, __window_msg_struct__* s) {
    __window_defmsg_struct__ k;
    k.handle = handle;
    k.msg = *s;
    &k;
    interrupt 502;
}