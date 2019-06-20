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

int gui_enable_fresh() {
    return gui_enable(5);
}
int gui_disable_fresh() {
    return gui_enable(4);
}
int gui_fresh() {
    gui_enable(6);
    gui_enable(7);
}
int gui_reset() {
    return gui_enable(8);
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

int gui_clear(int r, int g, int b) {
    0xff << 24 | r << 16 | g << 8 | b;
    interrupt 306;
}

int gui_rect(int x, int y) {
    x << 16 | y;
    interrupt 307;
}

int gui_font_refresh() {
    interrupt 308;
}

int gui_font_size(int size) {
    size;
    interrupt 309;
}

int gui_font_family(char *name) {
    name;
    interrupt 310;
}

int gui_draw_text(char* text) {
    text;
    interrupt 311;
}

int gui_lua(char* text) {
    text;
    interrupt 350;
}

int gui_hue2rgb(int n1, int n2, int hue)
{
    if (hue < 0)
        hue += 240;
    if (hue > 240)
        hue -= 240;
    if (hue < 60)
        return n1 + ((n2 - n1) * hue + 20) / 40;
    if (hue < 120)
        return n2;
    if (hue < 160)
        return n1 + ((n2 - n1) * (160 - hue) + 20) / 40;
    return n1;
}
int gui_hsl2rgb(int h, int s, int l) {
    int Magic1, Magic2;

    if (0 == s)
    {
        int k = (l * 255) / 240;
        return k | k << 8 | k << 16;
    }
    else
    {
        if (l <= 120)
            Magic2 = (l * (240 + s) + 120) / 240;
        else
            Magic2 = l + s - ((l * s) + 120) / 240;

        Magic1 = 2 * l - Magic2;

        int r = (gui_hue2rgb(Magic1, Magic2, h + 80) * 255 + 120) / 240;
        int g = (gui_hue2rgb(Magic1, Magic2, h) * 255 + 120) / 240;
        int b = (gui_hue2rgb(Magic1, Magic2, h - 80) * 255 + 120) / 240;

        return r << 16 | g << 8 | b;
    }
}