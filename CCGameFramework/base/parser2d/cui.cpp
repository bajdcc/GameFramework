//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include "parser2d.h"

// 光栅化算法

bool Parser2DEngine::check_cord(int x, int y)
{
    return x > 0 && y > 0 && x <= rect.Height && y <= rect.Width;
}

void Parser2DEngine::move_to(int x, int y)
{
    if (check_cord(x, y)) {
        cur_pt.x = x;
        cur_pt.y = y;
    }
}

// CHECKED X, Y
bool Parser2DEngine::setpixel(int x, int y) {
    auto b = &buffer[(x * rect.Width + y) * 4];
    b[0] = cur_bursh.b;
    b[1] = cur_bursh.g;
    b[2] = cur_bursh.r;
    b[3] = cur_bursh.a;
    return true;
}

// 画直线
// Refer: https://zhuanlan.zhihu.com/p/30553006
// Modified from https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
void Parser2DEngine::bresenham(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (setpixel(x0, y0), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void Parser2DEngine::line_to(int x, int y)
{
    if (check_cord(x, y)) {
        bresenham(cur_pt.x, cur_pt.y, x, y);
        cur_pt.x = x;
        cur_pt.y = y;
    }
}