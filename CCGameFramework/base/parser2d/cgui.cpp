//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cgui.h"
#include "cexception.h"
#include "crev.h"
#include "../../ui/gdi/Gdi.h"
#include "Parser2D.h"
#include <ui\window\Window.h>
#include <zlib\zlib.h>

#ifdef REPORT_ERROR
#undef REPORT_ERROR
#endif
#define REPORT_ERROR 1
#ifdef REPORT_ERROR_FILE
#undef REPORT_ERROR_FILE
#endif
#define REPORT_ERROR_FILE "runtime.log"

#define AST_FILE "ast.log"

#define LOG_AST 0
#define LOG_DEP 0

#define IPS_STAT_TIME 1s

#define ENTRY_FILE "/sys/entry"

#define MAKE_ARGB(a,r,g,b) ((uint32_t)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define MAKE_RGB(r,g,b) MAKE_ARGB(255,r,g,b)
#define GET_R(rgb) (LOBYTE(rgb))
#define GET_G(rgb) (LOBYTE(((WORD)(rgb)) >> 8))
#define GET_B(rgb) (LOBYTE((WORD)((rgb)>>16)))
#define GET_A(rgb) (LOBYTE((rgb)>>24))

extern int g_argc;
extern char** g_argv;

namespace clib {

    cgui::cgui() {
        std::fill(screen_ref.begin(), screen_ref.end(), 0);
        init_screen(0);
        switch_screen(0);
        switch_screen_display(0);
    }

    cgui& cgui::singleton() {
        static clib::cgui gui;
        return gui;
    }

    string_t cgui::load_file(const string_t& name) {
        std::smatch res;
        string_t path;
        if (std::regex_match(name, res, re_path)) {
            path = FILE_ROOT + res[0].str() + ".cpp";
        }
        if (path.empty())
            error("file not exists: " + name);
        std::ifstream t(path);
        if (t) {
            std::stringstream buffer;
            buffer << t.rdbuf();
            auto str = buffer.str();
            cvfs::convert_utf8_to_gbk(str);
            std::vector<byte> data(str.begin(), str.end());
            vm->as_root(true);
            vm->write_vfs(name, data);
            vm->as_root(false);
            return str;
        }
        std::vector<byte> data;
        if (vm->read_vfs(name, data)) {
            return string_t(data.begin(), data.end());
        }
        error("file not exists: " + name);
        return "";
    }

    bool cgui::exist_file(const string_t& name) {
        std::smatch res;
        string_t path;
        if (std::regex_match(name, res, re_path)) {
            path = FILE_ROOT + res[0].str() + ".cpp";
        }
        if (path.empty())
            return false;
        std::ifstream t(path);
        if (t) {
            return true;
        }
        if (vm->exist_vfs(name)) {
            return true;
        }
        return false;
    }

    bool cgui::exist_bin(const string_t& name)
    {
        std::smatch res;
        string_t path;
        if (std::regex_match(name, res, re_path)) {
            path = BIN_ROOT + res[0].str() + ".bin";
        }
        if (path.empty())
            return false;
        std::ifstream t(path);
        if (!t) {
            return false;
        }
        if (cache.find(name) != cache.end())
            return true;
        std::ifstream ifs(path, std::ios::binary);
        if (ifs) {
            auto start = std::chrono::system_clock::now();
            auto p = ifs.rdbuf();
            auto size = p->pubseekoff(0, std::ios::end, std::ios::in);
            p->pubseekpos(0, std::ios::in);
            std::vector<byte> data;
            data.resize((size_t)size);
            p->sgetn((char*)data.data(), size);
            if (data.size() < 12) {
                return false;
            }
            if (strncmp((const char*)data.data(), "CCOS", 4) == 0) {
                CString stat;
                if (strncmp((const char*)data.data() + 4, "TEXT", 4) == 0) {
                    auto size2 = *((uLongf*)(data.data() + 8));
                    if (size2 != data.size() - 12)
                        return false;
                    data.erase(data.begin(), data.begin() + 12);
                    cache.insert(std::make_pair(name, data));
                    vm->as_root(true);
                    vm->write_vfs(name + ".bin", crev::conv(data));
                    vm->as_root(false);
                    auto end = std::chrono::system_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    stat.Format(L"加载缓存：%S，耗时：%lldms", name.c_str(), duration.count());
                    vm->add_stat(stat);
                    return true;
                }
                else if (strncmp((const char*)data.data() + 4, "ZLIB", 4) == 0) {
                    auto size2 = *((uLongf*)(data.data() + 8));
                    uLongf newsize = size2;
                    std::vector<byte> newdata(size2);
                    auto r = uncompress(newdata.data(), &newsize, data.data() + 12, data.size() - 12);
                    if (r == Z_OK && newsize == size2) {
                        cache.insert(std::make_pair(name, newdata));
                        vm->as_root(true);
                        vm->write_vfs(name + ".bin", crev::conv(newdata));
                        vm->as_root(false);
                        auto end = std::chrono::system_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                        stat.Format(L"加载缓存：%S，耗时：%lldms", name.c_str(), duration.count());
                        vm->add_stat(stat);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool cgui::save_bin(const string_t& name)
    {
        std::smatch res;
        string_t path;
        if (std::regex_match(name, res, re_path)) {
            path = BIN_ROOT + res[0].str() + ".bin";
        }
        if (path.empty())
            return false;
        std::ofstream ofs(path, std::ios::binary);
        if (ofs) {
            const auto& data = cache.at(name);
            vm->as_root(true);
            vm->write_vfs(name + ".bin", crev::conv(data));
            vm->as_root(false);
            std::vector<byte> output(compressBound(data.size()));
            uLongf size;
            auto r = compress(output.data(), &size, data.data(), data.size());
            if (r == Z_OK) {
                ofs.write("CCOSZLIB", 8);
                auto size2 = data.size();
                ofs.write((const char*)& size2, 4);
                ofs.write((const char*)output.data(), size);
            }
            else {
                ofs.write("CCOSTEXT", 8);
                size = data.size();
                ofs.write((const char*)& size, 4);
                ofs.write((const char*)data.data(), data.size());
            }
            return true;
        }
        return false;
    }

    bool ConvertFileTimeToLocalTime(const FILETIME* lpFileTime, SYSTEMTIME* lpSystemTime)
    {
        if (!lpFileTime || !lpSystemTime) {
            return false;
        }
        FILETIME ftLocal;
        FileTimeToLocalFileTime(lpFileTime, &ftLocal);
        FileTimeToSystemTime(&ftLocal, lpSystemTime);
        return true;
    }

    bool ConvertLocalTimeToFileTime(const SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime)
    {
        if (!lpSystemTime || !lpFileTime) {
            return false;
        }

        FILETIME ftLocal;
        SystemTimeToFileTime(lpSystemTime, &ftLocal);
        LocalFileTimeToFileTime(&ftLocal, lpFileTime);
        return true;
    }

    bool cgui::get_fs_time(const string_t& name, const string_t& ext, std::vector<string_t>& time)
    {
        std::smatch res;
        string_t path;
        if (std::regex_match(name, res, re_path)) {
            path = BIN_ROOT + res[0].str() + ext;
        }
        if (path.empty())
            return false;

        HANDLE hFile;
        // Create, Access, Write
        FILETIME ft[3];
        SYSTEMTIME st[3];

        auto pp = CString(CStringA(path.c_str()));

        hFile = CreateFile(pp, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (INVALID_HANDLE_VALUE == hFile) {
            return false;
        }
        GetFileTime(hFile, &ft[0], &ft[1], &ft[2]);
        for (auto i = 0; i < 3; i++) {
            ConvertFileTimeToLocalTime(&ft[i], &st[i]);
            CStringA buf;
            buf.Format("%04d-%02d-%02d %02d:%02d:%02d.%03d",
                st[i].wYear,
                st[i].wMonth,
                st[i].wDay,
                st[i].wHour,
                st[i].wMinute,
                st[i].wSecond,
                st[i].wMilliseconds);
            time.push_back(buf.GetBuffer(0));
        }

        CloseHandle(hFile);
        return true;
    }

    void cgui::reset() {
        if (vm) {
            vm.reset();
            gen.reset();
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

    void cgui::clear_cache()
    {
        cache.clear();
        cache_code.clear();
        cache_dep.clear();
    }

    void cgui::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const Parser2DEngine::BrushBag& brushes, bool paused, decimal fps) {
        if (!paused && !entered) {
            if (cvm::global_state.input->interrupt) {
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

    void cgui::draw_text(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const Parser2DEngine::BrushBag& brushes) {
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

    void cgui::draw_window(const CRect& bounds)
    {
        if (vm)
            vm->paint_window(bounds);
    }

    void cgui::reset_ips()
    {
        if (vm)
            vm->reset_ips();
    }

    void cgui::tick() {
        if (exited)
            return;
        if (running) {
            try {
                if (!vm->run(cycle, cycles)) {
                    running = false;
                    exited = true;
                    put_string("\n[!] clibos exited.");
                    vm.reset();
                    gen.reset();
                }
            }
            catch (const cexception& e) {
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
                vm = std::make_unique<cvm>();
                std::vector<string_t> args;
                if (g_argc > 0) {
                    args.emplace_back(ENTRY_FILE);
                    for (int i = 1; i < g_argc; ++i) {
                        args.emplace_back(g_argv[i]);
                    }
                }
                if (compile(ENTRY_FILE, args, decltype(args)()) != -1) {
                    running = true;
                }
            }
        }
    }

    void cgui::put_string(const string_t& str) {
        for (auto& s : str) {
            put_char(s);
        }
    }

    void cgui::put_char(int c) {
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
                static string_t pat{ R"([A-Za-z][0-9a-f]{1,8})" };
                static std::regex re(pat);
                std::smatch res;
                string_t s(cmd_string.begin(), cmd_string.end());
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
            memset(buffer, 0, (uint)size);
            std::fill(colors_bg, colors_bg + size, color_bg);
            std::fill(colors_fg, colors_fg + size, color_fg);
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

    void cgui::input_call(int c) {
        auto old = screen_ptr;
        screen_ptr = screen_id;
        input(c);
        screen_ptr = old;
    }

    void cgui::new_line() {
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
        memcpy(buffer, buffer + cols, (uint)cols * (rows - 1));
        memset(&buffer[cols * (rows - 1)], 0, (uint)cols);
        memcpy(colors_bg, colors_bg + cols, (uint)cols * (rows - 1) * sizeof(uint32_t));
        std::fill(&colors_bg[cols * (rows - 1)], &colors_bg[cols * (rows)], color_bg);
        memcpy(colors_fg, colors_fg + cols, (uint)cols * (rows - 1) * sizeof(uint32_t));
        std::fill(&colors_fg[cols * (rows - 1)], &colors_fg[cols * (rows)], color_fg);
    }

    void cgui::draw_char(const char& c) {
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

    void cgui::error(const string_t& str) {
        throw cexception(ex_gui, str);
    }

    void cgui::set_cycle(int cycle) {
        if (cycle == 0) {
            cycle_set = false;
            this->cycle = GUI_CYCLES;
        }
        else {
            cycle_set = true;
            this->cycle = cycle;
        }
    }

    void cgui::set_ticks(int ticks) {
        this->ticks = ticks;
    }

    void cgui::move(bool left) {
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

    void cgui::forward(int& x, int& y, bool forward) {
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

    string_t cgui::input_buffer() const {
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

    bool cgui::init_screen(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (screens[n])
            return false;
        screens[n].reset(new screen_t());
        auto& scr = *screens[n].get();
        auto& input = scr.input;
        auto& memory = scr.memory;
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
        buffer = memory.alloc_array<char>((uint)size);
        assert(buffer);
        memset(buffer, 0, (uint)size);
        colors_bg = memory.alloc_array<uint32_t>((uint)size);
        assert(colors_bg);
        color_bg = 0;
        std::fill(colors_bg, colors_bg + size, color_bg);
        colors_fg = memory.alloc_array<uint32_t>((uint)size);
        assert(colors_fg);
        color_fg = MAKE_RGB(255, 255, 255);
        std::fill(colors_fg, colors_fg + size, color_fg);
        color_bg_stack.push_back(color_bg);
        color_fg_stack.push_back(color_fg);
        if (vm) {
            CString s;
            s.Format(L"初始化屏幕（%d）", n);
            vm->add_stat(s);
        }
        return true;
    }

    bool cgui::switch_screen_display(int n)
    {
        if (screen_id == n)
            return true;
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (!screens[n])
            return false;
        screen_id = n;
        cvm::global_state.input_s = &screens[n]->input;
        return true;
    }

    int cgui::current_screen() const
    {
        return screen_id;
    }

    void cgui::screen_ref_add(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return;
        if (!screens[n])
            return;
        screen_ref[n]++;
    }

    void cgui::screen_ref_dec(int n)
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

    cvm::global_input_t* cgui::get_screen_interrupt()
    {
        if (screen_interrupt.empty())
        return nullptr;
        auto n = screen_interrupt.back();
        screen_interrupt.pop_back();
        return &screens[n]->input;
    }

    bool cgui::switch_screen(int n)
    {
        if (screen_ptr == n)
            return true;
        if (n < 0 || n >= (int)screens.size())
            return false;
        if (!screens[n])
            return false;
        screen_ptr = n;
        cvm::global_state.input = &screens[n]->input;
        return true;
    }

    void cgui::resize(int r, int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& memory = scr.memory;
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
        buffer = memory.alloc_array<char>((uint)size);
        assert(buffer);
        if (!buffer)
            error("gui memory overflow");
        memset(buffer, 0, (uint)size);
        auto old_bg = colors_bg;
        colors_bg = memory.alloc_array<uint32_t>((uint)size);
        assert(colors_bg);
        if (!colors_bg)
            error("gui memory overflow");
        std::fill(colors_bg, colors_bg + size, 0);
        auto old_fg = colors_fg;
        colors_fg = memory.alloc_array<uint32_t>((uint)size);
        assert(colors_fg);
        if (!colors_fg)
            error("gui memory overflow");
        std::fill(colors_fg, colors_fg + size, MAKE_RGB(255, 255, 255));
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
        memory.free(old_buffer);
        memory.free(old_fg);
        memory.free(old_bg);
    }

    CSize cgui::get_size() const {
        auto& scr = *screens[screen_ptr].get();
        return { scr.cols * GUI_FONT_W, scr.rows * GUI_FONT_H };
    }

    std::unordered_set<string_t> cgui::get_dep(string_t& path) const
    {
        auto f = cache_code.find(path);
        if (f != cache_code.end()) {
            return cache_dep.at(path);
        }
        return std::unordered_set<string_t>();
    }

    CString cgui::get_disp(cvm::disp_t t) const
    {
        if (vm) {
            return vm->get_disp(t);
        }
        return CString();
    }

    int cgui::new_screen(int n)
    {
        if (n < 0 || n >= (int)screens.size())
            return 1;
        if (screens[n])
            return 2;
        init_screen(n);
        return 0;
    }

    void cgui::load_dep(string_t& path, std::unordered_set<string_t>& deps) {
        auto f = cache_code.find(path);
        if (f != cache_code.end()) {
            deps.insert(cache_dep[path].begin(), cache_dep[path].end());
            return;
        }
        auto code = load_file(path);
        static string_t pat_inc{ "#include[ ]+\"([/A-Za-z0-9_-]+?)\"" };
        static std::regex re_inc(pat_inc);
        std::smatch res;
        auto begin = code.cbegin();
        auto end = code.cend();
        std::vector<std::tuple<int, int, string_t>> records;
        {
            auto offset = 0;
            while (std::regex_search(begin, end, res, re_inc)) {
                if (res[1].str() == path) {
                    error("cannot include self: " + path);
                }
                if (offset + res.position() > 0) {
                    if (code[offset + res.position() - 1] != '\n') {
                        error("invalid include: " + res[1].str());
                    }
                }
                records.emplace_back(offset + res.position(),
                    offset + res.position() + res.length(),
                    res[1].str());
                offset += std::distance(begin, res[0].second);
                begin = res[0].second;
            }
        }
        if (!records.empty()) {
            // has #include directive
            std::unordered_set<string_t> _deps;
            for (auto& r : records) {
                auto& include_path = std::get<2>(r);
                load_dep(include_path, _deps);
                _deps.insert(include_path);
            }
            std::stringstream sc;
            int prev = 0;
            for (auto& r : records) {
                auto& start = std::get<0>(r);
                auto& length = std::get<1>(r);
                if (prev < start) {
                    auto frag = code.substr((uint)prev, (uint)(start - prev));
                    sc << frag;
                }
                auto incs = code.substr((uint)start, (uint)(length - start));
                sc << "// => " << incs;
                prev = length;
            }
            if (prev < (int)code.length()) {
                auto frag = code.substr((uint)prev, code.length() - (uint)prev);
                sc << frag;
            }
            cache_code.insert(std::make_pair(path, sc.str()));
            cache_dep.insert(std::make_pair(path, _deps));
        }
        else {
            // no #include directive
            cache_code.insert(std::make_pair(path, code));
            cache_dep.insert(std::make_pair(path, std::unordered_set<string_t>()));
        }
        load_dep(path, deps);
    }

    string_t cgui::do_include(string_t& path) { // DAG solution for include
        std::vector<string_t> v; // VERTEX(Map id to name)
        std::unordered_map<string_t, int> deps; // VERTEX(Map name to id)
        {
            std::unordered_set<string_t> _deps;
            load_dep(path, _deps);
            if (_deps.empty())
                return cache_code[path]; // no include
            _deps.insert(path);
            v.resize(_deps.size());
            std::copy(_deps.begin(), _deps.end(), v.begin());
            int i = 0;
            for (auto& d : v) {
                deps.insert(std::make_pair(d, i++));
            }
        }
        auto n = v.size();
        std::vector<std::vector<bool>> DAG(n); // DAG(Map id to id)
        std::unordered_set<size_t> deleted;
        std::vector<size_t> topo; // 拓扑排序
        for (size_t i = 0; i < n; ++i) {
            DAG[i].resize(n);
            for (size_t j = 0; j < n; ++j) {
                auto& _d = cache_dep[v[i]];
                if (_d.find(v[j]) != _d.end())
                    DAG[i][j] = true;
            }
        }
        // DAG[i][j] == true  =>  i 包含 j
        for (size_t i = 0; i < n; ++i) { // 每次找出零入度点并删除
            size_t right = n;
            for (size_t j = 0; j < n; ++j) { // 找出零入度点
                if (deleted.find(j) == deleted.end()) {
                    bool success = true;
                    for (size_t k = 0; k < n; ++k) {
                        if (DAG[j][k]) {
                            success = false;
                            break;
                        }
                    }
                    if (success) { // 找到
                        right = j;
                        break;
                    }
                }
            }
            if (right != n) {
                for (size_t k = 0; k < n; ++k) { // 删除点
                    DAG[k][right] = false;
                }
                topo.push_back(right);
                deleted.insert(right);
            }
        }
        if (topo.size() != n) {
            error("topo failed: " + path);
        }
#if LOG_DEP
        ATLTRACE("[SYSTEM] DEP  | ---------------\n");
        ATLTRACE("[SYSTEM] DEP  | PATH: %s\n", path.c_str());
        for (size_t i = 0; i < n; ++i) {
            ATLTRACE("[SYSTEM] DEP  | [%d] ==> %s\n", i, v[topo[i]].c_str());
        }
        ATLTRACE("[SYSTEM] DEP  | ---------------\n");
#endif
        std::stringstream ss;
        for (auto& tp : topo) {
            ss << "pragma \"note:" << v[tp] << "\";" << std::endl;
            ss << cache_code[v[tp]] << std::endl;
        }
        return ss.str();
    }

    int cgui::compile(const string_t& path, const std::vector<string_t>& args, const std::vector<string_t>& paths) {
        if (path.empty())
            return -1;
        auto fail_errno = -1;
        auto new_path = path;
        auto bin_exist = false;
        if (path[0] != '/') {
            for (auto& p : paths) {
                auto pp = p == "/" ? ('/' + path) : (p + '/' + path);
                if (exist_bin(pp)) {
                    new_path = pp;
                    bin_exist = true;
                    break;
                }
            }
        }
        else if (exist_bin(new_path)) {
            bin_exist = true;
        }
        if (bin_exist) {
            // 判断生成的二进制文件是否最新
            // 即：生成时间大于代码修改时间
            // 失败的话，就删除cache缓存
            std::vector<string_t> t1, t2;
            if (get_fs_time(new_path, ".cpp", t1) && get_fs_time(new_path, ".bin", t2) && t1[2] > t2[2]) {
                // FAILED
                cache.erase(new_path);
            }
        }
        else if (path[0] != '/') {
            for (auto& p : paths) {
                auto pp = p + '/' + path;
                if (exist_file(pp)) {
                    new_path = pp;
                    break;
                }
            }
        }
        CString stat;
        try {
            auto c = cache.find(new_path);
            if (c != cache.end()) {
                return vm->load(new_path, c->second, args);
            }
            auto code = do_include(new_path);
            fail_errno = -2;
            gen.reset();
            auto start = std::chrono::system_clock::now();
            auto root = p.parse(code, &gen);
#if LOG_AST
            {
                std::ofstream log(AST_FILE, std::ios::app | std::ios::out);
                log << std::endl << std::endl;
                cast::print(root, 0, log);
                log << std::endl << std::endl;
            }
#endif
            gen.gen(new_path, root);
            auto file = gen.file();
            auto end = std::chrono::system_clock::now();
            p.clear_ast();
            cache.insert(std::make_pair(new_path, file));
            save_bin(new_path);
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            stat.Format(L"编译：%S，耗时：%lldms", new_path.c_str(), duration.count());
            vm->add_stat(stat);
            return vm->load(new_path, file, args);
        }
        catch (const cexception& e) {
            gen.reset();
#if LOG_VM
            {
                CStringA s; s.Format("[SYSTEM] ERR  | PATH: %s, %s\n", new_path.c_str(), e.message().c_str());
                cvm::global_state.log_err.push_back(s.GetBuffer(0));
                cvm::logging(CString(s));
            }
#endif
            ATLTRACE("[SYSTEM] ERR  | PATH: %s, %s\n", new_path.c_str(), e.message().c_str());
#if REPORT_ERROR
            {
                std::ofstream log(REPORT_ERROR_FILE, std::ios::app | std::ios::out);
                log << "[SYSTEM] ERR  | PATH: " << new_path << ", " << e.message() << std::endl;
            }
#endif
            return fail_errno;
        }
    }

    void cgui::input_set(bool valid) {
        auto& scr = *screens[screen_ptr].get();
        auto& memory = scr.memory;
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

    const wchar_t* mapVirtKey[] = {
        L"VK_RESERVED",
        L"VK_LBUTTON",
        L"VK_RBUTTON",
        L"VK_CANCEL",
        L"VK_MBUTTON",
        L"VK_XBUTTON1",
        L"VK_XBUTTON2",
        L"VK_RESERVED",
        L"VK_BACK",
        L"VK_TAB",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_CLEAR",
        L"VK_RETURN",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_SHIFT",
        L"VK_CONTROL",
        L"VK_MENU",
        L"VK_PAUSE",
        L"VK_CAPITAL",
        L"VK_HANGUL",
        L"VK_RESERVED",
        L"VK_JUNJA",
        L"VK_FINAL",
        L"VK_KANJI",
        L"VK_RESERVED",
        L"VK_ESCAPE",
        L"VK_CONVERT",
        L"VK_NONCONVERT",
        L"VK_ACCEPT",
        L"VK_MODECHANGE",
        L"VK_SPACE",
        L"VK_PRIOR",
        L"VK_NEXT",
        L"VK_END",
        L"VK_HOME",
        L"VK_LEFT",
        L"VK_UP",
        L"VK_RIGHT",
        L"VK_DOWN",
        L"VK_SELECT",
        L"VK_PRINT",
        L"VK_EXECUTE",
        L"VK_SNAPSHOT",
        L"VK_INSERT",
        L"VK_DELETE",
        L"VK_HELP",
        L"VK_0",
        L"VK_1",
        L"VK_2",
        L"VK_3",
        L"VK_4",
        L"VK_5",
        L"VK_6",
        L"VK_7",
        L"VK_8",
        L"VK_9",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_A",
        L"VK_B",
        L"VK_C",
        L"VK_D",
        L"VK_E",
        L"VK_F",
        L"VK_G",
        L"VK_H",
        L"VK_I",
        L"VK_J",
        L"VK_K",
        L"VK_L",
        L"VK_M",
        L"VK_N",
        L"VK_O",
        L"VK_P",
        L"VK_Q",
        L"VK_R",
        L"VK_S",
        L"VK_T",
        L"VK_U",
        L"VK_V",
        L"VK_W",
        L"VK_X",
        L"VK_Y",
        L"VK_Z",
        L"VK_LWIN",
        L"VK_RWIN",
        L"VK_APPS",
        L"VK_RESERVED",
        L"VK_SLEEP",
        L"VK_NUMPAD0",
        L"VK_NUMPAD1",
        L"VK_NUMPAD2",
        L"VK_NUMPAD3",
        L"VK_NUMPAD4",
        L"VK_NUMPAD5",
        L"VK_NUMPAD6",
        L"VK_NUMPAD7",
        L"VK_NUMPAD8",
        L"VK_NUMPAD9",
        L"VK_MULTIPLY",
        L"VK_ADD",
        L"VK_SEPARATOR",
        L"VK_SUBTRACT",
        L"VK_DECIMAL",
        L"VK_DIVIDE",
        L"VK_F1",
        L"VK_F2",
        L"VK_F3",
        L"VK_F4",
        L"VK_F5",
        L"VK_F6",
        L"VK_F7",
        L"VK_F8",
        L"VK_F9",
        L"VK_F10",
        L"VK_F11",
        L"VK_F12",
        L"VK_F13",
        L"VK_F14",
        L"VK_F15",
        L"VK_F16",
        L"VK_F17",
        L"VK_F18",
        L"VK_F19",
        L"VK_F20",
        L"VK_F21",
        L"VK_F22",
        L"VK_F23",
        L"VK_F24",
        L"VK_NAVIGATION_VIEW",
        L"VK_NAVIGATION_MENU",
        L"VK_NAVIGATION_UP",
        L"VK_NAVIGATION_DOWN",
        L"VK_NAVIGATION_LEFT",
        L"VK_NAVIGATION_RIGHT",
        L"VK_NAVIGATION_ACCEPT",
        L"VK_NAVIGATION_CANCEL",
        L"VK_NUMLOCK",
        L"VK_SCROLL",
        L"VK_OEM_FJ_JISHO",
        L"VK_OEM_FJ_MASSHOU",
        L"VK_OEM_FJ_TOUROKU",
        L"VK_OEM_FJ_LOYA",
        L"VK_OEM_FJ_ROYA",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_LSHIFT",
        L"VK_RSHIFT",
        L"VK_LCONTROL",
        L"VK_RCONTROL",
        L"VK_LMENU",
        L"VK_RMENU",
        L"VK_BROWSER_BACK",
        L"VK_BROWSER_FORWARD",
        L"VK_BROWSER_REFRESH",
        L"VK_BROWSER_STOP",
        L"VK_BROWSER_SEARCH",
        L"VK_BROWSER_FAVORITES",
        L"VK_BROWSER_HOME",
        L"VK_VOLUME_MUTE",
        L"VK_VOLUME_DOWN",
        L"VK_VOLUME_UP",
        L"VK_MEDIA_NEXT_TRACK",
        L"VK_MEDIA_PREV_TRACK",
        L"VK_MEDIA_STOP",
        L"VK_MEDIA_PLAY_PAUSE",
        L"VK_LAUNCH_MAIL",
        L"VK_LAUNCH_MEDIA_SELECT",
        L"VK_LAUNCH_APP1",
        L"VK_LAUNCH_APP2",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_OEM_1 // ';:' for US",
        L"VK_OEM_PLUS // '+' any country",
        L"VK_OEM_COMMA // ',' any country",
        L"VK_OEM_MINUS // '-' any country",
        L"VK_OEM_PERIOD // '.' any country",
        L"VK_OEM_2 // '/?' for US",
        L"VK_OEM_3 // '`~' for US",
        L"VK_RESERVED",
        L"VK_RESERVED",
        L"VK_GAMEPAD_A",
        L"VK_GAMEPAD_B",
        L"VK_GAMEPAD_X",
        L"VK_GAMEPAD_Y",
        L"VK_GAMEPAD_RIGHT_SHOULDER",
        L"VK_GAMEPAD_LEFT_SHOULDER",
        L"VK_GAMEPAD_LEFT_TRIGGER",
        L"VK_GAMEPAD_RIGHT_TRIGGER",
        L"VK_GAMEPAD_DPAD_UP",
        L"VK_GAMEPAD_DPAD_DOWN",
        L"VK_GAMEPAD_DPAD_LEFT",
        L"VK_GAMEPAD_DPAD_RIGHT",
        L"VK_GAMEPAD_MENU",
        L"VK_GAMEPAD_VIEW",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_UP",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_DOWN",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT",
        L"VK_GAMEPAD_LEFT_THUMBSTICK_LEFT",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_UP",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT",
        L"VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT",
        L"VK_OEM_4",
        L"VK_OEM_5",
        L"VK_OEM_6",
        L"VK_OEM_7",
        L"VK_OEM_8",
        L"VK_RESERVED",
        L"VK_OEM_AX",
        L"VK_OEM_102",
        L"VK_ICO_HELP",
        L"VK_ICO_00",
        L"VK_PROCESSKEY",
        L"VK_ICO_CLEAR",
        L"VK_PACKET",
        L"VK_RESERVED",
        L"VK_OEM_RESET",
        L"VK_OEM_JUMP",
        L"VK_OEM_PA1",
        L"VK_OEM_PA2",
        L"VK_OEM_PA3",
        L"VK_OEM_WSCTRL",
        L"VK_OEM_CUSEL",
        L"VK_OEM_ATTN",
        L"VK_OEM_FINISH",
        L"VK_OEM_COPY",
        L"VK_OEM_AUTO",
        L"VK_OEM_ENLW",
        L"VK_OEM_BACKTAB",
        L"VK_ATTN",
        L"VK_CRSEL",
        L"VK_EXSEL",
        L"VK_EREOF",
        L"VK_PLAY",
        L"VK_ZOOM",
        L"VK_NONAME",
        L"VK_PA1",
        L"VK_OEM_CLEAR",
        L"VK_RESERVED",
    };

    void cgui::input(int c) {
        auto& scr = *screens[screen_ptr].get();
        auto& memory = scr.memory;
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
            cvm::global_state.input_s->interrupt = true;
            cmd_state = false;
            if (input_state) {
                ptr_x = ptr_rx;
                ptr_y = ptr_ry;
                put_char('\n');
                cvm::global_state.input_s->input_content.clear();
                cvm::global_state.input_s->input_read_ptr = 0;
                cvm::global_state.input_s->input_success = true;
                cvm::global_state.input_s->input_code = 0;
                input_state = false;
                cvm::global_state.input_s->input_single = false;
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
        if (cvm::global_state.input_s->input_single) {
            if (c > 0 && c < 256 && (std::isprint(c) || c == '\r')) {
                if (c == '\r')
                    c = '\n';
                put_char(c);
                ptr_x = ptr_rx;
                ptr_y = ptr_ry;
                string_t s;
                s += c;
                cvm::global_state.input_s->input_content = s;
                cvm::global_state.input_s->input_read_ptr = 0;
                cvm::global_state.input_s->input_success = true;
                cvm::global_state.input_s->input_code = 0;
                cvm::global_state.input_s->input_single = false;
                input_state = false;
            }
            else {
#if LOG_VM
                {
                    CStringA s; s.Format("[SYSTEM] GUI  | Input invalid single key: %d\n", c);
                    cvm::global_state.log_err.push_back(s.GetBuffer(0));
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
            cvm::global_state.log_info.push_back(s.GetBuffer(0));
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
            cvm::global_state.input_s->input_content = input_buffer();
            cvm::global_state.input_s->input_read_ptr = 0;
            cvm::global_state.input_s->input_success = true;
            cvm::global_state.input_s->input_code = 0;
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
                cvm::global_state.log_err.push_back(s.GetBuffer(0));
                cvm::logging(CString(s));
            }
#endif
            ATLTRACE("[SYSTEM] GUI  | Input invalid special key: %d\n", c & 0xff);
            return;
            }
            cvm::global_state.input_s->input_content = input_buffer();
            cvm::global_state.input_s->input_read_ptr = 0;
            cvm::global_state.input_s->input_success = true;
            cvm::global_state.input_s->input_code = C;
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

    int cgui::reset_cycles() {
        auto c = cycles;
        cycles = 0;
        return c;
    }

    void cgui::hit(int n)
    {
        if (vm)
            vm->hit(n);
    }

    bool cgui::try_input(int c)
    {
        if (vm) {
            return vm->try_input(c & 0xffff, !(c & 0x20000));
        }
        return false;
    }

    int cgui::cursor() const
    {
        if (vm)
            return vm->cursor();
        return 1;
    }

    void cgui::output() const
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

    void cgui::exec_cmd(const string_t& s) {
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
