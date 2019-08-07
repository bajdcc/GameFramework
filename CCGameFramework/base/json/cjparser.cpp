//
// Project: clibjson
// Author: bajdcc
//

#include "stdafx.h"
#include <sstream>
#include "cjparser.h"
#include "cjlexer.h"
#include "cjast.h"

namespace clib {

    cparser_json::cparser_json(string_t str)
            : lexer(str) {}

    ast_node_json *cparser_json::parse() {
        // 清空词法分析结果
        lexer.reset();
        // 清空AST
        ast.reset();
        // 语法分析（递归下降）
        program();
        return ast.get_root();
    }

    ast_node_json *cparser_json::root() const {
        return ast.get_root();
    }

    cdom cparser_json::obj() {
        return cdom(&ast, root()->child);
    }

    void cparser_json::next() {
        lexer_t token;
        do {
            token = lexer.next();
            if (token == l_error) {
                auto err = lexer.recent_error();
                printf("[%04d:%03d] %-12s - %s\n",
                       err.line,
                       err.column,
                       ERROR_STRING(err.err).c_str(),
                       err.str.c_str());
            }
        } while (token == l_newline || token == l_space || token == l_error);
#if 0
        if (token != l_end) {
            printf("[%04d:%03d] %-12s - %s\n",
                   lexer.get_last_line(),
                   lexer.get_last_column(),
                   LEX_STRING(lexer.get_type()).c_str(),
                   lexer.current().c_str());
        }
#endif
    }

    void cparser_json::program() {
        next();
        ast.add_child(object());
        if (!lexer.is_type(l_end)) {
            error("incomplete json");
        }
    }

    void cparser_json::expect(bool flag, const string_t &info) {
        if (!flag) {
            error(info);
        }
    }

    void cparser_json::match_operator(operator_t type) {
        expect(lexer.is_operator(type), string_t("expect operator " + OPERATOR_STRING(type)));
        next();
    }

    void cparser_json::match_type(lexer_t type) {
        expect(lexer.is_type(type), string_t("expect type " + LEX_STRING(type)));
        next();
    }

    void cparser_json::match_number() {
        expect(lexer.is_number(), "expect number");
        next();
    }

    void cparser_json::match_integer() {
        expect(lexer.is_integer(), "expect integer");
        next();
    }

    void cparser_json::error(const string_t &info) {
        printf("[%04d:%03d] ERROR: %s\n", lexer.get_line(), lexer.get_column(), info.c_str());
        throw std::exception();
    }

    ast_node_json *cparser_json::object() {
        if (lexer.is_type(l_end)) { // 结尾
            error("unexpected token EOF of expression");
        }
        if (lexer.is_type(l_operator)) {
            if (lexer.is_operator(op_lbrace)) {
                return _object();
            } else if (lexer.is_operator(op_lsquare)) {
                return _list();
            }
            error("invalid operator");
        }
        if (lexer.is_number()) {
            ast_node_json *node = nullptr;
            auto type = lexer.get_type();
            switch (type) {
#define DEFINE_NODE_INT(t) \
            case l_##t: \
                node = ast.new_node(ast_json_##t); \
                node->data._##t = lexer.get_##t(); \
                break;
                DEFINE_NODE_INT(char)
                DEFINE_NODE_INT(uchar)
                DEFINE_NODE_INT(short)
                DEFINE_NODE_INT(ushort)
                DEFINE_NODE_INT(int)
                DEFINE_NODE_INT(uint)
                DEFINE_NODE_INT(long)
                DEFINE_NODE_INT(ulong)
                DEFINE_NODE_INT(float)
                DEFINE_NODE_INT(double)
#undef DEFINE_NODE_INT
                default:
                    error("invalid number");
                    break;
            }
            match_number();
            return node;
        }
        if (lexer.is_type(l_string)) {
            std::stringstream ss;
            ss << lexer.get_string();
#if 0
            printf("[%04d:%03d] String> %04X '%s'\n", clexer.get_line(), clexer.get_column(), idx, clexer.get_string().c_str());
#endif
            match_type(l_string);

            while (lexer.is_type(l_string)) {
                ss << lexer.get_string();
#if 0
                printf("[%04d:%03d] String> %04X '%s'\n", clexer.get_line(), clexer.get_column(), idx, clexer.get_string().c_str());
#endif
                match_type(l_string);
            }
            auto node = ast.new_node(ast_json_string);
            ast.set_str(node, ss.str());
            return node;
        }
        if (lexer.is_type(l_identifier)) {
            auto node = ast.new_node(ast_json_string);
            ast.set_str(node, lexer.get_identifier());
            match_type(l_identifier);
            return node;
        }
        error("invalid type");
        return nullptr;
    }

    ast_node_json *cparser_json::_object() {
        next();
        auto node = ast.new_node(ast_json_obj);
        while (!lexer.is_operator(op_rbrace)) {
            cast_json::set_child(node, _pair());
            if (lexer.is_operator(op_comma)) {
                match_operator(op_comma);
            }
        }
        match_operator(op_rbrace);
        return node;
    }

    ast_node_json *cparser_json::_list() {
        next();
        auto node = ast.new_node(ast_json_list);
        while (!lexer.is_operator(op_rsquare)) {
            cast_json::set_child(node, object());
            if (lexer.is_operator(op_comma)) {
                match_operator(op_comma);
            }
        }
        match_operator(op_rsquare);
        return node;
    }

    ast_node_json *cparser_json::_pair() {
        auto node = ast.new_node(ast_json_pair);
        cast_json::set_child(node, object());
        switch (node->child->flag) {
            case ast_json_string:
            case ast_json_int:
                break;
            default:
                error("invalid key type");
        }
        match_operator(op_colon);
        cast_json::set_child(node, object());
        return node;
    }
}
