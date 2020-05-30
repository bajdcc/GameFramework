//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cjsgui.h"
#include "../parser2d/cnet.h"
#include "ui/window/Window.h"

#ifdef REPORT_ERROR
#undef REPORT_ERROR
#endif
#define REPORT_ERROR 1
#ifdef REPORT_ERROR_FILE
#undef REPORT_ERROR_FILE
#endif
#define REPORT_ERROR_FILE "js_runtime.log"
#define REPORT_STAT 1
#define REPORT_STAT_FILE "stat.log"
#define STAT_DELAY_N 60
#define STAT_MAX_N 25

#define AST_FILE "js_ast.log"

#define LOG_AST 0
#define LOG_DEP 0

#define IPS_STAT_TIME 1s

#define ENTRY_FILE "src/main.js"

#define MAKE_ARGB(a,r,g,b) ((uint32_t)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define MAKE_RGB(r,g,b) MAKE_ARGB(255,r,g,b)
#define GET_R(rgb) (LOBYTE(rgb))
#define GET_G(rgb) (LOBYTE(((WORD)(rgb)) >> 8))
#define GET_B(rgb) (LOBYTE((WORD)((rgb)>>16)))
#define GET_A(rgb) (LOBYTE((rgb)>>24))

extern int g_argc;
extern char** g_argv;

namespace clib {

    cjsgui::cjsgui() {
        std::fill(screen_ref.begin(), screen_ref.end(), 0);
        init_screen(0);
        switch_screen(0);
        switch_screen_display(0);
    }

    cjsgui& cjsgui::singleton() {
        static clib::cjsgui gui;
        return gui;
    }

    void cjsgui::reset() {
        if (vm) {
            vm.reset();
            std::fill(screens.begin(), screens.end(), nullptr);
            for (auto& s : screens) {
                s.reset(nullptr);
            }
            init_screen(0);
            screen_ptr = -1;
            switch_screen(0);
            screen_id = -1;
            switch_screen_display(0);
            std::fill(screen_ref.begin(), screen_ref.end(), 0);
            screen_interrupt.clear();
            reset_cycles();
            reset_ips();
        }
        running = false;
        exited = false;
    }

    void cjsgui::clear_cache()
    {
        cache.clear();
        cache_code.clear();
        cache_dep.clear();
    }

    void cjsgui::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const JS2DEngine::BrushBag& brushes, bool paused, decimal fps) {
        if (!paused && !entered) {
            if (global_state.input->interrupt) {
                cycle = GUI_CYCLES;
            }
            else if (cycle_set) {
                // ...
            }
            else if (cycle_stable > 0) {
                if (fps > GUI_MAX_FPS_RATE) {
                    cycle = min(cycle << 1, GUI_MAX_CYCLE);
                }
                else if (fps < GUI_MIN_FPS_RATE) {
                    cycle_stable--;
                }
            }
            else if (fps > GUI_MAX_FPS_RATE) {
                if (cycle_speed >= 0) {
                    cycle_speed = min(cycle_speed + 1, GUI_MAX_SPEED);
                    cycle = min(cycle << cycle_speed, GUI_MAX_CYCLE);
                }
                else {
                    cycle_speed = 0;
                }
            }
            else if (fps < GUI_MIN_FPS_RATE) {
                if (cycle_speed <= 0) {
                    cycle_speed = max(cycle_speed - 1, -GUI_MAX_SPEED);
                    cycle = max(cycle >> (-cycle_speed), GUI_MIN_CYCLE);
                }
                else {
                    cycle_speed = 0;
                }
            }
            else {
                if (cycle_stable == 0) {
                    cycle_speed = 0;
                    cycle_stable = GUI_CYCLE_STABLE;
                }
            }
            entered = true;
            for (int i = 0; i < ticks + cycle_speed; ++i) {
                tick();
            }
            entered = false;
        }
        using namespace std::chrono_literals;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - last_time) >= IPS_STAT_TIME) {
            last_time = std::chrono::system_clock::now();
            reset_ips();
        }
        draw_text(rt, bounds, brushes);
        draw_window(bounds);
    }

    void cjsgui::draw_text(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const JS2DEngine::BrushBag& brushes) {
        if (!screens[screen_id])
            return;
        auto& scr = *screens[screen_id].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        int w = bounds.Width();
        int h = bounds.Height();
        int width = cols * GUI_FONT_W;
        int height = rows * GUI_FONT_H;

        int x = max((w - width) / 2, 0);
        int y = max((h - height) / 2, 0);
        auto old_x = x;
        char c;

        CComPtr<ID2D1SolidColorBrush> b;
        TCHAR s[2] = { 0 };
        char sc[3] = { 0 };
        bool ascii = true;
        bool ascii_head = false;

        for (auto i = 0; i < rows; ++i) {
            for (auto j = 0; j < cols; ++j) {
                ascii = true;
                c = buffer[i * cols + j];
                if (c < 0 && (i != rows - 1 || j != cols - 1)) {
                    WORD wd = (((BYTE)c) << 8) | ((BYTE)buffer[i * cols + j + 1]);
                    if (wd >= 0x8140 && wd <= 0xFEFE) { // GBK
                        if (j < cols - 1)
                            ascii = false;
                        else
                            ascii_head = true;
                    }
                }
                if (j == 0 && ascii_head) {
                    ascii_head = false;
                    ascii = true;
                }
                if (ascii) {
                    if (colors_bg[i * cols + j]) {
                        rt->CreateSolidColorBrush(D2D1::ColorF(colors_bg[i * cols + j]), &b);
                        rt->FillRectangle(
                            D2D1::RectF((float)bounds.left + x, (float)bounds.top + y + GUI_FONT_H_1,
                            (float)bounds.left + x + GUI_FONT_W, (float)bounds.top + y + GUI_FONT_H_2), b);
                        b.Release();
                    }
                    if (c > 0) {
                        if (std::isprint(buffer[i * cols + j])) {
                            rt->CreateSolidColorBrush(D2D1::ColorF(colors_fg[i * cols + j]), &b);
                            s[0] = c;
                            rt->DrawText(s, 1, brushes.cmdTF->textFormat,
                                D2D1::RectF((float)bounds.left + x, (float)bounds.top + y + GUI_FONT_H_1,
                                (float)bounds.left + x + GUI_FONT_W, (float)bounds.top + y + GUI_FONT_H_2), b);
                            b.Release();
                        }
                        else if (c == '\7') {
                            rt->CreateSolidColorBrush(D2D1::ColorF(colors_fg[i * cols + j]), &b);
                            rt->FillRectangle(
                                D2D1::RectF((float)bounds.left + x, (float)bounds.top + y + GUI_FONT_H_1,
                                (float)bounds.left + x + GUI_FONT_W, (float)bounds.top + y + GUI_FONT_H_2), b);
                            b.Release();
                        }
                    }
                    else if (c < 0) {
                        rt->CreateSolidColorBrush(D2D1::ColorF(colors_fg[i * cols + j]), &b);
                        rt->FillRectangle(
                            D2D1::RectF((float)bounds.left + x, (float)bounds.top + y + GUI_FONT_H_1,
                            (float)bounds.left + x + GUI_FONT_W, (float)bounds.top + y + GUI_FONT_H_2), b);
                        b.Release();
                    }
                    x += GUI_FONT_W;
                }
                else {
                    if (colors_bg[i * cols + j]) {
                        rt->CreateSolidColorBrush(D2D1::ColorF(colors_bg[i * cols + j]), &b);
                        rt->FillRectangle(
                            D2D1::RectF((float)bounds.left + x, (float)bounds.top + y + GUI_FONT_H_1,
                            (float)bounds.left + x + GUI_FONT_W * 2, (float)bounds.top + y + GUI_FONT_H_2), b);
                        b.Release();
                    }
                    sc[0] = c;
                    sc[1] = buffer[i * cols + j + 1];
                    auto utf = cnet::GBKToStringT(sc);
                    s[0] = (TCHAR)(utf[0]);
                    j++;
                    rt->CreateSolidColorBrush(D2D1::ColorF(colors_fg[i * cols + j]), &b);
                    rt->DrawText(s, 1, brushes.gbkTF->textFormat,
                        D2D1::RectF((float)bounds.left + x + GUI_FONT_W_C1, (float)bounds.top + y + GUI_FONT_H_C1,
                        (float)bounds.left + x + GUI_FONT_W_C2, (float)bounds.top + y + GUI_FONT_H_C2), b);
                    b.Release();
                    x += GUI_FONT_W * 2;
                }
            }
            x = old_x;
            y += GUI_FONT_H;
            if (y + GUI_FONT_H >= bounds.Height()) {
                break;
            }
        }

        if (input_state) {
            input_ticks++;
            if (input_ticks > GUI_INPUT_CARET) {
                input_caret = !input_caret;
                input_ticks = 0;
            }
            if (input_caret) {
                if (ptr_x <= cols - 1) {
                    int _x = max((w - width) / 2, 0) + ptr_x * GUI_FONT_W;
                    int _y = max((h - height) / 2, 0) + ptr_y * GUI_FONT_H;
                    rt->CreateSolidColorBrush(D2D1::ColorF(color_fg_stack.back()), &b);
                    rt->DrawText(_T("_"), 1, brushes.cmdTF->textFormat,
                        D2D1::RectF((float)bounds.left + _x, (float)bounds.top + _y + GUI_FONT_H_1,
                        (float)bounds.left + _x + GUI_FONT_W, (float)bounds.top + _y + GUI_FONT_H_2), b);
                    b.Release();
                }
                else if (ptr_y < rows - 1) {
                    int _x = max((w - width) / 2, 0);
                    int _y = max((h - height) / 2, 0) + (ptr_y + 1) * GUI_FONT_H;
                    rt->CreateSolidColorBrush(D2D1::ColorF(color_fg_stack.back()), &b);
                    rt->DrawText(_T("_"), 1, brushes.cmdTF->textFormat,
                        D2D1::RectF((float)bounds.left + _x, (float)bounds.top + _y + GUI_FONT_H_1,
                        (float)bounds.left + _x + GUI_FONT_W, (float)bounds.top + _y + GUI_FONT_H_2), b);
                    b.Release();
                }
            }
        }
    }

    void cjsgui::draw_window(const CRect& bounds)
    {
        if (vm)
            vm->paint_window(bounds);
    }

    void cjsgui::reset_ips()
    {
        if (vm)
            vm->reset_ips();
    }

    void cjsgui::tick() {
        if (exited)
            return;
        if (running) {
            if (vm->get_state() == 1) {
                vm->set_state(2);
                std::vector<std::string> args;
                if (g_argc > 0) {
                    args.emplace_back(ENTRY_FILE);
                    for (int i = 1; i < g_argc; ++i) {
                        args.emplace_back(g_argv[i]);
                    }
                }
                if (compile(ENTRY_FILE, args, decltype(args)()) == -1) {
                    running = false;
                }
            }
            try {
                if (!vm->run(cycle, cycles)) {
                    running = false;
                    exited = true;
                    put_string("\n[!] clibjs exited.");
                    vm.reset();
                }
            }
            catch (const cjs_exception& e) {
                ATLTRACE("[SYSTEM] ERR  | RUNTIME ERROR: %s\n", e.message().c_str());
#if REPORT_ERROR
                {
                    std::ofstream log(REPORT_ERROR_FILE, std::ios::app | std::ios::out);
                    log << "[SYSTEM] ERR  | RUNTIME ERROR: " << e.message() << std::endl;
                }
#endif
                exited = true;
                running = false;
                //vm.reset();
                //gen.reset();
                //running = false;
            }
        }
        else {
            if (!vm) {
                vm = std::make_unique<cjs>();
                running = true;
            }
        }
    }

    void cjsgui::put_string(const std::string& str) {
        for (auto& s : str) {
            put_char(s);
        }
    }

    void cjsgui::put_char(int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& cmd_state = scr.cmd_state;
        auto& cmd_string = scr.cmd_string;
        auto& size = scr.size;

        if (cmd_state) {
            if (c == '\033') {
                static std::string pat{ R"([A-Za-z][0-9a-f]{1,8})" };
                static std::regex re(pat);
                std::smatch res;
                std::string s(cmd_string.begin(), cmd_string.end());
                if (std::regex_search(s, res, re)) {
                    try {
                        exec_cmd(s);
                    }
                    catch (const std::invalid_argument&) {
                        // '/dev/random' : cause error
                    }
                }
                cmd_string.clear();
                cmd_state = false;
            }
            else {
                cmd_string.push_back(c);
            }
            return;
        }
        else if (c == '\033') {
            cmd_state = true;
            return;
        }
        if (c == 0)
            return;
        if (c == '\n') {
            ptr_x = 0;
            if (ptr_y == rows - 1) {
                new_line();
            }
            else {
                ptr_y++;
            }
        }
        else if (c == '\b') {
            auto ascii = true;
            if (ptr_x != 0 || ptr_y != 0) {
                auto cc = buffer[ptr_y * cols + ptr_x - 1];
                if (cc < 0) {
                    WORD wd = (((BYTE)cc) << 8) | ((BYTE)buffer[ptr_y * cols + ptr_x]);
                    if (wd >= 0x8140 && wd <= 0xFEFE) { // GBK
                        ascii = false;
                    }
                }
            }
            if (ptr_mx == -1 && ptr_my == -1 && ptr_x > 0) {
                forward(ptr_x, ptr_y, false);
                draw_char('\u0000');
                if (!ascii) {
                    forward(ptr_x, ptr_y, false);
                    draw_char('\u0000');
                }
            }
            else {
                if (ptr_mx + ptr_my * cols < ptr_x + ptr_y * cols) {
                    forward(ptr_x, ptr_y, false);
                    draw_char('\u0000');
                    if (!ascii) {
                        forward(ptr_x, ptr_y, false);
                        draw_char('\u0000');
                    }
                    if (!(ptr_x == ptr_rx && ptr_y == ptr_ry)) {
                        for (auto i = ptr_y * cols + ptr_x; i < ptr_ry * cols + ptr_rx; ++i) {
                            buffer[i] = buffer[i + 1];
                            colors_bg[i] = colors_bg[i + 1];
                            colors_fg[i] = colors_fg[i + 1];
                        }
                        buffer[ptr_ry * cols + ptr_rx] = '\0';
                        colors_bg[ptr_ry * cols + ptr_rx] = color_bg;
                        colors_fg[ptr_ry * cols + ptr_rx] = color_fg;
                    }
                    forward(ptr_rx, ptr_ry, false);
                    if (!ascii) {
                        forward(ptr_rx, ptr_ry, false);
                    }
                }
            }
        }
        else if (c == 0xff) {
            if (ptr_rx + ptr_ry * cols > ptr_x + ptr_y * cols) {
                move(false);
                put_char('\b');
            }
        }
        else if (c == '\u0002') {
            ptr_x--;
            while (ptr_x >= 0) {
                draw_char('\u0000');
                ptr_x--;
            }
            ptr_x = 0;
        }
        else if (c == '\r') {
            ptr_x = 0;
        }
        else if (c == '\f') {
            ptr_x = 0;
            ptr_y = 0;
            ptr_mx = 0;
            ptr_my = 0;
            ptr_rx = 0;
            ptr_ry = 0;
            std::fill(buffer.begin(), buffer.end(), 0);
            std::fill(colors_bg.begin(), colors_bg.end(), color_bg);
            std::fill(colors_fg.begin(), colors_fg.end(), color_fg);
        }
        else {
            auto rx = ptr_rx == -1 ? ptr_x : ptr_rx;
            auto ry = ptr_ry == -1 ? ptr_y : ptr_ry;
            auto end = rx == cols - 1 && ry == rows - 1;
            auto nl = rx == ptr_x && rx == cols - 1;
            if (end) {
                if (nl) {
                    draw_char(c);
                    new_line();
                    ptr_x = 0;
                }
                else {
                    new_line();
                    ptr_y--;
                    draw_char(c);
                    ptr_x++;
                }
            }
            else {
                draw_char(c);
                forward(ptr_x, ptr_y, true);
            }
        }
    }

    void cjsgui::input_call(int c) {
        auto old = screen_ptr;
        screen_ptr = screen_id;
        input(c);
        screen_ptr = old;
    }

    void cjsgui::new_line() {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        if (ptr_my != -1) {
            if (ptr_my == 0) {
                ptr_mx = ptr_my = 0;
            }
            else {
                ptr_my--;
            }
        }
        memcpy(buffer.data(), buffer.data() + cols, (uint)cols * (rows - 1));
        memset(buffer.data() + cols * (rows - 1), 0, (uint)cols);
        memcpy(colors_bg.data(), colors_bg.data() + cols, (uint)cols * (rows - 1) * sizeof(uint32_t));
        memcpy(colors_fg.data(), colors_fg.data() + cols, (uint)cols * (rows - 1) * sizeof(uint32_t));
    }

    void cjsgui::draw_char(const char& c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& size = scr.size;

        if (input_state && c) {
            forward(ptr_rx, ptr_ry, true);
            if (!(ptr_x == ptr_rx && ptr_y == ptr_ry)) {
                for (auto i = ptr_ry * cols + ptr_rx; i > ptr_y * cols + ptr_x; --i) {
                    buffer[i] = buffer[i - 1];
                    colors_bg[i] = colors_bg[i - 1];
                    colors_fg[i] = colors_fg[i - 1];
                }
            }
        }
        buffer[ptr_y * cols + ptr_x] = c;
        colors_bg[ptr_y * cols + ptr_x] = color_bg;
        colors_fg[ptr_y * cols + ptr_x] = color_fg;
    }

    void cjsgui::error(const std::string& str) {
        throw cjs_exception(str);
    }

    void cjsgui::set_cycle(int cycle) {
        if (cycle == 0) {
            cycle_set = false;
            this->cycle = GUI_CYCLES;
        }
        else {
            cycle_set = true;
            this->cycle = cycle;
        }
    }

    void cjsgui::set_ticks(int ticks) {
        this->ticks = ticks;
    }

    void cjsgui::move(bool left) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        if (left) {
            if (ptr_mx + ptr_my * cols < ptr_x + ptr_y * cols) {
                forward(ptr_x, ptr_y, false);
            }
        }
        else {
            if (ptr_x + ptr_y * cols < ptr_rx + ptr_ry * cols) {
                forward(ptr_x, ptr_y, true);
            }
        }
    }

    void cjsgui::forward(int& x, int& y, bool forward) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& size = scr.size;

        if (forward) {
            if (x == cols - 1) {
                x = 0;
                if (y != rows - 1) {
                    y++;
                }
            }
            else {
                x++;
            }
        }
        else {
            if (y == 0) {
                if (x != 0) {
                    x--;
                }
            }
            else {
                if (x != 0) {
                    x--;
                }
                else {
                    x = cols - 1;
                    y--;
                }
            }
        }
    }

    std::string cjsgui::input_buffer() const {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        auto begin = ptr_mx + ptr_my * cols;
        auto end = ptr_x + ptr_y * cols;
        std::stringstream ss;
        for (int i = begin; i <= end; ++i) {
            if (buffer[i])
                ss << buffer[i];
        }
        return ss.str();
    }

    bool cjsgui::init_screen(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (screens[n])
            return false;
        screens[n] = std::make_unique<screen_t>();
        auto& scr = *screens[n].get();
        auto& input = scr.input;
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& size = scr.size;
        input.id = n;
        buffer.resize(size);
        std::fill(buffer.begin(), buffer.end(), 0);
        colors_bg.resize(size);
        color_bg = 0;
        std::fill(colors_bg.begin(), colors_bg.end(), color_bg);
        colors_fg.resize(size);
        color_fg = MAKE_RGB(255, 255, 255);
        std::fill(colors_fg.begin(), colors_fg.end(), color_fg);
        color_bg_stack.push_back(color_bg);
        color_fg_stack.push_back(color_fg);
        if (vm) {
            CString s;
            s.Format(L"初始化屏幕（%d）", n);
            vm->add_stat(s);
        }
        return true;
    }

    bool cjsgui::switch_screen_display(int n)
    {
        if (screen_id == n)
            return true;
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (!screens[n])
            return false;
        screen_id = n;
        global_state.input_s = &screens[n]->input;
        return true;
    }

    int cjsgui::current_screen() const
    {
        return screen_id;
    }

    void cjsgui::screen_ref_add(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return;
        if (!screens[n])
            return;
        screen_ref[n]++;
    }

    void cjsgui::screen_ref_dec(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return;
        if (!screens[n])
            return;
        screen_ref[n]--;
        if (n != 0 && screen_ref[n] <= 0) {
            if (screen_id == n) {
                switch_screen_display(0);
            }
            screens[n].reset(nullptr);
            if (vm) {
                CString s;
                s.Format(L"关闭屏幕（%d）", n);
                vm->add_stat(s);
            }
        }
    }

    cjsgui::global_input_t* cjsgui::get_screen_interrupt()
    {
        if (screen_interrupt.empty())
        return nullptr;
        auto n = screen_interrupt.back();
        screen_interrupt.pop_back();
        return &screens[n]->input;
    }

    cjsgui::global_state_t& cjsgui::get_global()
    {
        return global_state;
    }

    bool cjsgui::switch_screen(int n)
    {
        if (screen_ptr == n)
            return true;
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (!screens[n])
            return false;
        screen_ptr = n;
        global_state.input = &screens[n]->input;
        return true;
    }

    void cjsgui::resize(int r, int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& size = scr.size;

        if (r == 0 && c == 0) {
            r = GUI_ROWS;
            c = GUI_COLS;
        }
        auto old_rows = rows;
        auto old_cols = cols;
        rows = max(10, min(r, 60));
        cols = max(20, min(c, 200));
        ATLTRACE("[SYSTEM] GUI  | Resize: from (%d, %d) to (%d, %d)\n", old_rows, old_cols, rows, cols);
        size = rows * cols;
        auto old_buffer = buffer;
        buffer.resize(size);
        std::fill(buffer.begin(), buffer.end(), 0);
        auto old_bg = colors_bg;
        colors_bg.resize(size);
        std::fill(colors_bg.begin(), colors_bg.end(), color_bg);
        auto old_fg = colors_fg;
        colors_fg.resize(size);
        std::fill(colors_fg.begin(), colors_fg.end(), color_fg);
        auto min_rows = min(old_rows, rows);
        auto min_cols = min(old_cols, cols);
        auto delta_rows = old_rows - min_rows;
        for (int i = 0; i < min_rows; ++i) {
            for (int j = 0; j < min_cols; ++j) {
                buffer[i * cols + j] = old_buffer[(delta_rows + i) * old_cols + j];
                colors_bg[i * cols + j] = old_bg[(delta_rows + i) * old_cols + j];
                colors_fg[i * cols + j] = old_fg[(delta_rows + i) * old_cols + j];
            }
        }
        ptr_x = min(ptr_x, cols);
        ptr_y = min(ptr_y, rows);
        ptr_mx = min(ptr_mx, cols);
        ptr_my = min(ptr_my, rows);
        ptr_rx = min(ptr_rx, cols);
        ptr_ry = min(ptr_ry, rows);
    }

    CSize cjsgui::get_size() const {
        auto& scr = *screens[screen_ptr].get();
        return { scr.cols * GUI_FONT_W, scr.rows * GUI_FONT_H };
    }

    std::unordered_set<std::string> cjsgui::get_dep(std::string& path) const
    {
        auto f = cache_code.find(path);
        if (f != cache_code.end()) {
            return cache_dep.at(path);
        }
        return std::unordered_set<std::string>();
    }

    CString cjsgui::get_disp(types::disp_t t) const
    {
        static TCHAR sz[256];
        std::wstringstream ss;
        switch (t) {
        case D_PS:
            break;
        case D_HTOP:
            break;
        case D_HANDLE:
            break;
        case D_WINDOW:
            break;
        case D_MEM:
            break;
        case D_STAT: {
            if (stat_n > 0) {
                (*const_cast<int*>(&stat_n))--;
                if (stat_s.empty()) {
                    (*const_cast<int*>(&stat_n)) = 0;
                }
                else {
                    ss << (wchar_t)(L'0' + stat_s.size());
                    for (const auto& s : stat_s) {
                        ss << s.GetString() << std::endl;
                    }
                }
            }
            else {
                if (!stat_s.empty()) {
                    while (!stat_s.empty()) {
                        const_cast<std::list<CString>*>(&stat_s)->pop_back();
                    }
                }
                return L"";
            }
        }
                   break;
        default:
            break;
        }
        return CString(ss.str().c_str());
    }

    void cjsgui::add_stat(const CString& s, bool show)
    {
        if (show) {
            stat_n = STAT_DELAY_N;
            if (stat_s.size() >= STAT_MAX_N)
                stat_s.pop_front();
            stat_s.push_back(s);
        }
#if REPORT_STAT
        {
            std::ofstream log(REPORT_STAT_FILE, std::ios::app | std::ios::out);
            log << CStringA(s).GetBuffer(0) << std::endl;
        }
#endif
    }

    int cjsgui::new_screen(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return 1;
        if (screens[n])
            return 2;
        init_screen(n);
        return 0;
    }

    int cjsgui::compile(const std::string& path, const std::vector<std::string>& args, const std::vector<std::string>& paths) {
        if (path.empty())
            return -1;
        std::stringstream ss;
        ss << "sys.exec_file(\"" << path << "\", [";
        for (size_t i = 0; i < args.size(); i++) {
            ss << args[i];
            if (i + 1 != args.size()) {
                ss << ", ";
            }
        }
        ss << "]);";
        vm->exec("<entry>", ss.str());
        return 0;
    }

    void cjsgui::input_set(bool valid) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& size = scr.size;

        if (valid) {
            input_state = true;
            ptr_mx = ptr_x;
            ptr_my = ptr_y;
            ptr_rx = ptr_x;
            ptr_ry = ptr_y;
        }
        else {
            input_state = false;
            ptr_mx = -1;
            ptr_my = -1;
            ptr_rx = -1;
            ptr_ry = -1;
        }
        input_ticks = 0;
        input_caret = false;
    }

    extern const wchar_t* mapVirtKey[];

    void cjsgui::input(int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;
        auto& input_state = scr.input_state;
        auto& input_ticks = scr.input_ticks;
        auto& input_caret = scr.input_caret;
        auto& cmd_state = scr.cmd_state;
        auto& size = scr.size;

        {
            CString str;
            if (c & GUI_SPECIAL_MASK) {
                auto cc = c & 0xffff;
                cc = min(cc, 255);
                str.Format(L"屏幕（%d）键盘输入：%d 0x%x %s", screen_ptr, cc, cc, mapVirtKey[cc]);
                if (c & GUI_SPECIAL_MASK) {
                    auto cc = (c & 0xff) - VK_F1;
                    if (cc >= 0 && cc < (int)screens.size()) {
                        if (switch_screen_display(cc)) {
                            str.AppendFormat(L"，切换到屏幕（%d）成功", cc);
                        }
                        else {
                            str.AppendFormat(L"，切换到屏幕（%d）失败", cc);
                        }
                    }
                }
            }
            else
                str.Format(L"屏幕（%d）键盘输入：%d 0x%x %c", screen_ptr, c, c, isprint(c) ? wchar_t(c) : L'?');
            vm->add_stat(str);
        }
        if (c == 3) {
            screen_interrupt.push_back(screen_ptr);
            global_state.input_s->interrupt = true;
            cmd_state = false;
            if (input_state) {
                ptr_x = ptr_rx;
                ptr_y = ptr_ry;
                put_char('\n');
                global_state.input_s->input_content.clear();
                global_state.input_s->input_read_ptr = 0;
                global_state.input_s->input_success = true;
                global_state.input_s->input_code = 0;
                input_state = false;
                global_state.input_s->input_single = false;
            }
            if (screen_ptr == 0) {
                for (auto i = 1; i < (int)screens.size(); i++) {
                    if (screens[i]) {
                        screen_ptr = i;
                        input(c);
                        screens[i]->input.interrupt_force = true;
                    }
                }
                screen_ptr = 0;
            }
            return;
        }
        if (!input_state)
            return;
        if (global_state.input_s->input_single) {
            if (c > 0 && c < 256 && (std::isprint(c) || c == '\r')) {
                if (c == '\r')
                    c = '\n';
                put_char(c);
                ptr_x = ptr_rx;
                ptr_y = ptr_ry;
                std::string s;
                s += c;
                global_state.input_s->input_content = s;
                global_state.input_s->input_read_ptr = 0;
                global_state.input_s->input_success = true;
                global_state.input_s->input_code = 0;
                global_state.input_s->input_single = false;
                input_state = false;
            }
            else {
#if LOG_VM
                {
                    CStringA s; s.Format("[SYSTEM] GUI  | Input invalid single key: %d\n", c);
                    global_state.log_err.push_back(s.GetBuffer(0));
                    cvm::logging(CString(s));
                }
#endif
            }
            return;
        }
        if (c < 0) {
            put_char(c);
            return;
        }
        if (c < 0xffff && c > 0xff) {
            CString s;
            s.AppendChar(c);
            CStringA s2(s);
            put_char(s2[0]);
            put_char(s2[1]);
            return;
        }
        if (!((c & GUI_SPECIAL_MASK) || std::isprint(c) || c == '\b' || c == '\n' || c == '\r' || c == 4 || c == 7 || c == 26 || c == 22)) {
#if LOG_VM
            CStringA s; s.Format("[SYSTEM] GUI  | Input: %d\n", (int)c);
            global_state.log_info.push_back(s.GetBuffer(0));
#endif
            ATLTRACE("[SYSTEM] GUI  | Input: %d\n", (int)c);
            return;
        }
        if (c == '\b') {
            put_char('\b');
            return;
        }
        if (c == '\r' || c == 4 || c == 26) {
            ptr_x = ptr_rx;
            ptr_y = ptr_ry;
            put_char('\n');
            global_state.input_s->input_content = input_buffer();
            global_state.input_s->input_read_ptr = 0;
            global_state.input_s->input_success = true;
            global_state.input_s->input_code = 0;
            input_state = false;
            return;
        }
        if (c & GUI_SPECIAL_MASK) {
            char C = (char)-9;
            switch (c & 0xff) {
            case VK_LEFT:
                // C = (char) -12;
                // break;
                move(true);
                return;
            case VK_UP:
                C = (char)-10;
                break;
            case VK_RIGHT:
                // C = (char) -13;
                // break;
                move(false);
                return;
            case VK_DOWN:
                C = (char)-11;
                break;
            case VK_HOME:
                ptr_x = ptr_mx;
                ptr_y = ptr_my;
                return;
            case VK_END:
                ptr_x = ptr_rx;
                ptr_y = ptr_ry;
                return;
            case VK_DELETE:
                put_char(0xff);
                return;
            case VK_ESCAPE:
                return;
            case VK_SPACE:
                return;
            case VK_BACK:
                return;
            case VK_RETURN:
                input('\r');
                return;
            /*case 0x71: // SHIFT
                return;
            case 0x72: // CTRL
                return;
            case 0x74: // ALT
                return;*/
            case VK_SHIFT: // SHIFT
                return;
            case VK_CONTROL: // CTRL
                return;
            case VK_MENU: // ALT
                return;
            default:
#if LOG_VM
            {
                CStringA s; s.Format("[SYSTEM] GUI  | Input invalid special key: %d\n", c & 0xff);
                global_state.log_err.push_back(s.GetBuffer(0));
                cvm::logging(CString(s));
            }
#endif
            ATLTRACE("[SYSTEM] GUI  | Input invalid special key: %d\n", c & 0xff);
            return;
            }
            global_state.input_s->input_content = input_buffer();
            global_state.input_s->input_read_ptr = 0;
            global_state.input_s->input_success = true;
            global_state.input_s->input_code = C;
            input_state = false;
            auto begin = ptr_mx + ptr_my * cols;
            auto end = ptr_x + ptr_y * cols;
            for (int i = begin; i <= end; ++i) {
                buffer[i] = 0;
                colors_bg[i] = color_bg;
                colors_fg[i] = color_fg;
            }
            ptr_x = ptr_mx;
            ptr_y = ptr_my;
            ptr_rx = ptr_mx;
            ptr_ry = ptr_my;
        }
        else if (c == 22) { // Ctrl+V
            OpenClipboard(window->GetWindowHandle());
            if (IsClipboardFormatAvailable(CF_TEXT))
            {
                HGLOBAL hg = GetClipboardData(CF_TEXT);
                if (hg) {
                    LPCSTR q = (LPCSTR)GlobalLock(hg);
                    if (q != NULL)
                    {
                        CStringA A(q);
                        for (auto i = 0; i < A.GetLength(); i++) {
                            put_char(A[i]);
                        }
                    }
                    GlobalUnlock(hg);
                }
            }
            CloseClipboard();
        }
        else {
            put_char((char)(c & 0xff));
        }
    }

    int cjsgui::reset_cycles() {
        auto c = cycles;
        cycles = 0;
        return c;
    }

    void cjsgui::hit(int n)
    {
        if (vm)
            vm->hit(n);
    }

    bool cjsgui::try_input(int c)
    {
        if (vm) {
            return vm->try_input(c & 0xffff, !(c & 0x20000));
        }
        return false;
    }

    int cjsgui::cursor() const
    {
        if (vm)
            return vm->cursor();
        return 1;
    }

    void cjsgui::output() const
    {
        auto& scr = *screens[screen_id].get();
        auto& rows = scr.rows;
        auto& cols = scr.cols;
        auto& buffer = scr.buffer;

        if (!vm)
            return;
        OpenClipboard(window->GetWindowHandle());
        std::vector<char> outs;
        for (auto i = 0; i < rows; ++i) {
            for (auto j = 0; j < cols; ++j) {
                const auto& b = buffer[i * cols + j];
                if (b == 0) {
                    break;
                }
                if (b > 0) {
                    if (std::isprint(b))
                        outs.push_back(b);
                    else
                        outs.push_back(' ');
                }
                else {
                    outs.push_back(b);
                }
            }
            outs.push_back('\r');
            outs.push_back('\n');
        }
        while (outs.size() > 2) {
            if (outs.back() == '\n' && *(outs.rbegin() + 1) == '\r') {
                outs.pop_back();
                outs.pop_back();
            }
            else {
                break;
            }
        }
        outs.push_back('\0');
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, outs.size());
        if (hg) {
            auto data = (char*)GlobalLock(hg);
            if (data) {
                CopyMemory(data, outs.data(), outs.size());
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hg);
            }
            GlobalUnlock(hg);
        }
        CloseClipboard();
    }

    void cjsgui::exec_cmd(const std::string& s) {
        auto& scr = *screens[screen_ptr].get();
        auto& cols = scr.cols;
        auto& rows = scr.rows;
        auto& buffer = scr.buffer;
        auto& color_bg = scr.color_bg;
        auto& color_fg = scr.color_fg;
        auto& colors_bg = scr.colors_bg;
        auto& colors_fg = scr.colors_fg;
        auto& color_bg_stack = scr.color_bg_stack;
        auto& color_fg_stack = scr.color_fg_stack;
        auto& ptr_x = scr.ptr_x;
        auto& ptr_y = scr.ptr_y;
        auto& ptr_rx = scr.ptr_rx;
        auto& ptr_ry = scr.ptr_ry;
        auto& ptr_mx = scr.ptr_mx;
        auto& ptr_my = scr.ptr_my;

        switch (s[0]) {
        case 'B': { // 设置背景色
            color_bg = (uint32_t)std::stoul(s.substr(1), nullptr, 16);
        }
                  break;
        case 'F': { // 设置前景色
            color_fg = (uint32_t)std::stoul(s.substr(1), nullptr, 16);
        }
                  break;
        case 'S': { // 设置
            static int cfg;
            cfg = (uint32_t)std::stoul(s.substr(1), nullptr, 10);
            switch (cfg) {
            case 0: { // 换行
                if (ptr_x > 0) {
                    ptr_x = 0;
                    if (ptr_y == rows - 1) {
                        new_line();
                    }
                    else {
                        ptr_y++;
                    }
                }
            }
                    break;
            case 1: // 保存背景色
                color_bg_stack.push_back(color_bg);
                break;
            case 2: // 保存前景色
                color_fg_stack.push_back(color_fg);
                break;
            case 3: // 恢复背景色
                color_bg = color_bg_stack.back();
                if (color_bg_stack.size() > 1) color_bg_stack.pop_back();
                break;
            case 4: // 恢复前景色
                color_fg = color_fg_stack.back();
                if (color_fg_stack.size() > 1) color_fg_stack.pop_back();
                break;
            default:
                break;
            }
        }
                  break;
        default:
            break;
        }
    }
}
