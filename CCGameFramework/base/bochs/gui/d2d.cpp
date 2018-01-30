#include "stdafx.h"
#include "d2d.h"
#include "gui.h"
#include "../bochs.h"
#include "../plugin.h"
#include "render/Direct2DRender.h"

static unsigned prev_cursor_x = 0;
static unsigned prev_cursor_y = 0;
static BYTE vgafont[256][64];
static int xChar = 8, yChar = 16;
static unsigned int text_rows = 25, text_cols = 80;
static Bit8u text_pal_idx[16];
static Bit8u h_panning = 0, v_panning = 0;
static Bit16u line_compare = 1023;
static RGBQUAD cmap_index[259];

static bx_d2d_gui_c *theGui = NULL;
IMPLEMENT_GUI_PLUGIN_CODE(d2d)

bx_d2d_gui_c::bx_d2d_gui_c()
{
    buffer = X86WindowElementRenderer::GetBuffer();
    size = X86WindowElementRenderer::GetSize();
}

void bx_d2d_gui_c::specific_init(int argc, char** argv, unsigned header_bar_y)
{
    memset(cmap_index, 0, sizeof(cmap_index));
    buffer = nullptr;
    size.cx = size.cy = 0;
    error = FALSE;
}

void bx_d2d_gui_c::text_update(Bit8u* old_text, Bit8u* new_text, unsigned long cursor_x, unsigned long cursor_y,
    bx_vga_tminfo_t* tm_info)
{
    unsigned char data[64];
    Bit8u *old_line, *new_line;
    Bit8u cAttr, cChar;
    unsigned int curs, hchars, i, offset, rows, x, y, xc, yc;
    BOOL forceUpdate = FALSE, blink_state, blink_mode;
    Bit8u *text_base;
    Bit8u cfwidth, cfheight, cfheight2, font_col, font_row, font_row2;
    Bit8u split_textrow, split_fontrows;
    unsigned int yc2, cs_y;
    BOOL split_screen;

    blink_mode = (tm_info->blink_flags & BX_TEXT_BLINK_MODE) > 0;
    blink_state = (tm_info->blink_flags & BX_TEXT_BLINK_STATE) > 0;
    if (blink_mode) {
        if (tm_info->blink_flags & BX_TEXT_BLINK_TOGGLE)
            forceUpdate = 1;
    }
    if (charmap_updated) {
        for (unsigned c = 0; c<256; c++) {
            if (char_changed[c]) {
                memset(data, 0, sizeof(data));
                BOOL gfxchar = tm_info->line_graphics && ((c & 0xE0) == 0xC0);
                for (i = 0; i<(unsigned)yChar; i++) {
                    data[i * 2] = vga_charmap[c * 32 + i];
                    if (gfxchar) {
                        data[i * 2 + 1] = (data[i * 2] << 7);
                    }
                }
                memcpy(vgafont[c], data, 64);
                char_changed[c] = 0;
            }
        }
        forceUpdate = TRUE;
        charmap_updated = 0;
    }
    for (i = 0; i<16; i++) {
        text_pal_idx[i] = tm_info->actl_palette[i];
    }

    if ((tm_info->h_panning != h_panning) || (tm_info->v_panning != v_panning)) {
        forceUpdate = 1;
        h_panning = tm_info->h_panning;
        v_panning = tm_info->v_panning;
    }
    if (tm_info->line_compare != line_compare) {
        forceUpdate = 1;
        line_compare = tm_info->line_compare;
    }

    // first invalidate character at previous and new cursor location
    if ((prev_cursor_y < text_rows) && (prev_cursor_x < text_cols)) {
        curs = prev_cursor_y * tm_info->line_offset + prev_cursor_x * 2;
        old_text[curs] = ~new_text[curs];
    }
    if ((tm_info->cs_start <= tm_info->cs_end) && (tm_info->cs_start < yChar) &&
        (cursor_y < text_rows) && (cursor_x < text_cols)) {
        curs = cursor_y * tm_info->line_offset + cursor_x * 2;
        old_text[curs] = ~new_text[curs];
    }
    else {
        curs = 0xffff;
    }

    rows = text_rows;
    if (v_panning) rows++;
    y = 0;
    cs_y = 0;
    text_base = new_text - tm_info->start_address;
    if (line_compare < size.cx) {
        split_textrow = (line_compare + v_panning) / yChar;
        split_fontrows = ((line_compare + v_panning) % yChar) + 1;
    }
    else {
        split_textrow = rows + 1;
        split_fontrows = 0;
    }
    split_screen = 0;
    do {
        hchars = text_cols;
        if (h_panning) hchars++;
        if (split_screen) {
            yc = line_compare + cs_y * yChar + 1;
            font_row = 0;
            if (rows == 1) {
                cfheight = ((Bit16u)size.cx - line_compare - 1) % yChar;
                if (cfheight == 0) cfheight = yChar;
            }
            else {
                cfheight = yChar;
            }
        }
        else if (v_panning) {
            if (y == 0) {
                yc = 0;
                font_row = v_panning;
                cfheight = yChar - v_panning;
            }
            else {
                yc = y * yChar - v_panning;
                font_row = 0;
                if (rows == 1) {
                    cfheight = v_panning;
                }
                else {
                    cfheight = yChar;
                }
            }
        }
        else {
            yc = y * yChar;
            font_row = 0;
            cfheight = yChar;
        }
        if (!split_screen && (y == split_textrow)) {
            if (split_fontrows < cfheight) cfheight = split_fontrows;
        }
        new_line = new_text;
        old_line = old_text;
        x = 0;
        offset = cs_y * tm_info->line_offset;
        do {
            if (h_panning) {
                if (hchars > text_cols) {
                    xc = 0;
                    font_col = h_panning;
                    cfwidth = xChar - h_panning;
                }
                else {
                    xc = x * xChar - h_panning;
                    font_col = 0;
                    if (hchars == 1) {
                        cfwidth = h_panning;
                    }
                    else {
                        cfwidth = xChar;
                    }
                }
            }
            else {
                xc = x * xChar;
                font_col = 0;
                cfwidth = xChar;
            }
            if (forceUpdate || (old_text[0] != new_text[0])
                || (old_text[1] != new_text[1])) {
                cChar = new_text[0];
                if (blink_mode) {
                    cAttr = new_text[1] & 0x7F;
                    if (!blink_state && (new_text[1] & 0x80))
                        cAttr = (cAttr & 0x70) | (cAttr >> 4);
                }
                else {
                    cAttr = new_text[1];
                }
                {
                    auto hh = min(cfheight, 16);
                    auto ww = min(cfwidth, 8);
                    auto fg = (cAttr) & 0xf;
                    auto bg = (cAttr >> 4) & 0xf;
                    auto fc = cmap_index[(BYTE)text_pal_idx[fg]];
                    auto bc = cmap_index[(BYTE)text_pal_idx[bg]];
                    for (auto i = 0; i < hh; i++)
                    {
                        auto read = &buffer[4 * (xc + size.cx * (yc + i))];
                        for (auto j = 0; j < ww; j++)
                        {
                            const auto hit = (vgafont[cChar][i * 2] >> (ww - j - 1)) & 1;
                            const auto clr = hit == 1 ? fc : bc;
                            read[0] = clr.rgbBlue;
                            read[1] = clr.rgbGreen;
                            read[2] = clr.rgbRed;
                            read[3] = 255;
                            read += 4;
                        }
                    }
                }
                if (offset == curs) {
                    if (font_row == 0) {
                        yc2 = yc + tm_info->cs_start;
                        font_row2 = tm_info->cs_start;
                        cfheight2 = tm_info->cs_end - tm_info->cs_start + 1;
                    }
                    else {
                        if (v_panning > tm_info->cs_start) {
                            yc2 = yc;
                            font_row2 = font_row;
                            cfheight2 = tm_info->cs_end - v_panning + 1;
                        }
                        else {
                            yc2 = yc + tm_info->cs_start - v_panning;
                            font_row2 = tm_info->cs_start;
                            cfheight2 = tm_info->cs_end - tm_info->cs_start + 1;
                        }
                    }
                    cAttr = ((cAttr >> 4) & 0xF) + ((cAttr & 0xF) << 4);
                    {
                        auto hh = min(cfheight2, 16);
                        auto ww = min(cfwidth, 8);
                        auto fg = (cAttr) & 0xf;
                        auto bg = (cAttr >> 4) & 0xf;
                        auto fc = cmap_index[(BYTE)text_pal_idx[fg]];
                        auto bc = cmap_index[(BYTE)text_pal_idx[bg]];
                        for (auto i = 0; i < hh; i++)
                        {
                            auto read = &buffer[4 * (xc + size.cx * (yc2 + i))];
                            for (auto j = 0; j < ww; j++)
                            {
                                const auto hit = (vgafont[cChar][i * 2] >> (ww - j - 1)) & 1;
                                const auto clr = hit == 1 ? fc : bc;
                                read[0] = clr.rgbBlue;
                                read[1] = clr.rgbGreen;
                                read[2] = clr.rgbRed;
                                read[3] = 255;
                                read += 4;
                            }
                        }
                    }
                }
            }
            x++;
            new_text += 2;
            old_text += 2;
            offset += 2;
        } while (--hchars);
        if (!split_screen && (y == split_textrow)) {
            new_text = text_base;
            forceUpdate = 1;
            cs_y = 0;
            if (tm_info->split_hpanning) h_panning = 0;
            rows = ((size.cx - line_compare + yChar - 2) / yChar) + 1;
            split_screen = 1;
        }
        else {
            y++;
            cs_y++;
            new_text = new_line + tm_info->line_offset;
            old_text = old_line + tm_info->line_offset;
        }
    } while (--rows);

    h_panning = tm_info->h_panning;

    prev_cursor_x = cursor_x;
    prev_cursor_y = cursor_y;
}

void bx_d2d_gui_c::graphics_tile_update(Bit8u* tile, unsigned x, unsigned y)
{
}

void bx_d2d_gui_c::handle_events()
{
}

void bx_d2d_gui_c::flush()
{
}

void bx_d2d_gui_c::clear_screen()
{
    for (DWORD *read = (DWORD*)buffer, *read_end = read + (size.cx * size.cy); read != read_end; read++)
        *read = 0xFF000000;
}

bx_bool bx_d2d_gui_c::palette_change(Bit8u index, Bit8u red, Bit8u green, Bit8u blue)
{
    cmap_index[index].rgbRed = red;
    cmap_index[index].rgbBlue = blue;
    cmap_index[index].rgbGreen = green;
    return BX_TRUE;
}

void bx_d2d_gui_c::dimension_update(unsigned x, unsigned y, unsigned fheight, unsigned fwidth, unsigned bpp)
{
    size = CSize(x, y);
    error = X86WindowElementRenderer::SetSize(CSize(x, y));
    buffer = X86WindowElementRenderer::GetBuffer();
}

unsigned bx_d2d_gui_c::create_bitmap(const unsigned char* bmap, unsigned xdim, unsigned ydim)
{
    return 0;
}

unsigned bx_d2d_gui_c::headerbar_bitmap(unsigned bmap_id, unsigned alignment, void(* f)())
{
    return 0;
}

void bx_d2d_gui_c::replace_bitmap(unsigned hbar_id, unsigned bmap_id)
{
}

void bx_d2d_gui_c::show_headerbar()
{
}

int bx_d2d_gui_c::get_clipboard_text(Bit8u** bytes, Bit32s* nbytes)
{
    return 0;
}

int bx_d2d_gui_c::set_clipboard_text(char* snapshot, Bit32u len)
{
    return 0;
}

void bx_d2d_gui_c::exit()
{
}

void bx_d2d_gui_c::statusbar_setitem_specific(int element, bx_bool active, bx_bool w)
{
}

void bx_d2d_gui_c::mouse_enabled_changed_specific(bx_bool val)
{
}

void bx_d2d_gui_c::get_capabilities(Bit16u *xres, Bit16u *yres, Bit16u *bpp)
{
    auto _size = X86WindowElementRenderer::GetSize();
    *xres = (Bit16u)_size.cx;
    *yres = (Bit16u)_size.cy;
    *bpp = 32;
}

void bx_d2d_gui_c::set_tooltip(unsigned hbar_id, const char *tip)
{
}

void bx_d2d_gui_c::set_mouse_mode_absxy(bx_bool mode)
{
}

#if BX_SHOW_IPS
void bx_d2d_gui_c::show_ips(Bit32u ips_count)
{
}
#endif