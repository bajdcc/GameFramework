//
// Project: clibparser
// Created by bajdcc
//

// 窗口
#include "/include/fs"
#include "/include/readfile"
#include "/include/string"

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
    int code, comctl;
    int param1, param2;
};

int window_get_msg(int id, __window_msg_struct__* s) {
    int c = 0, i;
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

int window_post_msg(int handle, int code, int comctl, int param1, int param2) {
    __window_defmsg_struct__ k;
    k.handle = handle;
    k.msg.code = code;
    k.msg.comctl = comctl;
    k.msg.param1 = param1;
    k.msg.param2 = param2;
    &k;
    interrupt 502;
}

// API

int window_set_text(int handle, char* text) {
    window_post_msg(handle, 0xC, -1, text, 0);
}

int window_get_text(int handle, char* text) {
    window_post_msg(handle, 0xD, -1, text, 0);
}

enum window_comctl_type {
    layout_absolute = 1,
    layout_linear,
    layout_grid,
    comctl_label,
    comctl_button,
    comctl_image,
    comctl_edit,
    comctl_svg,
};

struct __window_create_comctl_struct__ {
    int handle;
    int type;
};

long window_create_comctl(int handle, int type) {
    __window_create_comctl_struct__ s;
    s.handle = handle;
    s.type = type;
    &s;
    interrupt 503;
}

long window_get_base(int handle) {
    handle;
    interrupt 504;
}

struct __window_comctl_connect_struct__ {
    long handle;
    long child;
};

int window_comctl_connect(long handle, long child) {
    __window_comctl_connect_struct__ s;
    s.handle = handle;
    s.child = child;
    &s;
    interrupt 505;
}

struct __window_comctl_set_flag_struct__ {
    long handle;
    int flag;
};

int window_comctl_set_flag(long handle, int flag) {
    __window_comctl_set_flag_struct__ s;
    s.handle = handle;
    s.flag = flag;
    &s;
    interrupt 506;
}

enum window_style {
    style_win10 = 1,
    style_win10_white,
};

struct __window_set_style_struct__ {
    int handle;
    int style;
};

int window_set_style(int handle, int style) {
    __window_set_style_struct__ s;
    s.handle = handle;
    s.style = style;
    &s;
    interrupt 507;
}

struct __window_comctl_set_bound_struct__ {
    long handle;
    int left, top, right, bottom;
};

int window_comctl_set_bound(long handle, int left, int top, int right, int bottom) {
    __window_comctl_set_bound_struct__ s;
    s.handle = handle;
    s.left = left;
    s.top = top;
    s.right = right;
    s.bottom = bottom;
    &s;
    interrupt 509;
}

struct __window_comctl_set_text_struct__ {
    long handle;
    char* text;
};

int window_comctl_set_text(long handle, char* text) {
    __window_comctl_set_text_struct__ s;
    s.handle = handle;
    s.text = text;
    &s;
    interrupt 510;
}

char* window_comctl_get_text(long handle) {
    &handle;
    interrupt 512;
}

struct __window_comctl_set_ptr_struct__ {
    long handle;
    char* ptr; int len;
};

int window_comctl_set_ptr(long handle, char* ptr, int len) {
    __window_comctl_set_ptr_struct__ s;
    s.handle = handle;
    s.ptr = ptr;
    s.len = len;
    &s;
    interrupt 511;
}

struct __window_get_comctl_struct__ {
    int handle;
    int comctl;
};

int window_get_comctl(long handle) {
    __window_get_comctl_struct__* s = &handle;
    return s->comctl;
}

int window_get_handle(long handle) {
    __window_get_comctl_struct__* s = &handle;
    return s->handle;
}

// ----------------------------------------
// LAYOUT

int window_layout_linear_set_vertical_align(long handle) { window_comctl_set_flag(handle, 0); }
int window_layout_linear_set_horizontal_align(long handle) { window_comctl_set_flag(handle, 1); }

// ----------------------------------------
// LABEL

int window_comctl_label_set_vertical_align_top(long handle) { window_comctl_set_flag(handle, 10); }
int window_comctl_label_set_vertical_align_middle(long handle) { window_comctl_set_flag(handle, 11); }
int window_comctl_label_set_vertical_align_bottom(long handle) { window_comctl_set_flag(handle, 12); }
int window_comctl_label_set_horizontal_align_left(long handle) { window_comctl_set_flag(handle, 13); }
int window_comctl_label_set_horizontal_align_middle(long handle) { window_comctl_set_flag(handle, 14); }
int window_comctl_label_set_horizontal_align_right(long handle) { window_comctl_set_flag(handle, 15); }

// ----------------------------------------
// IMAGE

int window_comctl_image_set_ptr_from_url(long handle, char* path) {
    window_comctl_set_ptr(handle, path, strlen(path));
}