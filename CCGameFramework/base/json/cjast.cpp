//
// Project: clibjson
// Created by bajdcc
//

#include "stdafx.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include "cjast.h"

namespace clib {

    cast_json::cast_json() {
        init();
    }

    void cast_json::init() {
        root = new_node(ast_json_root);
        current = root;
    }

    ast_node_json *cast_json::get_root() const {
        return root;
    }

    ast_node_json *cast_json::new_node(ast_json_t type) {
        if (nodes.available() < 64) {
            printf("AST ERROR: 'nodes' out of memory\n");
            throw std::exception();
        }
        auto node = nodes.alloc<ast_node_json>();
        memset(node, 0, sizeof(ast_node_json));
        node->flag = type;
        return node;
    }

    ast_node_json *cast_json::set_child(ast_node_json *node, ast_node_json *child) {
        child->parent = node;
        if (node->child == nullptr) { // 没有孩子
            node->child = child;
            child->prev = child->next = child;
        } else { // 有孩子，添加到末尾
            child->prev = node->child->prev;
            child->next = node->child;
            node->child->prev->next = child;
            node->child->prev = child;
        }
        return node;
    }

    ast_node_json *cast_json::set_sibling(ast_node_json *node, ast_node_json *sibling) {
        sibling->parent = node->parent;
        sibling->prev = node;
        sibling->next = node->next;
        node->next = sibling;
        return sibling;
    }

    int cast_json::children_size(ast_node_json *node) {
        if (!node || !node->child)
            return 0;
        node = node->child;
        auto i = node;
        auto n = 0;
        do {
            n++;
            i = i->next;
        } while (i != node);
        return n;
    }

    ast_node_json *cast_json::add_child(ast_node_json *node) {
        return set_child(current, node);
    }

    ast_node_json *cast_json::new_child(ast_json_t type, bool step) {
        auto node = new_node(type);
        set_child(current, node);
        if (step)
            current = node;
        return node;
    }

    ast_node_json *cast_json::new_sibling(ast_json_t type, bool step) {
        auto node = new_node(type);
        set_sibling(current, node);
        if (step)
            current = node;
        return node;
    }

    void cast_json::to(ast_to_json_t type) {
        switch (type) {
            case to_json_parent:
                current = current->parent;
                break;
            case to_json_prev:
                current = current->prev;
                break;
            case to_json_next:
                current = current->next;
                break;
            case to_json_child:
                current = current->child;
                break;
        }
    }

    void cast_json::set_str(ast_node_json *node, const string_t &str) {
        if (strings.available() < 64) {
            printf("AST ERROR: 'strings' out of memory\n");
            throw std::exception();
        }
        auto len = str.length();
        auto s = strings.alloc_array<char>(len + 1);
        memcpy(s, str.c_str(), len);
        s[len] = 0;
        node->data._string = s;
    }

    std::string cast_json::display_str(ast_node_json *node) {
        std::stringstream ss;
        for (auto c = node->data._string; *c != 0; c++) {
            if (*c < 0) {
                ss << *c;
            } else if(isprint(*c)) {
                ss << *c;
            } else {
                if (*c == '\n')
                    ss << "\n";
                else
                    ss << ".";
            }
        }
        return ss.str();
    }

    void cast_json::reset() {
        nodes.clear();
        strings.clear();
        init();
    }

    template<class T>
    static void ast_recursion(ast_node_json *node, int level, std::ostream &os, T f) {
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

    void cast_json::print(ast_node_json *node, int level, std::ostream &os) {
        if (node == nullptr)
            return;
        auto rec = [&](auto n, auto l, auto &os) { cast_json::print(n, l, os); };
        auto type = (ast_json_t) node->flag;
        if (node->parent) {
            if (node->parent->flag == ast_json_list || node->parent->flag == ast_json_obj) {
                os << "\n";
                os << std::setw(level * 4) << "";
            }
        }
        switch (type) {
            case ast_json_root: // 根结点，全局声明
                ast_recursion(node->child, level, os, rec);
                break;
            case ast_json_obj:
                os << '{';
                ast_recursion(node->child, level + 1, os, rec);
                os << "\n";
                os << std::setw(level * 4) << "";
                os << '}';
                break;
            case ast_json_list:
                os << '[';
                ast_recursion(node->child, level + 1, os, rec);
                os << "\n";
                os << std::setw(level * 4) << "";
                os << ']';
                break;
            case ast_json_pair:
                rec(node->child, level, os);
                os << ": ";
                rec(node->child->next, level, os);
                break;
            case ast_json_string:
                os << '"' << display_str(node) << '"';
                break;
            case ast_json_char:
                if (isprint(node->data._char))
                    os << '\'' << node->data._char << '\'';
                else if (node->data._char == '\n')
                    os << "'\\n'";
                else
                    os << "'\\x" << std::setiosflags(std::ios::uppercase) << std::hex
                       << std::setfill('0') << std::setw(2)
                       << (unsigned int) node->data._char << '\'';
                break;
            case ast_json_uchar:
                os << (unsigned int) node->data._uchar;
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
        if (node->parent) {
            if ((node->parent->flag == ast_json_list || node->parent->flag == ast_json_obj) &&
                node->next != node->parent->child) {
                os << ", ";
            }
        }
    }

    ast_node_json *cast_json::index(ast_node_json *node, int index) {
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

    ast_node_json *cast_json::index(ast_node_json *node, const string_t &index) {
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
}
