//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJS_H
#define CLIBJS_CJS_H

#include <string>
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

    private:
        void init_lib();

    private:
        cjsruntime rt;
    };
}

#endif //CLIBJS_CJS_H
