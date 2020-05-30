//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <ui\gdi\Gdi.h>
#include "cjs.h"
#include "cjsparser.h"
#include "cjsgen.h"
#include "cjsgui.h"

#define LOG_AST 0
#define LOG_FILE 0
#define LOG_FILENAME "js_output.log"
#define LIBRARY_FILE R"(lib/clib.js)"

#define STAT_DELAY_N 60
#define STAT_MAX_N 10

namespace clib {

    cjs::cjs() {
        rt.set_readonly(false);
        rt.init(this);
        init_lib();
    }

    backtrace_direction cjs::check(js_pda_edge_t edge, js_ast_node *node) {
        return b_next;
    }

    void cjs::error_handler(int, const std::vector<js_pda_trans> &, int &) {
    }

    int cjs::exec(const std::string &filename, const std::string &input) {
        if (input.empty())
            return 0;
        CString stat;
        auto p = std::make_unique<cjsparser>();
        std::string error_string;
        std::string code_name;
        if (!filename.empty() && (filename[0] == '<' || filename[0] == '('))
            code_name = filename;
        else
            code_name = "(" + filename + ":1:1) <entry>";
        auto last = std::chrono::system_clock::now();
        cjs_code_result::ref code;
        try {
            if (p->parse(input, error_string, this) == nullptr) {
                std::stringstream ss;
                ss << "throw new SyntaxError('" << jsv_string::convert(error_string) << "')";
                return exec(code_name, ss.str());
            }
#if LOG_AST
            cjsast::print(p->root(), 0, input, std::cout);
#endif
            auto g = std::make_unique<cjsgen>();
            g->gen_code(p->root(), &input, filename);
            p = nullptr;
#if LOG_FILE
            std::ofstream ofs(LOG_FILENAME);
        if (ofs)
            cjsast::print(p.root(), 0, input, ofs);
#endif
            code = std::move(g->get_code());
            assert(code);
            code->code->debugName = code_name;
            g = nullptr;
        } catch (const clib::cjs_exception &e) {
            stat.Format(L"编译：%S，失败", filename.c_str());
            cjsgui::singleton().add_stat(stat);
            std::stringstream ss;
            ss << "throw new SyntaxError('" << jsv_string::convert(e.message()) << "')";
            return exec(code_name, ss.str());
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last);
        stat.Format(L"编译：%S，耗时：%lldms", filename.c_str(), duration.count());
        cjsgui::singleton().add_stat(stat);
        return rt.eval(std::move(code), filename);
    }

    void cjs::add_stat(const CString& s, bool show)
    {
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
    }

    void cjs::paint_window(const CRect& bounds)
    {
    }

    void cjs::reset_ips()
    {
    }

    void cjs::hit(int n)
    {
    }

    bool cjs::try_input(int c, bool ch)
    {
        return false;
    }

    int cjs::cursor() const
    {
        return 0;
    }

    bool cjs::run(int cycle, int& cycles)
    {
        return rt.run_internal(cycle, cycles) != 11;
    }

    int cjs::get_state() const
    {
        return rt.get_state();
    }

    void cjs::set_state(int n) 
    {
        return rt.set_state(n);
    }

    void cjs::init_lib() {
        char buf[256];
        snprintf(buf, sizeof(buf), "sys.exec_file(\"%s\");\n", LIBRARY_FILE);
        exec("<library>", buf);
    }
}