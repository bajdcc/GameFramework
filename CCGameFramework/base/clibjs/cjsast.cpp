//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <cstring>
#include <cassert>
#include <sstream>
#include <iomanip>
#include "cjsast.h"

namespace clib {

    cjsast::cjsast() {
        init();
    }

    void cjsast::init() {
        root = new_node(a_root);
        current = root;
    }

    js_ast_node *cjsast::get_root() const {
        return root;
    }

    js_ast_node *cjsast::new_node(js_ast_t type) {
        auto node = (js_ast_node *) nodes.alloc(sizeof(js_ast_node));
        node->start = node->end = node->line = node->column = 0;
        node->attr = 0;
        node->next = node->prev = node->parent = node->child = nullptr;
        node->data._ins._1 = node->data._ins._2 = 0;
        node->flag = type;
        node->idx = -1;
        return node;
    }

    js_ast_node *cjsast::set_child(js_ast_node *node, js_ast_node *child) {
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

    js_ast_node *cjsast::set_sibling(js_ast_node *node, js_ast_node *sibling) {
        sibling->parent = node->parent;
        sibling->prev = node;
        sibling->next = node->next;
        node->next = sibling;
        return sibling;
    }

    int cjsast::children_size(js_ast_node *node) {
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

    js_ast_node *cjsast::add_child(js_ast_node *node) {
        return set_child(current, node);
    }

    js_ast_node *cjsast::new_child(js_ast_t type, bool step) {
        auto node = new_node(type);
        set_child(current, node);
        if (step)
            current = node;
        return node;
    }

    js_ast_node *cjsast::new_sibling(js_ast_t type, bool step) {
        auto node = new_node(type);
        set_sibling(current, node);
        if (step)
            current = node;
        return node;
    }

    void cjsast::remove(js_ast_node *node) {
        if (node->parent && node->parent->child == node) {
            if (node->next == node) {
                node->parent->child = nullptr;
            } else {
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
        nodes.free((char *) node);
    }

    void cjsast::remove_force(js_ast_node* node) {
        nodes.free((char*)node);
    }

    void cjsast::to(js_ast_to_t type) {
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

    void cjsast::set_str(js_ast_node *node, const std::string &str) {
        auto len = str.length();
        auto s = strings.alloc(len + 1);
        memcpy(s, str.c_str(), len);
        s[len] = 0;
        node->data._string = s;
    }

    std::string cjsast::display_str(const char *str) {
        std::stringstream ss;
        for (auto c = str; *c != 0; c++) {
            if (*c < 0) {
                ss << *c;
            } else if (isprint(*c)) {
                if (*c == '"')
                    ss << "\\\"";
                else
                    ss << *c;
            } else {
                if (*c == '\n')
                    ss << "\\n";
                else
                    ss << ".";
            }
        }
        return ss.str();
    }

    void cjsast::reset() {
        nodes.reset();
        strings.reset();
        init();
    }

    template<class T>
    static void ast_recursion(js_ast_node *node, int level, const std::string &text, std::ostream &os, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            f(i, level, text, os);
            return;
        }
        f(i, level, text, os);
        i = i->next;
        while (i != node) {
            f(i, level, text, os);
            i = i->next;
        }
    }

    void cjsast::print(js_ast_node *node, int level, const std::string &text, std::ostream &os) {
        if (node == nullptr)
            return;
        auto rec = [&](auto n, auto l, const auto &t, auto &os) { cjsast::print(n, l, t, os); };
        auto type = (js_ast_t) node->flag;
        if (type != a_collection)
            os << std::setfill(' ') << std::setw(level) << "";
        switch (type) {
            case a_root: // 根结点，全局声明
                ast_recursion(node->child, level, text, os, rec);
                break;
            case a_collection:
                if ((node->attr & a_exp) && node->child == node->child->next) {
                    ast_recursion(node->child, level, text, os, rec);
                } else {
                    os << std::setfill(' ') << std::setw(level) << "";
                    os << js_coll_string(node->data._coll) << std::endl;
                    ast_recursion(node->child, level + 1, text, os, rec);
                }
                break;
            case a_keyword:
                os << "keyword: " << js_lexer_string(node->data._keyword)
                   << " " << text.substr(node->start, node->end - node->start)
                   << " [" << node->line << ":" << node->column
                   << ":" << node->start << ":" << node->end << "]"
                   << std::endl;
                break;
            case a_operator:
                os << "operator: " << js_lexer_string(node->data._op)
                   << " " << text.substr(node->start, node->end - node->start)
                   << " [" << node->line << ":" << node->column
                   << ":" << node->start << ":" << node->end << "]"
                   << std::endl;
                break;
            case a_literal:
                os << "id: " << node->data._identifier
                   << " [" << node->line << ":" << node->column
                   << ":" << node->start << ":" << node->end << "]"
                   << std::endl;
                break;
            case a_string:
                os << "string: " << display_str(node->data._string)
                   << " [" << node->line << ":" << node->column
                   << ":" << node->start << ":" << node->end << "]"
                   << std::endl;
                break;
            case a_regex:
                os << "regex: " << display_str(node->data._regex)
                   << " [" << node->line << ":" << node->column
                   << ":" << node->start << ":" << node->end << "]"
                   << std::endl;
                break;
            case a_number:
                os << "number: " << node->data._number
                   << " \"" << text.substr(node->start, node->end - node->start) << "\""
                   << " [" << node->line << ":" << node->column
                   << ":" << node->start << ":" << node->end << "]"
                   << std::endl;
                break;
            case a_rule:
                os << "rule: " << js_lexer_string(node->data._op) << std::endl;
                break;
            default:
                break;
        }
    }

    std::string cjsast::to_string(js_ast_node *node) {
        if (node == nullptr)
            return "";
        std::stringstream ss;
        switch (node->flag) {
            case a_keyword:
                ss << "keyword: " << js_lexer_string(node->data._keyword);
                break;
            case a_operator:
                ss << "operator: " << js_lexer_string(node->data._op);
                break;
            case a_literal:
                ss << "id: " << node->data._string;
                break;
            case a_string:
                ss << "string: " << display_str(node->data._string);
                break;
            case a_regex:
                ss << "regex: " << display_str(node->data._regex);
                break;
            case a_number:
                ss << "number: " << node->data._number;
                break;
            case a_rule:
                ss << "rule: " << js_lexer_string(node->data._op);
                break;
            default:
                break;
        }
        return ss.str();
    }

    bool cjsast::ast_equal(js_ast_t type, js_lexer_t lex) {
        switch (type) {
            case a_literal:
                return lex == ID;
            case a_string:
                return lex == STRING;
            case a_regex:
                return lex == REGEX;
            case a_number:
                return lex == NUMBER;
            default:
                break;
        }
        return false;
    }

    js_ast_node *cjsast::index(js_ast_node *node, int index) {
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

    js_ast_node *cjsast::index(js_ast_node *node, const std::string &index) {
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

    void cjsast::unlink(js_ast_node *node) {
        if (node->parent) {
            auto &parent = node->parent;
            auto &ptr = node;
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
                    } else {
                        i->next->next->prev = i;
                        i->next = i->next->next;
                    }
                    break;
                } else {
                    i = i->next;
                }
            } while (i != parent->child);
            node->parent = nullptr;
            node->prev = node->next = node;
        }
    }
}
