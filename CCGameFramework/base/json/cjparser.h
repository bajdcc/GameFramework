//
// Project: clibjson
// Author: bajdcc
//
#ifndef CLIBJSON_PARSER_H
#define CLIBJSON_PARSER_H

#include "../parser2d/types.h"
#include "cjlexer.h"
#include "cjast.h"
#include "cjdom.h"

namespace clib {

    class cparser_json {
    public:
        explicit cparser_json(string_t str);
        ~cparser_json() = default;

        cparser_json(const cparser_json &) = delete;
        cparser_json &operator=(const cparser_json &) = delete;

        ast_node_json *parse();
        ast_node_json *root() const;
        cdom obj();

    private:
        void next();

        void program();
        ast_node_json *object();
        ast_node_json *_object();
        ast_node_json *_list();
        ast_node_json *_pair();

    private:
        void expect(bool, const string_t &);
        void match_operator(operator_t);
        void match_type(lexer_t);
        void match_number();
        void match_integer();

        void error(const string_t &);

    private:
        lexer_t base_type{l_none};

    private:
        clexer_json lexer;
        cast_json ast;
    };
}
#endif //CLIBJSON_PARSER_H