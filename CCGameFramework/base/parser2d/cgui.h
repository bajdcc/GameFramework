//
// Project: cliblisp
// Created by bajdcc
//

#ifndef CLIBLISP_CGUI_H
#define CLIBLISP_CGUI_H

#include <d2d1.h>
#include <array>
#include <deque>
#include "types.h"
#include "cparser.h"
#include "cgen.h"
#include "parser2d.h"
#include "cui.h"

#define GUI_FONT GLUT_BITMAP_9_BY_15
#define GUI_FONT_W 9
#define GUI_FONT_W_C1 1
#define GUI_FONT_W_C2 (GUI_FONT_W * 2 + 1)
#define GUI_FONT_H 17
#define GUI_FONT_H_1 0
#define GUI_FONT_H_2 17
#define GUI_FONT_H_C1 -2
#define GUI_FONT_H_C2 15
#define GUI_ROWS 30
#define GUI_COLS 84
#define GUI_SIZE (GUI_ROWS * GUI_COLS)
#define GUI_CYCLES 1000
#define GUI_MAX_SPEED 4
#define GUI_MAX_CYCLE 100000000
#define GUI_MIN_CYCLE 10
#define GUI_MIN_FPS_RATE 0.5
#define GUI_MAX_FPS_RATE 0.8
#define GUI_CYCLE_STABLE 100
#define GUI_TICKS 1
#define GUI_INPUT_CARET 15
#define GUI_MEMORY (256 * 1024)
#define GUI_SPECIAL_MASK 0x2000

namespace clib {

    class cgui {
    public:
        cgui();
        ~cgui() = default;

        cgui(const cgui&) = delete;
        cgui& operator=(const cgui&) = delete;

        void draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const Parser2DEngine::BrushBag& brushes, bool paused, decimal fps);
        int compile(const string_t& path, const std::vector<string_t>& args);

        void put_string(const string_t& str);
        void put_char(char c);
        void input_char(char c);

        void set_cycle(int cycle);
        void set_ticks(int ticks);
        void resize(int rows, int cols);
        CSize get_size() const;

        void input_set(bool valid);
        void input(int c);
        void reset_cmd();
        int reset_cycles();

    private:
        void tick();
        void draw_text(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, const Parser2DEngine::BrushBag& brushes);

        void new_line();
        inline void draw_char(const char& c);

        void load_dep(string_t& path, std::unordered_set<string_t>& deps);
        string_t do_include(string_t& path);

        void exec_cmd(const string_t& s);

        static void error(const string_t&);
        void move(bool left);
        void forward(int& x, int& y, bool forward);
        string_t input_buffer() const;

    public:
        static cgui& singleton();

        string_t load_file(string_t& name);
        void reset();

    private:
        cgen gen;
        cparser p;
        std::unique_ptr<cvm> vm;
        memory_pool<GUI_MEMORY> memory;
        char* buffer{ nullptr };
        uint32_t* colors_bg{ nullptr };
        uint32_t* colors_fg{ nullptr };
        std::unordered_map<string_t, std::vector<byte>> cache;
        std::unordered_map<string_t, string_t> cache_code;
        std::unordered_map<string_t, std::unordered_set<string_t>> cache_dep;
        std::vector<uint32_t> color_bg_stack;
        std::vector<uint32_t> color_fg_stack;
        bool running{ false };
        bool exited{ false };
        int cycle{ GUI_CYCLES };
        int ticks{ GUI_TICKS };
        int ptr_x{ 0 };
        int ptr_y{ 0 };
        int ptr_mx{ 0 };
        int ptr_my{ 0 };
        int ptr_rx{ 0 };
        int ptr_ry{ 0 };
        int rows{ GUI_ROWS };
        int cols{ GUI_COLS };
        int size{ GUI_SIZE };
        bool input_state{ false };
        int input_ticks{ 0 };
        bool input_caret{ false };
        bool cmd_state{ false };
        std::vector<char> cmd_string;
        uint32_t color_bg;
        uint32_t color_fg;
        int cycles{ 0 };
        int cycle_speed{ 0 };
        int cycle_stable{ 0 };
        bool cycle_set{ false };
    };
}

#endif //CLIBLISP_CGUI_H
