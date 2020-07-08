//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <queue>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <sstream>
#include "cjsunit.h"

#define SHOW_RULE 0
#define SHOW_LABEL 0
#define SHOW_CLOSURE 0
#define DETECT_LEFT_RECURSION 0

#if DETECT_LEFT_RECURSION
#include <memory>
#include <bitset>
#endif

#define IS_SEQ(type) (type == u_sequence)
#define IS_BRANCH(type) (type == u_branch)
#define IS_COLLECTION(type) (type == u_sequence || type == u_branch || type == u_optional)

namespace clib {
    js_unit_rule *js_to_rule(js_unit *u) {
        assert(u->t == u_rule);
        return (js_unit_rule *) u;
    }

    js_unit_token *js_to_token(js_unit *u) {
        assert(u->t == u_token);
        return (js_unit_token *) u;
    }

    js_unit_collection *js_to_collection(js_unit *u) {
        assert(IS_COLLECTION(u->t));
        return (js_unit_collection *) u;
    }

    js_unit_collection *js_to_ref(js_unit *u) {
        assert(u->t == u_token_ref || u->t == u_rule_ref);
        return (js_unit_collection *) u;
    }

    js_unit &js_unit::operator=(const js_unit &u) {
        auto rule = js_to_rule(this);
        rule->child = builder->copy(const_cast<js_unit *>(&u));
        return *this;
    }

    js_unit &js_unit::operator+(const js_unit &u) {
        if ((!IS_COLLECTION(t) && !IS_COLLECTION(u.t)) ||
            (IS_BRANCH(t) && IS_BRANCH(u.t))) {
            return builder->collection(this, const_cast<js_unit *>(&u), u_sequence);
        } else if (IS_SEQ(t) && IS_SEQ(u.t)) {
            return builder->merge(this, const_cast<js_unit *>(&u));
        } else if (IS_SEQ(t)) {
            return builder->append(this, const_cast<js_unit *>(&u));
        } else if (IS_SEQ(u.t)) {
            return builder->append(const_cast<js_unit *>(&u), this);
        } else {
            return builder->collection(this, const_cast<js_unit *>(&u), u_sequence);
        }
    }

    js_unit &js_unit::operator|(const js_unit &u) {
        if ((!IS_COLLECTION(t) && !IS_COLLECTION(u.t)) ||
            (IS_SEQ(t) && IS_SEQ(u.t))) {
            return builder->collection(this, const_cast<js_unit *>(&u), u_branch);
        } else if (IS_BRANCH(t) && IS_BRANCH(u.t)) {
            return builder->merge(this, const_cast<js_unit *>(&u));
        } else if (IS_BRANCH(t)) {
            return builder->append(this, const_cast<js_unit *>(&u));
        } else if (IS_BRANCH(u.t)) {
            return builder->append(this, const_cast<js_unit *>(&u));
        } else {
            return builder->collection(this, const_cast<js_unit *>(&u), u_branch);
        }
    }

    js_unit &js_unit::operator*() {
        return builder->optional(this);
    }

    js_unit &js_unit::operator~() {
        if (t == u_token)
            return js_to_ref(builder->copy(this))->set_skip(true);
        if (t == u_token_ref) {
            if (!js_to_ref(this)->marked)
                return js_to_ref(this)->set_marked(true);
            else
                return js_to_ref(this)->set_skip(false);
        }
        assert(!"invalid type");
        return *this;
    }

    js_unit &js_unit::init(js_unit_builder *builder) {
        next = prev = nullptr;
        this->builder = builder;
        return *this;
    }

    js_unit &js_unit::set_t(js_unit_t type) {
        this->t = type;
        return *this;
    }

    js_unit &js_unit::operator()(void *cb) {
        return js_to_ref(builder->copy(this))->set_cb(cb);
    }

    js_unit_token &js_unit_token::set_type(js_lexer_t type) {
        this->type = type;
        return *this;
    }

    js_unit_collection &js_unit_collection::set_skip(bool skip) {
        this->skip = skip;
        return *this;
    }

    js_unit_collection &js_unit_collection::set_marked(bool marked) {
        this->marked = marked;
        return *this;
    }

    js_unit_collection &js_unit_collection::set_child(js_unit *node) {
        child = node;
        return *this;
    }

    js_unit_collection &js_unit_collection::set_cb(void *cb) {
        callback = cb;
        return *this;
    }

    js_unit_rule &js_unit_rule::set_s(const char *str) {
        s = str;
        return *this;
    }

    js_unit_rule &js_unit_rule::set_attr(uint32_t attr) {
        this->attr = attr;
        return *this;
    }

    const char *cjsunit::str(const std::string &s) {
        auto f = strings.find(s);
        if (f == strings.end()) {
            return strings.insert(s).first->c_str();
        }
        return f->c_str();
    }

    js_unit &cjsunit::token(const js_lexer_t &type) {
        return (*(js_unit_token *) nodes.alloc(sizeof(js_unit_token))).set_type(type).set_t(u_token).init(this);
    }

    js_unit *cjsunit::copy(js_unit *u) {
        if (u->t == u_token) { // copy token js_unit
            return &(*(js_unit_collection *) nodes.alloc(sizeof(js_unit_collection)))
                    .set_skip(false).set_marked(false).set_child(u)
                    .set_t(u_token_ref).init(this);
        }
        if (u->t == u_rule) { // copy rule js_unit
            return &(*(js_unit_collection *) nodes.alloc(sizeof(js_unit_collection)))
                    .set_skip(false).set_marked(false).set_child(u)
                    .set_t(u_rule_ref).init(this);
        }
        return u;
    }

    js_unit_collection &cjsunit::append(js_unit *collection, js_unit *child) {
        auto node = js_to_collection(collection);
        child = copy(child);
        if (node->child == nullptr) { // 没有孩子
            node->child = child;
            child->prev = child->next = child;
        } else { // 有孩子，添加到末尾
            child->prev = node->child->prev;
            child->next = node->child;
            node->child->prev->next = child;
            node->child->prev = child;
        }
        return *node;
    }

    js_unit_collection &cjsunit::merge(js_unit *a, js_unit *b) {
        auto nodeA = js_to_collection(a)->child;
        auto nodeB = js_to_collection(b)->child;
        nodeA->prev->next = nodeB;
        nodeB->prev->next = nodeA;
        std::swap(nodeA->prev, nodeB->prev);
        nodes.free((char *) b);
        return *js_to_collection(a);
    }

    js_unit_collection &cjsunit::collection(js_unit *a, js_unit *b, js_unit_t type) {
        a = copy(a);
        b = copy(b);
        a->next = a->prev = b;
        b->next = b->prev = a;
        return (js_unit_collection &) (*(js_unit_collection *) nodes.alloc(sizeof(js_unit_collection)))
                .set_child(a).set_t(type).init(this);
    }

    js_unit_collection &cjsunit::optional(js_unit *a) {
        a = copy(a);
        a->next = a->prev = a;
        return (js_unit_collection &) (*(js_unit_collection *) nodes.alloc(sizeof(js_unit_collection)))
                .set_child(a).set_t(u_optional).init(this);
    }

    js_unit &cjsunit::rule(const std::string &s, js_coll_t t, uint32_t attr) {
        auto f = rules.find(s);
        if (f == rules.end()) {
            auto &rule = (*(js_unit_rule *) nodes.alloc(sizeof(js_unit_rule)))
                    .set_s(str(s))
                    .set_attr(attr)
                    .set_child(nullptr)
                    .set_t(u_rule)
                    .init(this);
            js_nga_rule r;
            r.id = rules.size();
            r.status = nullptr;
            r.u = js_to_rule(&rule);
            r.recursive = 0;
            rulesMap.insert(std::make_pair(r.u->s, t));
            rules.insert(std::make_pair(s, r));
            return *r.u;
        }
        return *f->second.u;
    }

    template<class T>
    static void js_unit_recursion(js_unit *node, js_unit *parent, std::ostream &os, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            f(i, parent, os);
            return;
        }
        f(i, parent, os);
        i = i->next;
        while (i != node) {
            f(i, parent, os);
            i = i->next;
        }
    }

    void print(js_unit *node, js_unit *parent, std::ostream &os) {
        if (node == nullptr)
            return;
        auto rec = [&](auto n, auto p, auto &os) { print(n, p, os); };
        auto type = node->t;
        switch (type) {
            case u_none:
                break;
            case u_token: {
                auto token = js_to_token(node);
                if (token->type > KEYWORD_START && token->type < KEYWORD_END) {
                    os << js_lexer_string(token->type);
                } else if (token->type > OPERATOR_START && token->type < OPERATOR_END) {
                    os << "'" << js_lexer_string(token->type) << "'";
                } else {
                    os << "#" << js_lexer_string(token->type) << "#";
                }
            }
                break;
            case u_token_ref: {
                auto t = js_to_ref(node);
                if (t->skip)
                    os << "(";
                rec(t->child, node, os);
                if (t->skip)
                    os << ")";
            }
                break;
            case u_rule: {
                auto rule = js_to_rule(node);
                os << rule->s << " => ";
                rec(rule->child, node, os);
            }
                break;
            case u_rule_ref: {
                os << js_to_rule(js_to_ref(node)->child)->s;
            }
                break;
            case u_sequence:
            case u_branch:
            case u_optional: {
                auto co = js_to_collection(node);
                if (node->t == u_branch)
                    os << "( ";
                else if (node->t == u_optional)
                    os << "[ ";
                js_unit_recursion(co->child, node, os, rec);
                if (node->t == u_branch)
                    os << " )";
                else if (node->t == u_optional)
                    os << " ]";
            }
                break;
            default:
                break;
        }
        if (parent) {
            if (IS_COLLECTION(parent->t)) {
                auto p = js_to_collection(parent);
                if (node->next != p->child)
                    os << (IS_SEQ(p->t) ? " " : " | ");
            }
        }
    }

    void cjsunit::gen(js_unit *root) {
        gen_nga();
        check_nga();
        gen_pda(root);
    }

    void cjsunit::gen_nga() {
        for (auto &rule : rules) {
            current_rule = js_to_rule(rule.second.u);
#if SHOW_RULE
            print(current_rule, nullptr, std::cout);
            std::cout << std::endl;
#endif
            assert(current_rule->child);
            rule.second.status = delete_epsilon(conv_nga(current_rule->child));
        }
        current_rule = nullptr;
    }

    template<class T>
    static std::vector<T *> get_children(T *node) {
        std::vector<T *> v;
        if (node == nullptr)
            return v;
        auto i = node;
        if (i->next == i) {
            v.push_back(i);
            return v;
        }
        v.push_back(i);
        i = i->next;
        while (i != node) {
            v.push_back(i);
            i = i->next;
        }
        return v;
    }

    bool get_first_set(js_unit *node, js_nga_rule &rule) {
        if (node == nullptr)
            return true;
        switch (node->t) {
            case u_none:
                break;
            case u_token_ref: {
                rule.tokensFirstset.insert(js_to_token(js_to_ref(node)->child));
                return false;
            }
            case u_rule_ref: {
                rule.rulesFirstset.insert(js_to_rule(js_to_ref(node)->child));
                return false;
            }
            case u_sequence: {
                for (auto &u : get_children(js_to_collection(node)->child)) {
                    if (!get_first_set(u, rule)) {
                        return false;
                    }
                }
                return true;
            }
            case u_branch: {
                auto zero = false;
                for (auto &u : get_children(js_to_collection(node)->child)) {
                    if (get_first_set(u, rule) && !zero) {
                        zero = true;
                    }
                }
                return zero;
            }
            case u_optional: {
                for (auto &u : get_children(js_to_collection(node)->child)) {
                    get_first_set(u, rule);
                }
                return true;
            }
            default:
                break;
        }
        return false;
    }

    void cjsunit::check_nga() {
        for (auto &rule : rules) {
            js_nga_rule &r = rule.second;
            if (get_first_set(js_to_rule(r.u)->child, r)) {
                error("generate epsilon: " + print_unit(r.u));
            }
        }
        auto size = rules.size();
        std::vector<js_nga_rule *> rules_list(size);
        std::vector<std::vector<bool>> dep(size);
        std::unordered_map<js_unit *, int> ids;
        for (auto &rule : rules) {
            rules_list[rule.second.id] = &rule.second;
            ids.insert(std::make_pair(rule.second.u, rule.second.id));
        }
        for (size_t i = 0; i < size; ++i) {
            dep[i].resize(size);
            for (auto &r : rules_list[i]->rulesFirstset) {
                dep[i][ids[r]] = true;
            }
        }
        for (size_t i = 0; i < size; ++i) {
            if (dep[i][i]) {
                rules_list[i]->recursive = 1;
                dep[i][i] = false;
            }
        }
#if DETECT_LEFT_RECURSION
        {
            // INDIRECT LEFT RECURSION DETECTION
            // 由于VS的vector在DEBUG下性能太慢，因此选择bitset
            static const int MAX_SIZE = 512;
            static const int MAX_SIZE_Q = MAX_SIZE * MAX_SIZE;
            if (size >= MAX_SIZE) throw std::bad_alloc();
            std::unique_ptr<std::bitset<MAX_SIZE_Q>> a, b, r;
            a = std::make_unique<std::bitset<MAX_SIZE_Q>>();
            b = std::make_unique<std::bitset<MAX_SIZE_Q>>();
            r = std::make_unique<std::bitset<MAX_SIZE_Q>>();
            for (size_t l = 2; l < size; ++l) {
                for (size_t i = 0; i < size; ++i) {
                    for (size_t j = 0; j < size; ++j) {
                        r->reset(i * size + j);
                        for (size_t k = 0; k < size; ++k) {
                            if (a->test(i * size + k) && b->test(k * size + j)) {
                                r->set(i * size + j);
                                break;
                            }
                        }
                    }
                }
                for (size_t i = 0; i < size; ++i) {
                    if (r->test(i * (size + 1))) {
                        if (rules_list[i]->recursive < 2)
                            rules_list[i]->recursive = l;
                    }
                }
                *a = *r;
            }
            for (size_t i = 0; i < size; ++i) {
                if (rules_list[i]->recursive > 1) {
                    error("indirect left recursion: " + print_unit(rules_list[i]->u));
                }
            }
        }
#endif
        {
            // CALCULATE FIRST SET
            std::vector<bool> visited(size);
            for (size_t i = 0; i < size; ++i) {
                auto indep = -1;
                for (size_t j = 0; j < size; ++j) {
                    if (!visited[j]) {
                        auto flag = true;
                        for (size_t k = 0; k < size; ++k) {
                            if (dep[j][k]) {
                                flag = false;
                                break;
                            }
                        }
                        if (flag) {
                            indep = j;
                            break;
                        }
                    }
                }
                if (indep == -1) {
                    error("missing most independent rule");
                }
                for (auto &r : rules_list[indep]->rulesFirstset) {
                    auto &a = rules_list[indep]->tokensFirstset;
                    auto &b = rules_list[ids[r]]->tokensList;
                    a.insert(b.begin(), b.end());
                }
                {
                    auto &a = rules_list[indep]->tokensList;
                    auto &b = rules_list[indep]->tokensFirstset;
                    a.insert(b.begin(), b.end());
                    auto &c = rules_list[indep]->rulesFirstset;
                    if (c.find(rules_list[indep]->u) != c.end()) {
                        b.insert(a.begin(), a.end());
                    }
                }
                visited[indep] = true;
                for (size_t j = 0; j < size; ++j) {
                    dep[j][indep] = false;
                }
            }
            for (size_t i = 0; i < size; ++i) {
                if (rules_list[i]->tokensFirstset.empty()) {
                    error("empty first set: " + print_unit(rules_list[i]->u));
                }
            }
        }
    }

    template<class T>
    static void nga_recursion(js_unit *u, std::vector<js_nga_edge *> &v, T f) {
        auto node = js_to_collection(u)->child;
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            v.push_back(f(i));
            return;
        }
        v.push_back(f(i));
        i = i->next;
        while (i != node) {
            v.push_back(f(i));
            i = i->next;
        }
    }

    js_nga_edge *cjsunit::conv_nga(js_unit *u) {
        if (u == nullptr)
            return nullptr;
        auto rec = [&](auto u) { return conv_nga(u); };
        switch (u->t) {
            case u_token_ref:
            case u_rule_ref: {
                auto enga = u->builder->enga(u, u);
                enga->skip = js_to_ref(u)->skip;
                enga->marked = js_to_ref(u)->marked;
                enga->cb = js_to_ref(u)->callback;
                return enga;
            }
            case u_sequence: {
                auto enga = u->builder->enga(u, false);
                enga->data = u;
                std::vector<js_nga_edge *> edges;
                nga_recursion(u, edges, rec);
                for (auto &edge : edges) {
                    if (enga->begin != nullptr) {
                        u->builder->connect(enga->end, edge->begin, false);
                        enga->end = edge->end;
                    } else {
                        enga->begin = edge->begin;
                        enga->end = edge->end;
                    }
                }
                return enga;
            }
            case u_branch: {
                auto enga = u->builder->enga(u, true);
                enga->data = u;
                std::vector<js_nga_edge *> edges;
                nga_recursion(u, edges, rec);
                for (auto &edge : edges) {
                    u->builder->connect(enga->begin, edge->begin, false);
                    u->builder->connect(edge->end, enga->end, false);
                }
                return enga;
            }
            case u_optional: {
                std::vector<js_nga_edge *> edges;
                nga_recursion(u, edges, rec);
                auto &edge = edges.front();
                return u->builder->connect(edge->begin, edge->end, false);
            }
            default:
                break;
        }
        assert(!"not supported");
        return nullptr;
    }

    js_nga_edge *cjsunit::enga(js_unit *node, bool init) {
        auto _enga = (js_nga_edge *) nodes.alloc(sizeof(js_nga_edge));
        _enga->data = nullptr;
        _enga->skip = false;
        _enga->marked = false;
        _enga->cb = nullptr;
        if (init) {
            _enga->begin = status();
            _enga->end = status();
            _enga->begin->label = label(node, true);
            _enga->end->label = label(node, false);
        } else {
            _enga->begin = _enga->end = nullptr;
        }
        return _enga;
    }

    js_nga_edge *cjsunit::enga(js_unit *node, js_unit *u) {
        auto begin = status();
        auto end = status();
        begin->label = label(node, true);
        end->label = label(node, false);
        auto _enga = connect(begin, end, false);
        _enga->data = u;
        return _enga;
    }

    js_nga_edge *cjsunit::connect(js_nga_status *a, js_nga_status *b, bool is_pda) {
        auto new_edge = is_pda ? (js_pda_edge *) nodes.alloc(sizeof(js_pda_edge))
                               : (js_nga_edge *) nodes.alloc(sizeof(js_nga_edge));
        new_edge->begin = a;
        new_edge->end = b;
        new_edge->data = nullptr;
        add_edge(a->out, new_edge);
        add_edge(b->in, new_edge);
        return new_edge;
    }

    void cjsunit::disconnect(js_nga_status *status) {
        auto ins = get_children(status->in);
        for (auto &edge : ins) {
            remove_edge(edge->edge->end->in, edge);
            nodes.free((char *) edge);
        }
        auto outs = get_children(status->out);
        for (auto &edge : outs) {
            remove_edge(edge->edge->end->in, edge);
            nodes.free((char *) edge);
        }
        nodes.free((char *) status);
    }

    js_nga_status *cjsunit::status() {
        auto _status = (js_nga_status *) nodes.alloc(sizeof(js_nga_status));
        _status->final = false;
        _status->label = nullptr;
        _status->in = _status->out = nullptr;
        return _status;
    }

    js_pda_status *cjsunit::status(const char *label, int rule, bool final) {
        auto _status = (js_pda_status *) nodes.alloc(sizeof(js_pda_status));
        _status->label = label;
        _status->final = final;
        _status->in = _status->out = nullptr;
        _status->rule = rule;
        return _status;
    }

    void cjsunit::add_edge(js_nga_edge_list *&list, js_nga_edge *edge) {
        if (list == nullptr) {
            list = (js_nga_edge_list *) nodes.alloc(sizeof(js_nga_edge_list));
            list->edge = edge;
            list->prev = list->next = list;
        } else {
            auto new_edge = (js_nga_edge_list *) nodes.alloc(sizeof(js_nga_edge_list));
            new_edge->edge = edge;
            new_edge->prev = list->prev;
            new_edge->next = list;
            list->prev->next = new_edge;
            list->prev = new_edge;
        }
    }

    void cjsunit::remove_edge(js_nga_edge_list *&list, js_nga_edge_list *edge) {
        if (list == nullptr) {
            error("remove from empty edge list");
        } else if (list->next == list) {
            assert(list->edge == edge->edge);
            list = nullptr;
            nodes.free((char *) list);
        } else {
            if (list->edge == edge->edge) {
                list->prev->next = list->next;
                list->next->prev = list->prev;
                list = list->prev;
                nodes.free((char *) edge);
            } else {
                auto node = list->next;
                while (node != list) {
                    if (node->edge == edge->edge) {
                        node->prev->next = node->next;
                        node->next->prev = node->prev;
                        nodes.free((char *) node);
                        return;
                    }
                    node = node->next;
                }
                error("remove nothing from edge list");
            }
        }
    }

    const char *cjsunit::label(js_unit *focused, bool front) {
        std::stringstream ss;
        label(current_rule, nullptr, focused, front, ss);
#if SHOW_LABEL
        fprintf(stdout, "%s\n", ss.str().c_str());
#endif
        labels.emplace_back(ss.str());
        return labels.back().c_str();
    }

    template<class T>
    static void
    label_recursion(js_unit *node, js_unit *parent, js_unit *focused, bool front, std::ostream &os, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            f(i, parent, focused, front, os);
            return;
        }
        f(i, parent, focused, front, os);
        i = i->next;
        while (i != node) {
            f(i, parent, focused, front, os);
            i = i->next;
        }
    }

    void cjsunit::label(js_unit *node, js_unit *parent, js_unit *focused, bool front, std::ostream &os) {
        if (node == nullptr)
            return;
        auto rec = [this](auto _1, auto _2, auto _3, auto _4, auto &_5) {
            label(_1, _2, _3, _4, _5);
        };
        if (front && node == focused) {
            os << "@ ";
        }
        switch (node->t) {
            case u_none:
                break;
            case u_token: {
                auto token = js_to_token(node);
                if (token->type > KEYWORD_START && token->type < KEYWORD_END) {
                    os << js_lexer_string(token->type);
                } else if (token->type > OPERATOR_START && token->type < OPERATOR_END) {
                    os << "'" << js_lexer_string(token->type) << "'";
                } else {
                    os << "#" << js_lexer_string(token->type) << "#";
                }
            }
                break;
            case u_token_ref: {
                auto t = js_to_ref(node);
                if (t->skip)
                    os << "(";
                rec(js_to_ref(node)->child, node, focused, front, os);
                if (t->skip)
                    os << ")";
            }
                break;
            case u_rule: {
                auto _rule = js_to_rule(node);
                os << _rule->s << " => ";
                rec(_rule->child, node, focused, front, os);
            }
                break;
            case u_rule_ref: {
                os << js_to_rule(js_to_ref(node)->child)->s;
            }
                break;
            case u_sequence:
            case u_branch:
            case u_optional: {
                auto co = js_to_collection(node);
                if (node->t == u_branch)
                    os << "( ";
                else if (node->t == u_optional)
                    os << "[ ";
                label_recursion(co->child, node, focused, front, os, rec);
                if (node->t == u_branch)
                    os << " )";
                else if (node->t == u_optional)
                    os << " ]";
            }
                break;
            default:
                break;
        }
        if (!front && node == focused) {
            os << " @";
        }
        if (parent) {
            if (IS_COLLECTION(parent->t)) {
                auto p = js_to_collection(parent);
                if (node->next != p->child)
                    os << (IS_SEQ(p->t) ? " " : " | ");
            }
        }
    }

    template<class T>
    static bool has_filter_in_edges(js_nga_edge_list *node, const T &f) {
        if (node == nullptr)
            return false;
        auto i = node;
        if (i->next == i) {
            return f(i->edge);
        }
        if (f(i->edge))
            return true;
        i = i->next;
        while (i != node) {
            if (f(i->edge))
                return true;
            i = i->next;
        }
        return false;
    }

    template<class T>
    static std::vector<js_nga_edge_list *> get_filter_out_edges(js_nga_status *status, const T &f) {
        std::vector<js_nga_edge_list *> v;
        auto node = status->out;
        if (node == nullptr)
            return v;
        auto i = node;
        if (i->next == i) {
            if (f(i->edge))
                v.push_back(i);
            return v;
        }
        if (f(i->edge))
            v.push_back(i);
        i = i->next;
        while (i != node) {
            if (f(i->edge))
                v.push_back(i);
            i = i->next;
        }
        return v;
    }

    template<class T>
    std::vector<js_nga_status *> get_closure(js_nga_status *status, const T &f) {
        std::vector<js_nga_status *> v;
        std::queue<js_nga_status *> queue;
        std::unordered_set<js_nga_status *> set;
        queue.push(status);
        set.insert(status);
        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();
            v.push_back(current);
            auto out_ptr = current->out;
            if (out_ptr) {
                auto out = out_ptr->edge->end;
                if (f(out_ptr->edge) && set.find(out) == set.end()) {
                    set.insert(out);
                    queue.push(out);
                }
                if (out_ptr->next != out_ptr) {
                    auto head = out_ptr;
                    out_ptr = out_ptr->next;
                    while (out_ptr != head) {
                        out = out_ptr->edge->end;
                        if (f(out_ptr->edge) && set.find(out) == set.end()) {
                            set.insert(out);
                            queue.push(out);
                        }
                        out_ptr = out_ptr->next;
                    }
                }
            }
        }
        return v;
    }

    std::string cjsunit::print_unit(js_unit *u) {
        std::stringstream ss;
        print(u, nullptr, ss);
        return ss.str();
    }

    void cjsunit::error(const std::string &str) {
        throw cjs_exception(str);
    }

    js_nga_status *cjsunit::delete_epsilon(js_nga_edge *edge) {
        edge->end->final = true;
        auto nga_status_list = get_closure(edge->begin, [](auto it) { return true; });
#if SHOW_CLOSURE
        for (auto& c : nga_status_list) {
            fprintf(stdout, "%s\n", c->label);
        }
#endif
        // TODO: Complete Delete Epsilon
        std::vector<js_nga_status *> available_status;
        std::vector<const char *> available_labels;
        std::unordered_map<size_t, size_t> available_labels_map;
        available_labels.emplace_back(nga_status_list[0]->label);
        available_labels_map.insert(
                std::make_pair(std::hash<std::string>{}(std::string(nga_status_list[0]->label)), available_status.size()));
        available_status.push_back(nga_status_list[0]);
        for (auto _status = nga_status_list.begin() + 1; _status != nga_status_list.end(); _status++) {
            auto &status = *_status;
            if (has_filter_in_edges(status->in, [](auto it) { return it->data != nullptr; })
                && available_labels_map.find(std::hash<std::string>{}(status->label)) ==
                   available_labels_map.end()) {
                available_labels.emplace_back(status->label);
                available_labels_map.insert(
                        std::make_pair(std::hash<std::string>{}(status->label), available_status.size()));
                available_status.push_back(status);
            }
        }
        for (auto &status : available_status) {
            auto epsilon_closure = get_closure(status, [](auto it) { return it->data == nullptr; });
            for (auto &epsilon : epsilon_closure) {
                if (epsilon == status)
                    continue;
                if (epsilon->final)
                    status->final = true;
                auto out_edges = get_filter_out_edges(epsilon, [](auto it) { return it->data != nullptr; });
                for (auto &out_edge : out_edges) {
                    auto idx = available_labels_map.at(std::hash<std::string>{}(out_edge->edge->end->label));
                    auto e = connect(status, available_status[idx], false);
                    e->data = out_edge->edge->data;
                    e->skip = out_edge->edge->skip;
                    e->marked = out_edge->edge->marked;
                    e->cb = out_edge->edge->cb;
                }
            }
        }
        for (auto &status : nga_status_list) {
            auto out_edges = get_filter_out_edges(status, [](auto it) { return it->data == nullptr; });
            for (auto &out_edge : out_edges) {
                remove_edge(out_edge->edge->end->in, out_edge);
                remove_edge(status->out, out_edge);
            }
        }
        for (auto &status : nga_status_list) {
            if (std::find(available_status.begin(), available_status.end(), status) == available_status.end()) {
                disconnect(status);
            }
        }
        return edge->begin;
    }

    bool not_left_recursive_status(js_nga_status *status, js_unit *rule) {
        if (status->in)
            return true;
        return status->out &&
               has_filter_in_edges(
                       status->out,
                       [rule](auto it) {
                           return it->data &&
                                  !(it->data->t == u_rule_ref &&
                                    js_to_ref(it->data)->child ==
                                    rule);
                       });
    }

    bool is_left_resursive_edge(js_nga_edge *edge, js_unit *rule) {
        return !edge->begin->in && edge->data &&
               edge->data->t == u_rule_ref && js_to_ref(edge->data)->child == rule;
    }

    void cjsunit::gen_pda(js_unit *root) {
        auto size = rules.size();
        js_pda_status *init_status = nullptr;
        struct status_t {
            js_nga_status *nga;
            js_pda_status *pda;
        };
        std::vector<status_t> status_list;
        std::vector<js_nga_rule *> rules_list(size);
        std::vector<std::vector<js_nga_edge *>> adj(size);
        std::unordered_map<js_unit *, int> ids;
        std::unordered_map<js_pda_edge *, std::unordered_set<js_unit *>> LA;
        std::unordered_map<js_pda_edge *, js_pda_status *> prev;
        for (auto &rule : rules) {
            rules_list[rule.second.id] = &rule.second;
            ids.insert(std::make_pair(rule.second.u, rule.second.id));
        }
        for (auto &rule : rules) {
            js_nga_rule &r = rule.second;
            auto closure = get_closure(r.status, [](auto it) { return true; });
            for (auto &s : closure) {
                auto outs = get_filter_out_edges(s, [](auto it) { return it->data && it->data->t == u_rule_ref; });
                for (auto &o : outs) {
                    adj[ids[js_to_ref(o->edge->data)->child]].push_back(o->edge);
                }
                status_list.push_back({s, status(s->label, ids[r.u], s->final)});
            }
        }
        // GENERATE PDA STATUS
        for (size_t i = 0; i < status_list.size(); ++i) {
            auto &s = status_list[i];
            auto &nga_status = s.nga;
            auto &pda_status = s.pda;
            auto &ru = rules_list[pda_status->rule];
            auto &r = ru->u;
            if (not_left_recursive_status(nga_status, r)) {
                auto is_init = root == r;
                if (!nga_status->in && is_init) {
                    init_status = pda_status; // INITIAL STATE
                }
                auto outs = get_children(nga_status->out);
                std::unordered_set<js_unit *> token_set;
                for (auto &o : outs) {
                    auto node = o->edge->data;
                    if (node->t == u_rule_ref) {
                        if (!is_left_resursive_edge(o->edge, r)) {
                            auto rr = js_to_ref(o->edge->data)->child;
                            auto nga = rules_list[ids[rr]]->status;
                            if (not_left_recursive_status(nga, r)) {
                                // SHIFT
                                auto &edge = *(js_pda_edge *) connect(
                                        pda_status, std::find_if(
                                                status_list.begin(),
                                                status_list.end(),
                                                [nga](auto it) { return it.nga == nga; })->pda,
                                        true);
                                edge.data = node;
                                edge.type = e_shift;
                                decltype(token_set) res;
                                auto &fs = rules_list[ids[rr]]->tokensFirstset;
                                res.insert(fs.begin(), fs.end());
                                LA.insert(std::make_pair(&edge, res));
                            }
                        }
                    } else if (node->t == u_token_ref) {
                        // MOVE
                        auto &edge = *(js_pda_edge *) connect(
                                pda_status, std::find_if(
                                        status_list.begin(),
                                        status_list.end(),
                                        [o](auto it) { return it.nga == o->edge->end; })->pda,
                                true);
                        edge.data = node;
                        auto n = js_to_ref(node);
                        edge.type = n->skip ? e_pass : e_move;
                        auto t = js_to_token(n->child)->type;
                        if (t > RULE_START && t < RULE_END)
                            edge.type = e_rule;
                        edge.marked = n->marked;
                        edge.cb = n->callback;
                        token_set.insert(n->child);
                        decltype(token_set) res{n->child};
                        LA.insert(std::make_pair(&edge, res));
                    }
                }
                // FINAL
                if (nga_status->final) {
                    for (auto &o : adj[pda_status->rule]) {
                        auto &edge = *(js_pda_edge *) connect(
                                pda_status, std::find_if(
                                        status_list.begin(),
                                        status_list.end(),
                                        [o](auto it) { return it.nga == o->end; })->pda,
                                true);
                        edge.data = o->data;
                        auto _rule = rules_list[std::find_if(
                                status_list.begin(),
                                status_list.end(),
                                [o](auto it) { return it.nga == o->begin; })->pda->rule]->u;
                        if (is_left_resursive_edge(o, _rule)) {
                            // LEFT RECURSION
                            edge.type = _rule->attr & r_not_greed ? e_left_recursion_not_greed : e_left_recursion;
                            decltype(token_set) res;
                            auto _outs = get_filter_out_edges(edge.end, [](auto it) { return true; });
                            for (auto &_o : _outs) {
                                auto _d = _o->edge->data;
                                if (_d->t == u_token_ref) {
                                    res.insert(js_to_ref(_d)->child);
                                } else if (_d->t == u_rule_ref) {
                                    auto _r = rules_list[ids[js_to_ref(_d)->child]]->tokensFirstset;
                                    res.insert(_r.begin(), _r.end());
                                } else {
                                    error("invalid edge type: " + print_unit(_d));
                                }
                            }
                            LA.insert(std::make_pair(&edge, res));
                        } else {
                            // REDUCE
                            edge.type = _rule->attr & r_exp ? e_reduce_exp : e_reduce;
                            prev.insert(std::make_pair(&edge, std::find_if(
                                    status_list.begin(),
                                    status_list.end(),
                                    [o](auto it) { return it.nga == o->begin; })->pda));
                            LA.insert(std::make_pair(&edge, decltype(token_set)()));
                        }
                    }
                    if (is_init) {
                        auto &edge = *(js_pda_edge *) connect(pda_status, pda_status, true);
                        edge.type = e_finish;
                        LA.insert(std::make_pair(&edge, decltype(token_set)()));
                    }
                }
            }
        }
        std::unordered_map<std::string, int> mapLabelsToPda;
        // GENERATE PDA TABLE
        {
            auto closure = get_closure(init_status, [](auto it) { return true; });
            std::unordered_map<js_pda_status *, int> pids;
            for (auto &c : closure) {
                pids.insert(std::make_pair((js_pda_status *) c, (int) pids.size()));
            }
            for (size_t i = 0; i < closure.size(); ++i) {
                auto c = (js_pda_status *) closure[i];
                js_pda_rule pda{};
                pda.id = i;
                pda.rule = c->rule;
                pda.final = c->final;
                pda.coll = rulesMap[js_to_rule(rules_list[pda.rule]->u)->s];
                pda.label = c->label;
                pda.pred = false;
                pda.cb = false;
                pdas.push_back(pda);
                if (!adjusts.empty()) {
                    mapLabelsToPda[c->label] = i;
                }
            }
            for (size_t i = 0; i < closure.size(); ++i) {
                auto &p = pdas[i];
                auto outs = get_filter_out_edges(closure[i], [](auto it) { return true; });
                std::unordered_set<uint32_t> reduces;
                for (auto &o : outs) {
                    auto edge = (js_pda_edge *) o->edge;
                    js_pda_trans trans{};
                    trans.jump = pids[(js_pda_status *) edge->end];
                    trans.type = edge->type;
                    trans.marked = edge->marked;
                    trans.cb = edge->cb;
                    if (edge->cb && !p.cb)
                        p.cb = true;
                    trans.cost = 0;
                    auto v = prev.find(edge);
                    if (v != prev.end()) {
                        trans.status = pids[v->second];
                        trans.label = closure[trans.status]->label;
                    } else {
                        trans.status = -1;
                    }
                    if ((trans.type == e_reduce || trans.type == e_reduce_exp) && trans.status >= 0) {
                        auto h = 2 * (trans.jump * 2 * closure.size() + trans.status) + trans.type - e_reduce;
                        if (reduces.find(h) != reduces.end()) {
                            continue;
                        }
                        reduces.insert(h);
                    }
                    std::copy(std::begin(LA[edge]), std::end(LA[edge]), std::back_inserter(trans.LA));
                    p.trans.push_back(trans);
                }
                std::sort(p.trans.begin(), p.trans.end(), [](const auto &a, const auto &b) {
                    if (js_pda_edge_priority(a.type) < js_pda_edge_priority(b.type))
                        return true;
                    if (js_pda_edge_priority(a.type) > js_pda_edge_priority(b.type))
                        return false;
                    if (a.jump < b.jump)
                        return true;
                    if (a.jump > b.jump)
                        return false;
                    return a.status < b.status;
                });
            }
            for (const auto &adjust : adjusts) {
                if (adjust.ea == e_shift) {
                    auto r = get_closure(rules[js_to_rule(adjust.r)->s].status, [](auto it) { return true; });
                    auto &_r = pdas.at((size_t) mapLabelsToPda.at(r.front()->label));
                    auto a = get_closure(rules[js_to_rule(adjust.a)->s].status, [](auto it) { return true; });
                    const auto &_a = mapLabelsToPda.at(a.front()->label);
                    auto &t = _r.trans;
                    auto sa = std::find_if(
                            t.begin(), t.end(),
                            [_a](auto it) { return it.type == e_shift && it.jump == _a; });
                    if (sa != t.end()) {
                        if (adjust.cost != 0) {
                            sa->cost = adjust.cost;
                            std::sort(_r.trans.begin(), _r.trans.end(), [](const auto &a, const auto &b) {
                                if (a.cost > b.cost)
                                    return true;
                                if (a.cost < b.cost)
                                    return false;
                                if (js_pda_edge_priority(a.type) < js_pda_edge_priority(b.type))
                                    return true;
                                if (js_pda_edge_priority(a.type) > js_pda_edge_priority(b.type))
                                    return false;
                                if (a.jump < b.jump)
                                    return true;
                                if (a.jump > b.jump)
                                    return false;
                                return a.status < b.status;
                            });
                        }
                        if (adjust.pred) {
                            if (!_r.pred)_r.pred = true;
                            sa->pred = adjust.pred;
                        }
                    }
                } else if (adjust.ea == e_left_recursion) {
                    auto r = get_closure(rules[js_to_rule(adjust.r)->s].status, [](auto it) { return true; });
                    auto f = std::find_if(
                            r.begin(), r.end(),
                            [](auto it) { return it->final; });
                    if (f != r.end()) {
                        auto &_r = pdas.at((size_t) mapLabelsToPda.at((*f)->label));
                        auto &t = _r.trans;
                        auto sa = std::find_if(
                                t.begin(), t.end(),
                                [](auto it) { return it.type == e_left_recursion; });
                        if (sa != t.end()) {
                            if (adjust.cost != 0) {
                                sa->cost = adjust.cost;
                                std::sort(_r.trans.begin(), _r.trans.end(), [](const auto &a, const auto &b) {
                                    if (a.cost > b.cost)
                                        return true;
                                    if (a.cost < b.cost)
                                        return false;
                                    if (js_pda_edge_priority(a.type) < js_pda_edge_priority(b.type))
                                        return true;
                                    if (js_pda_edge_priority(a.type) > js_pda_edge_priority(b.type))
                                        return false;
                                    if (a.jump < b.jump)
                                        return true;
                                    if (a.jump > b.jump)
                                        return false;
                                    return a.status < b.status;
                                });
                            }
                            if (adjust.pred) {
                                if (!_r.pred)_r.pred = true;
                                sa->pred = adjust.pred;
                            }
                        }
                    }
                }
            }
        }
    }

    const std::vector<js_pda_rule> &cjsunit::get_pda() const {
        return pdas;
    }

    void cjsunit::adjust(js_unit *r, js_unit *a, js_pda_edge_t ea, int cost, void *pred) {
        adjusts.push_back({r, a, ea, cost, pred});
    }

    static void print(js_nga_status *node, std::ostream &os) {
        if (node == nullptr)
            return;
        auto nga_status_list = get_closure(node, [](auto it) { return true; });
        std::unordered_map<js_nga_status *, size_t> status_map;
        for (size_t i = 0; i < nga_status_list.size(); ++i) {
            status_map.insert(std::make_pair(nga_status_list[i], i));
        }
        for (auto status : nga_status_list) {
            os << "Status #" << status_map[status];
            if (status->final)
                os << " [FINAL]";
            os << " - " << status->label << std::endl;
            auto outs = get_filter_out_edges(status, [](auto it) { return true; });
            for (auto &out : outs) {
                os << "  To #" << status_map[out->edge->end] << ":  ";
                if (out->edge->data)
                    print(out->edge->data, nullptr, os);
                else
                    os << "EPSILON";
                os << std::endl;
            }
        }
    }

    static void dump_first_set(js_nga_rule *rule, std::ostream &os) {
        os << "-- Tokens: ";
        for (auto &t : rule->tokensList) {
            print(t, nullptr, os);
            os << " ";
        }
        os << std::endl;
        os << "-- First-set tokens: ";
        for (auto &t : rule->tokensFirstset) {
            print(t, nullptr, os);
            os << " ";
        }
        os << std::endl;
        os << "-- First-set rules: ";
        for (auto &r : rule->rulesFirstset) {
            os << js_to_rule(r)->s;
            os << " ";
        }
        os << std::endl;
    }

    static std::tuple<js_pda_edge_t, std::string, int> pda_edge_string[] = {
            std::make_tuple(e_shift, "shift", 2),
            std::make_tuple(e_pass, "pass", 10),
            std::make_tuple(e_rule, "rule", 1),
            std::make_tuple(e_move, "move", 1),
            std::make_tuple(e_left_recursion, "recursion", 3),
            std::make_tuple(e_left_recursion_not_greed, "recursion", 5),
            std::make_tuple(e_reduce, "reduce", 4),
            std::make_tuple(e_reduce_exp, "reduce", 4),
            std::make_tuple(e_finish, "finish", 0),
    };

    const std::string &js_pda_edge_str(js_pda_edge_t type) {
        assert(type >= e_shift && type <= e_finish);
        return std::get<1>(pda_edge_string[type]);
    }

    const int &js_pda_edge_priority(js_pda_edge_t type) {
        assert(type >= e_shift && type <= e_finish);
        return std::get<2>(pda_edge_string[type]);
    }

    void cjsunit::dump(std::ostream &os) {
        os << "==== RULE ====" << std::endl;
        for (auto &k : rules) {
            print(k.second.u, nullptr, os);
            os << std::endl;
        }
        os << "==== NGA  ====" << std::endl;
        for (auto &k : rules) {
            os << "** Rule: ";
            print(k.second.u, nullptr, os);
            os << std::endl;
            dump_first_set(&k.second, os);
            print(k.second.status, os);
            os << std::endl;
        }
        os << "==== PDA  ====" << std::endl;
        os << "** [Initial] State: " << pdas[0].label << std::endl;
        os << std::endl;
        for (auto &pda : pdas) {
            os << "**" << (pda.final ? " [FINAL]" : "") << " State #" << pda.id << ": " << pda.label << std::endl;
            for (auto &trans : pda.trans) {
                os << "    --> __________________" << std::endl;
                os << "    --> #" << trans.jump << ": " << pdas[trans.jump].label << std::endl;
                os << "    -->     Type: " << js_pda_edge_str(trans.type) << std::endl;
                if (trans.type == e_reduce) {
                    os << "    -->     Reduce: " << trans.label << std::endl;
                } else if (trans.type == e_reduce_exp) {
                    os << "    -->     Reduce(exp): " << trans.label << std::endl;
                }
                if (!trans.LA.empty()) {
                    for (size_t i = 0; i < trans.LA.size(); ++i) {
                        if (i % 5 == 0) {
                            os << "    -->     LA: ";
                        }
                        print(trans.LA[i], nullptr, os);
                        os << " ";
                        if (i % 5 == 4 || i == trans.LA.size() - 1) {
                            os << std::endl;
                        }
                    }
                }
            }
            os << std::endl;
        }
    }
}