//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include "render/Direct2DRenderTarget.h"
#include "parser2d.h"
#include "cvm.h"

// 光栅化算法

bool Parser2DEngine::check_cord(int x, int y) const
{
    return bitmap && x >= 0 && y >= 0 && x < rect.Width && y < rect.Height;
}

bool Parser2DEngine::ready() const
{
    return bitmap != nullptr;
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
    if (!bitmap) return false;
    rt2->BeginDraw();
    rt2->DrawRectangle(D2D1::RectF((FLOAT)x, (FLOAT)y, (FLOAT)x, (FLOAT)y), cur_bursh);
    rt2->EndDraw();
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
        rt2->BeginDraw();
        rt2->DrawLine(D2D1::Point2F((FLOAT)cur_pt.x, (FLOAT)cur_pt.y),
            D2D1::Point2F((FLOAT)x, (FLOAT)y), cur_bursh);
        rt2->EndDraw();
        cur_pt.x = x;
        cur_pt.y = y;
    }
}

void Parser2DEngine::draw_point(int x, int y)
{
    if (check_cord(x, y)) {
        setpixel(x, y);
    }
}

int Parser2DEngine::get_width() const
{
    return rect.Width;
}

int Parser2DEngine::get_height() const
{
    return rect.Height;
}

void Parser2DEngine::set_color(uint c)
{
    if (!bitmap) return;
    cur_bursh = nullptr;
    auto hr = rt2->CreateSolidColorBrush(D2D1::ColorF(c & 0xffffff, ((FLOAT)(c >> 24)) / 255.0f), &cur_bursh);
    if (FAILED(hr))
        ATLVERIFY(!"CreateSolidColorBrush failed");
}

void Parser2DEngine::clear(uint c)
{
    if (!bitmap) return;
    rt2->BeginDraw();
    rt2->Clear(D2D1::ColorF(c & 0xffffff, ((FLOAT)(c >> 24)) / 255.0f));
    rt2->EndDraw();
}

void Parser2DEngine::fill_rect(int x, int y)
{
    if (check_cord(x, y)) {
        rt2->BeginDraw();
        rt2->FillRectangle(D2D1::RectF((FLOAT)cur_pt.x, (FLOAT)cur_pt.y, (FLOAT)x + 1, (FLOAT)y + 1), cur_bursh);
        rt2->EndDraw();
        cur_pt.x = x;
        cur_pt.y = y;
    }
}

int Parser2DEngine::set_fresh(int fresh)
{
    if (fresh == -1) return auto_fresh;
    auto_fresh = fresh;
    return fresh;
}

void Parser2DEngine::reset()
{
    auto_fresh = 1;
    clib::cvm::global_state.gui = false;
    rt2.Release();
    bitmap.Release();
    rect.Width = 0;
    rect.Height = 0;
    cur_bursh.Release();
}

void Parser2DEngine::create_font()
{
    if (d2drt.lock() && font.size > 0 && !font.fontFamily.IsEmpty()) {
        auto d = d2drt.lock();
        font_format = d->CreateDirect2DTextFormat(font);
        backup_font.fontFamily = font.fontFamily;
        backup_font.size = font.size;
        backup_font.bold = font.bold;
        backup_font.italic = font.italic;
        backup_font.underline = font.underline;
        backup_font.strikeline = font.strikeline;
        backup_font.antialias = font.antialias;
        backup_font.verticalAntialias = font.verticalAntialias;
    }
}

void Parser2DEngine::set_font_size(int size)
{
    if (size > 0 && size < 100)
        font.size = size;
}

void Parser2DEngine::set_font_family(const string_t& name)
{
    if (!name.empty())
        font.fontFamily = name.c_str();
}

void Parser2DEngine::draw_font(const string_t& text)
{
    if (!bitmap) return;
    CString t(CStringA(text.c_str()));
    rt2->BeginDraw();
    rt2->DrawText(t.GetBuffer(0), t.GetLength(), font_format->textFormat,
        D2D1::RectF((float)cur_pt.x, (float)cur_pt.y,
        (float)rect.Width, (float)rect.Height), cur_bursh);
    rt2->EndDraw();
}
