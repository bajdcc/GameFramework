//
// Project: clibjson
// Created by CC
//

#include "stdafx.h"
#include <iostream>
#include "cjdom.h"
#include "cjast.h"

namespace clib {

    cdom::cdom(cast_json *ast, ast_node_json *node)
            : ast(ast), node(node) {}

    cdom cdom::operator[](int idx) const {
        if (node->flag == ast_json_list) {
            auto _node = cast_json::index(node, idx);
            if (_node)
                return cdom{ast, _node};
        }
        printf("cdom: invalid index %d\n", idx);
        throw std::exception();
    }

    cdom cdom::operator[](string_t idx) const {
        if (node->flag == ast_json_obj) {
            auto _node = cast_json::index(node, idx);
            if (_node)
                return cdom{ast, _node};
        }
        printf("cdom: invalid index '%s'\n", idx.c_str());
        throw std::exception();
    }

    void cdom::output(std::ostream &os) const {
        if (node == nullptr)
            return;
        auto type = (ast_json_t) node->flag;
        switch (type) {
            case ast_json_root:
            case ast_json_obj:
            case ast_json_list:
            case ast_json_pair:
                clib::cast_json::print(node, 0, std::cout);
                break;
            case ast_json_string:
                os << node->data._string;
                break;
            case ast_json_char:
                os << node->data._char;
                break;
            case ast_json_uchar:
                os << node->data._uchar;
                break;
            case ast_json_short:
                os << node->data._short;
                break;
            case ast_json_ushort:
                os << node->data._ushort;
                break;
            case ast_json_int:
                os << node->data._int;
                break;
            case ast_json_uint:
                os << node->data._uint;
                break;
            case ast_json_long:
                os << node->data._long;
                break;
            case ast_json_ulong:
                os << node->data._ulong;
                break;
            case ast_json_float:
                os << node->data._float;
                break;
            case ast_json_double:
                os << node->data._double;
                break;
        }
    }

#define DEFINE_NODE_ASSIGN(t) cdom &cdom::operator=(const LEX_T(t) &v) { \
    node->flag = ast_json_##t; \
    node->data._##t = v; \
    return *this; }

    DEFINE_NODE_ASSIGN(char)
    DEFINE_NODE_ASSIGN(uchar)
    DEFINE_NODE_ASSIGN(short)
    DEFINE_NODE_ASSIGN(ushort)
    DEFINE_NODE_ASSIGN(int)
    DEFINE_NODE_ASSIGN(uint)
    DEFINE_NODE_ASSIGN(long)
    DEFINE_NODE_ASSIGN(ulong)
    DEFINE_NODE_ASSIGN(float)
    DEFINE_NODE_ASSIGN(double)
#undef DEFINE_NODE_DATA

    cdom &cdom::operator=(const string_t &v) {
        node->flag = ast_json_string;
        ast->set_str(node, v);
        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const cdom &dom) {
        dom.output(os);
        return os;
    }
}
