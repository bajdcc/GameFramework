//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSPARSER_H
#define CLIBJS_CJSPARSER_H

#include <string>
#include <memory>
#include <unordered_set>
#include "cjslexer.h"
#include "cjsast.h"
#include "cjsunit.h"

namespace clib {

    using namespace types;

    enum backtrace_direction {
        b_success,
        b_next,
        b_error,
        b_removed,
        b_fail,
        b_fallback,
    };

    struct backtrace_t {
        int lexer_index;
        std::vector<int> state_stack;
        std::vector<js_ast_node*> ast_stack;
        std::vector<js_ast_node*> cache;
        int current_state;
        int reduced_rule;
        int trans;
        int parent;
        bool init;
    };

    class csemantic {
    public:
        virtual backtrace_direction check(js_pda_edge_t, js_ast_node *) = 0;
        virtual void error_handler(int, const std::vector<js_pda_trans> &, int &) = 0;
    };

    class cjsparser {
    public:
        cjsparser() = default;
        ~cjsparser() = default;

        cjsparser(const cjsparser &) = delete;
        cjsparser &operator=(const cjsparser &) = delete;

        bool parse(const std::string &str, std::string &, csemantic *s = nullptr);
        js_ast_node *root() const;
        void clear_ast();
        void dump_pda(std::ostream& os);

        using pda_coll_pred_cb = js_pda_coll_pred(*)(const cjslexer *, int idx);
        using terminal_cb = void (*)(const cjslexer *, int idx,
                                     std::vector<std::shared_ptr<backtrace_t>> &bks, std::shared_ptr<backtrace_t> &bk);

    private:

        void next();

        void gen();
        bool program(std::string& error_string, const std::string& str);
        js_ast_node *terminal();

        bool valid_trans(backtrace_t& bk, js_pda_trans &trans, js_unit_token** = nullptr);
        void do_trans(backtrace_t& bk, const js_pda_trans &trans);
        bool LA(backtrace_t& bk, js_unit* u, js_pda_trans& trans, js_unit_token** = nullptr);
    ;
        void expect(bool, const std::string &);
        void match_type(js_lexer_t);

        void error(const std::string &);

        void print_bk(std::vector<std::shared_ptr<backtrace_t>>& bks) const;
        void copy_bk(backtrace_t& dst, backtrace_t& src);
        void del_bk(backtrace_t&);

        static js_pda_coll_pred pred_for(const cjslexer *, int idx);
        static js_pda_coll_pred pred_in(const cjslexer *, int idx);

    private:
        const lexer_unit *current{nullptr};

    private:
        static std::unique_ptr<cjsunit> unit;
        std::unique_ptr<cjslexer> lexer;
        csemantic *semantic{nullptr};
        std::unique_ptr<cjsast> ast;
    };
}

#endif //CLIBJS_CJSPARSER_H
