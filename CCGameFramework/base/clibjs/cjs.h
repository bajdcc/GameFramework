//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJS_H
#define CLIBJS_CJS_H

#include <string>
#include "ui/gdi/Gdi.h"
#include "cjsparser.h"
#include "cjsruntime.h"

namespace clib {

    class cjs : public csemantic {
    public:
        cjs();
        ~cjs() = default;

        cjs(const cjs&) = delete;
        cjs& operator=(const cjs&) = delete;

        backtrace_direction check(js_pda_edge_t, js_ast_node*) override;
        void error_handler(int, const std::vector<js_pda_trans>&, int&) override;

        int exec(const std::string &filename, const std::string& input, bool top = true);

        void add_stat(const CString& s, bool show = true);

        void paint_window(const CRect& bounds);
        void reset_ips();
        void hit(int n);
        bool try_input(int c, bool ch = true);
        int cursor() const;

        bool run(int cycle, int& cycles);

    private:
        void init_lib();

    private:
        cjsruntime rt;
        int stat_n{ 0 };
        std::list<CString> stat_s;
    };
}

#endif //CLIBJS_CJS_H
