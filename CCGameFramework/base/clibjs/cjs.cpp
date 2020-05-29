//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include "cjs.h"
#include "cjsparser.h"
#include "cjsgen.h"

#define LOG_AST 0
#define LOG_FILE 0
#define LOG_FILENAME "output.txt"
#define LIBRARY_FILE ROOT_DIR R"(lib/clib.js)"

namespace clib {

    cjs::cjs() {
        rt.set_readonly(false);
        rt.init(this);
        init_lib();
        rt.set_readonly(true);
    }

    backtrace_direction cjs::check(js_pda_edge_t edge, js_ast_node *node) {
        return b_next;
    }

    void cjs::error_handler(int, const std::vector<js_pda_trans> &, int &) {
    }

    int cjs::exec(const std::string &filename, const std::string &input, bool top) {
        if (input.empty())
            return 0;
        auto p = std::make_unique<cjsparser>();
        std::string error_string;
        std::string code_name;
        if (!filename.empty() && (filename[0] == '<' || filename[0] == '('))
            code_name = filename;
        else
            code_name = "(" + filename + ":1:1) <entry>";
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
            std::stringstream ss;
            ss << "throw new SyntaxError('" << jsv_string::convert(e.message()) << "')";
            return exec(code_name, ss.str());
        }
        return rt.eval(std::move(code), filename, top);
    }

    void cjs::init_lib() {
        char buf[256];
        snprintf(buf, sizeof(buf), "sys.exec_file(\"%s\");\n", LIBRARY_FILE);
        exec("<library>", buf);
    }
}