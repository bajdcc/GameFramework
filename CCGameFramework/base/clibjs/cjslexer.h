//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSLEXER_H
#define CLIBJS_CJSLEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "cjstypes.h"

namespace clib {

    using namespace types;

    struct lexer_unit {
        js_lexer_t t{NONE};
        int idx{0};
        int len{0};
        int line{0};
        int column{0};
        int start{0};
        int end{0};
        int id{0};
    };

    class cjslexer {
    public:
        cjslexer();
        ~cjslexer() = default;

        cjslexer(const cjslexer &) = delete;
        cjslexer &operator=(const cjslexer &) = delete;

        void input(const std::string &text, std::string &);
        void dump() const;

        const lexer_unit &get_unit(int idx) const;
        int get_unit_size() const;
        std::string get_unit_desc(int idx) const;
        const char *get_data(int idx) const;
        bool valid_rule(int idx, js_lexer_t rule) const;

        int get_index() const;
        void inc_index();
        const lexer_unit &get_current_unit() const;

    private:
        static bool allow_expr(const std::vector<lexer_unit> &u);
        int alloc(int size);
        static lexer_unit alloc_unit(int line, int column, int start, int end);

    private:
        int index{0};
        std::string text;
        std::vector<char> data;
        std::vector<lexer_unit> units;
        std::unordered_map<std::string, js_lexer_t> mapKeyword;
        std::vector<bool> no_line;
    };
}

#endif //CLIBJS_CJSLEXER_H
