//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include <cstring>
#include <iomanip>
#include "cexception.h"
#include "cast.h"

namespace clib {

    cast::cast() {
        init();
    }

    void cast::init() {
        root = new_node(ast_root);
        current = root;
    }

    void cast::error(const string_t& str) {
        throw cexception(ex_ast, str);
    }

    ast_node* cast::get_root() const {
        return root;
    }

    ast_node* cast::new_node(ast_t type) {
        if (nodes.available() < 64) {
            error("'nodes' out of memory");
        }
        auto node = nodes.alloc<ast_node>();
        if (!node)
            error("'nodes' out of memory");
        memset(node, 0, sizeof(ast_node));
        node->flag = type;
        return node;
    }

    ast_node* cast::set_child(ast_node* node, ast_node* child) {
        child->parent = node;
        if (node->child == nullptr) { // 没有孩子
            node->child = child;
            child->prev = child->next = child;
        }
        else { // 有孩子，添加到末尾
            child->prev = node->child->prev;
            child->next = node->child;
            node->child->prev->next = child;
            node->child->prev = child;
        }
        return node;
    }

    ast_node* cast::set_sibling(ast_node* node, ast_node* sibling) {
        sibling->parent = node->parent;
        sibling->prev = node;
        sibling->next = node->next;
        node->next = sibling;
        return sibling;
    }

    int cast::children_size(ast_node* node) {
        if (!node || !node->child)
            return 0;
        node = node->child;
        auto i = node;
        auto n = 0;
        do {
            n++;
            i = i->next;
            assert(i);
        } while (i != node);
        return n;
    }

    ast_node* cast::add_child(ast_node * node) {
        return set_child(current, node);
    }

    ast_node* cast::new_child(ast_t type, bool step) {
        auto node = new_node(type);
        set_child(current, node);
        if (step)
            current = node;
        return node;
    }

    ast_node* cast::new_sibling(ast_t type, bool step) {
        auto node = new_node(type);
        set_sibling(current, node);
        if (step)
            current = node;
        return node;
    }

    void cast::remove(ast_node * node) {
        if (node->parent && node->parent->child == node) {
            if (node->next == node) {
                node->parent->child = nullptr;
            }
            else {
                node->parent->child = node->next;
            }
        }
        if (node->prev && node->prev != node) {
            node->prev->next = node->next;
        }
        if (node->next && node->next != node) {
            node->next->prev = node->prev;
        }
        if (node->child) {
            auto f = node->child;
            auto i = f;
            i->parent = nullptr;
            if (i->next != f) {
                i = i->next;
                do {
                    i->parent = nullptr;
                    i = i->next;
                } while (i != f);
            }
        }
        nodes.free(node);
    }

    void cast::to(ast_to_t type) {
        switch (type) {
        case to_parent:
            current = current->parent;
            break;
        case to_prev:
            current = current->prev;
            break;
        case to_next:
            current = current->next;
            break;
        case to_child:
            current = current->child;
            break;
        }
    }

    void cast::set_str(ast_node * node, const string_t & str) {
        if (strings.available() < 64) {
            error("'strings' out of memory");
        }
        auto len = str.length();
        auto s = strings.alloc_array<char>(len + 1);
        memcpy(s, str.c_str(), len);
        s[len] = 0;
        node->data._string = s;
    }

    std::string cast::display_str(const char* str) {
        std::stringstream ss;
        for (auto c = str; *c != 0; c++) {
            if (*c < 0) {
                ss << *c;
            }
            else if (isprint(*c)) {
                if (*c == '"')
                    ss << "\\\"";
                else
                    ss << *c;
            }
            else {
                if (*c == '\n')
                    ss << "\\n";
                else
                    ss << ".";
            }
        }
        return ss.str();
    }

    void cast::reset() {
        nodes.clear();
        strings.clear();
        init();
    }

    template<class T>
    static void ast_recursion(ast_node * node, int level, std::ostream & os, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            f(i, level, os);
            return;
        }
        f(i, level, os);
        i = i->next;
        while (i != node) {
            f(i, level, os);
            i = i->next;
        }
    }

    void cast::print(ast_node * node, int level, std::ostream & os) {
        if (node == nullptr)
            return;
        auto rec = [&](auto n, auto l, auto & os) { cast::print(n, l, os); };
        auto type = (ast_t)node->flag;
        if (type != ast_collection)
            os << std::setfill(' ') << std::setw(level) << "";
        switch (type) {
        case ast_root: // 根结点，全局声明
            ast_recursion(node->child, level, os, rec);
            break;
        case ast_collection:
            if ((node->attr & a_exp) && node->child == node->child->next) {
                ast_recursion(node->child, level, os, rec);
            }
            else {
                os << std::setfill(' ') << std::setw(level) << "";
                os << COLL_STRING(node->data._coll) << std::endl;
                ast_recursion(node->child, level + 1, os, rec);
            }
            break;
        case ast_keyword:
            os << "keyword: " << KEYWORD_STRING(node->data._keyword) << std::endl;
            break;
        case ast_operator:
            os << "operator: " << OP_STRING(node->data._op) << std::endl;
            break;
        case ast_literal:
            os << "id: " << node->data._string << std::endl;
            break;
        case ast_string:
            os << "string: " << '"' << display_str(node->data._string) << '"' << std::endl;
            break;
        case ast_char:
            os << "char: ";
            if (isprint(node->data._char))
                os << '\'' << node->data._char << '\'';
            else if (node->data._char == '\n')
                os << "'\\n'";
            else
                os << "'\\x" << std::setiosflags(std::ios::uppercase) << std::hex
                << std::setfill('0') << std::setw(2)
                << (unsigned int)node->data._char << '\'';
            os << std::endl;
            break;
        case ast_uchar:
            os << "uchar: " << (unsigned int)node->data._uchar << std::endl;
            break;
        case ast_short:
            os << "short: " << node->data._short << std::endl;
            break;
        case ast_ushort:
            os << "ushort: " << node->data._ushort << std::endl;
            break;
        case ast_int:
            os << "int: " << node->data._int << std::endl;
            break;
        case ast_uint:
            os << "uint: " << node->data._uint << std::endl;
            break;
        case ast_long:
            os << "long: " << node->data._long << std::endl;
            break;
        case ast_ulong:
            os << "ulong: " << node->data._ulong << std::endl;
            break;
        case ast_float:
            os << "float: " << node->data._float << std::endl;
            break;
        case ast_double:
            os << "double: " << node->data._double << std::endl;
            break;
        }
    }

    string_t cast::to_string(ast_node * node) {
        if (node == nullptr)
            return "";
        std::stringstream ss;
        switch (node->flag) {
        case ast_keyword:
            ss << "keyword: " << KEYWORD_STRING(node->data._keyword);
            break;
        case ast_operator:
            ss << "operator: " << OP_STRING(node->data._op);
            break;
        case ast_literal:
            ss << "id: " << node->data._string;
            break;
        case ast_string:
            ss << "string: " << '"' << display_str(node->data._string) << '"';
            break;
        case ast_char:
            ss << "char: ";
            if (isprint(node->data._char))
                ss << '\'' << node->data._char << '\'';
            else if (node->data._char == '\n')
                ss << "'\\n'";
            else
                ss << "'\\x" << std::setiosflags(std::ios::uppercase) << std::hex
                << std::setfill('0') << std::setw(2)
                << (unsigned int)node->data._char << '\'';
            break;
        case ast_uchar:
            ss << "uchar: " << (unsigned int)node->data._uchar;
            break;
        case ast_short:
            ss << "short: " << node->data._short;
            break;
        case ast_ushort:
            ss << "ushort: " << node->data._ushort;
            break;
        case ast_int:
            ss << "int: " << node->data._int;
            break;
        case ast_uint:
            ss << "uint: " << node->data._uint;
            break;
        case ast_long:
            ss << "long: " << node->data._long;
            break;
        case ast_ulong:
            ss << "ulong: " << node->data._ulong;
            break;
        case ast_float:
            ss << "float: " << node->data._float;
            break;
        case ast_double:
            ss << "double: " << node->data._double;
            break;
        default:
            break;
        }
        return ss.str();
    }

    ast_node* cast::index(ast_node * node, int index) {
        auto child = node->child;
        if (child) {
            if (child->next == child) {
                return index == 0 ? child : nullptr;
            }
            auto head = child;
            for (auto i = 0; i < index; ++i) {
                child = child->next;
                if (child == head)
                    return nullptr;
            }
            return child;
        }
        return nullptr;
    }

    ast_node* cast::index(ast_node * node, const string_t & index) {
        auto child = node->child;
        if (child) {
            if (child->next == child) {
                return index == child->child->data._string ? child : nullptr;
            }
            auto head = child;
            auto i = head;
            do {
                if (index == i->child->data._string)
                    return i->child->next;
                i = i->next;
            } while (i != head);
        }
        return nullptr;
    }

    std::tuple<ast_t, string_t, lexer_t, int> ast_list[] = {
            std::make_tuple(ast_root, "root", l_none, 0),
            std::make_tuple(ast_collection, "coll", l_none, 0),
            std::make_tuple(ast_keyword, "keyword", l_none, 0),
            std::make_tuple(ast_operator, "operator", l_operator , 0),
            std::make_tuple(ast_literal, "literal", l_identifier, 0),
            std::make_tuple(ast_string, "string", l_string, 0),
            std::make_tuple(ast_char, "char", l_char, 1),
            std::make_tuple(ast_uchar, "uchar", l_uchar, 2),
            std::make_tuple(ast_short, "short", l_short, 3),
            std::make_tuple(ast_ushort, "ushort", l_ushort, 4),
            std::make_tuple(ast_int, "int", l_int, 5),
            std::make_tuple(ast_uint, "uint", l_uint, 6),
            std::make_tuple(ast_long, "long", l_long, 7),
            std::make_tuple(ast_ulong, "ulong", l_ulong, 8),
            std::make_tuple(ast_float, "float", l_float, 9),
            std::make_tuple(ast_double, "double", l_double, 10),
    };

    const string_t& cast::ast_str(ast_t type) {
        assert(type >= ast_root && type <= ast_double);
        return std::get<1>(ast_list[type]);
    }

    bool cast::ast_equal(ast_t type, lexer_t lex) {
        assert(type >= ast_root && type <= ast_double);
        return std::get<2>(ast_list[type]) == lex;
    }

    int cast::ast_prior(ast_t type) {
        assert(type >= ast_root && type <= ast_double);
        return std::get<3>(ast_list[type]);
    }

    lexer_t cast::ast_lexer(ast_t type) {
        assert(type >= ast_root && type <= ast_double);
        return std::get<2>(ast_list[type]);
    }

    void cast::unlink(ast_node * node) {
        if (node->parent) {
            auto& parent = node->parent;
            auto& ptr = node;
            auto i = parent->child;
            if (i->next == i) {
                assert(i->prev == i);
                assert(parent->child == node);
                parent->child = nullptr;
                node->parent = nullptr;
                node->prev = node->next = node;
                return;
            }
            if (ptr == parent->child) {
                parent->child = i->next;
                i->prev->next = parent->child;
                parent->child->prev = i->prev;
                node->parent = nullptr;
                node->prev = node->next = node;
                return;
            }
            do {
                if (i->next == ptr) {
                    if (i->next->next == parent->child) {
                        i->next = parent->child;
                        parent->child->prev = i;
                    }
                    else {
                        i->next->next->prev = i;
                        i->next = i->next->next;
                    }
                    break;
                }
                else {
                    i = i->next;
                }
            } while (i != parent->child);
            node->parent = nullptr;
            node->prev = node->next = node;
        }
    }
}
