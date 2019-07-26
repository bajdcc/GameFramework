//
// Project: clibparser
// Created by CC
//

#include "stdafx.h"
#include <algorithm>
#include <functional>
#include <queue>
#include <iostream>
#include <sstream>
#include "cunit.h"
#include "cexception.h"

#define SHOW_RULE 0
#define SHOW_LABEL 0
#define SHOW_CLOSURE 0
#define DETECT_LEFT_RECURSION 0

#define IS_SEQ(type) (type == u_sequence)
#define IS_BRANCH(type) (type == u_branch)
#define IS_COLLECTION(type) (type == u_sequence || type == u_branch || type == u_optional)

namespace clib {

    unit_rule* to_rule(unit* u) {
        assert(u->t == u_rule);
        return (unit_rule*)u;
    }

    unit_token* to_token(unit * u) {
        assert(u->t == u_token);
        return (unit_token*)u;
    }

    unit_collection* to_collection(unit * u) {
        assert(IS_COLLECTION(u->t));
        return (unit_collection*)u;
    }

    unit_collection* to_ref(unit * u) {
        assert(u->t == u_token_ref || u->t == u_rule_ref);
        return (unit_collection*)u;
    }

    unit& unit::operator=(const unit & u) {
        auto rule = to_rule(this);
        rule->child = builder->copy(const_cast<unit*>(&u));
        return *this;
    }

    unit& unit::operator+(const unit & u) {
        if ((!IS_COLLECTION(t) && !IS_COLLECTION(u.t)) ||
            (IS_BRANCH(t) && IS_BRANCH(u.t))) {
            return builder->collection(this, const_cast<unit*>(&u), u_sequence);
        }
        else if (IS_SEQ(t) && IS_SEQ(u.t)) {
            return builder->merge(this, const_cast<unit*>(&u));
        }
        else if (IS_SEQ(t)) {
            return builder->append(this, const_cast<unit*>(&u));
        }
        else if (IS_SEQ(u.t)) {
            return builder->append(const_cast<unit*>(&u), this);
        }
        else {
            return builder->collection(this, const_cast<unit*>(&u), u_sequence);
        }
    }

    unit& unit::operator|(const unit & u) {
        if ((!IS_COLLECTION(t) && !IS_COLLECTION(u.t)) ||
            (IS_SEQ(t) && IS_SEQ(u.t))) {
            return builder->collection(this, const_cast<unit*>(&u), u_branch);
        }
        else if (IS_BRANCH(t) && IS_BRANCH(u.t)) {
            return builder->merge(this, const_cast<unit*>(&u));
        }
        else if (IS_BRANCH(t)) {
            return builder->append(this, const_cast<unit*>(&u));
        }
        else if (IS_BRANCH(u.t)) {
            return builder->append(this, const_cast<unit*>(&u));
        }
        else {
            return builder->collection(this, const_cast<unit*>(&u), u_branch);
        }
    }

    unit& unit::operator*() {
        return builder->optional(this);
    }

    unit& unit::operator~() {
        if (t == u_token)
            return to_ref(builder->copy(this))->set_skip(true);
        if (t == u_token_ref)
            return to_ref(this)->set_marked(true);
        assert(!"invalid type");
        return *this;
    }

    unit& unit::init(unit_builder * builder) {
        next = prev = nullptr;
        this->builder = builder;
        return *this;
    }

    unit& unit::set_t(unit_t type) {
        this->t = type;
        return *this;
    }

    unit_token& unit_token::set_type(lexer_t type) {
        this->type = type;
        return *this;
    }

    unit_token& unit_token::set_op(operator_t op) {
        value.op = op;
        return *this;
    }

    unit_token& unit_token::set_keyword(keyword_t keyword) {
        value.keyword = keyword;
        return *this;
    }

    unit_collection& unit_collection::set_skip(bool skip) {
        this->skip = skip;
        return *this;
    }

    unit_collection& unit_collection::set_marked(bool marked) {
        this->marked = marked;
        return *this;
    }

    unit_collection& unit_collection::set_child(unit * node) {
        child = node;
        return *this;
    }

    unit_rule& unit_rule::set_s(const char* str) {
        s = str;
        return *this;
    }

    unit_rule& unit_rule::set_attr(uint32 attr) {
        this->attr = attr;
        return *this;
    }

    const char* cunit::str(const string_t & s) {
        auto f = strings.find(s);
        if (f == strings.end()) {
            return strings.insert(s).first->c_str();
        }
        return f->c_str();
    }

    unit& cunit::token(const lexer_t & type) {
        return (*nodes.alloc<unit_token>()).set_type(type).set_t(u_token).init(this);
    }

    unit& cunit::token(const operator_t & op) {
        return (*nodes.alloc<unit_token>()).set_type(l_operator).set_op(op).set_t(u_token).init(this);
    }

    unit& cunit::token(const keyword_t & keyword) {
        return (*nodes.alloc<unit_token>()).set_type(l_keyword).set_keyword(keyword).set_t(u_token).init(this);
    }

    unit* cunit::copy(unit * u) {
        if (u->t == u_token) { // copy token unit
            return &(*nodes.alloc<unit_collection>())
                .set_skip(false).set_marked(false).set_child(u)
                .set_t(u_token_ref).init(this);
        }
        if (u->t == u_rule) { // copy rule unit
            return &(*nodes.alloc<unit_collection>())
                .set_skip(false).set_marked(false).set_child(u)
                .set_t(u_rule_ref).init(this);
        }
        return u;
    }

    unit_collection& cunit::append(unit * collection, unit * child) {
        auto node = to_collection(collection);
        child = copy(child);
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
        return *node;
    }

    unit_collection& cunit::merge(unit * a, unit * b) {
        auto nodeA = to_collection(a)->child;
        auto nodeB = to_collection(b)->child;
        nodeA->prev->next = nodeB;
        nodeB->prev->next = nodeA;
        std::swap(nodeA->prev, nodeB->prev);
        nodes.free(b);
        return *to_collection(a);
    }

    unit_collection& cunit::collection(unit * a, unit * b, unit_t type) {
        a = copy(a);
        b = copy(b);
        a->next = a->prev = b;
        b->next = b->prev = a;
        return (unit_collection&)(*nodes.alloc<unit_collection>()).set_child(a).set_t(type).init(this);
    }

    unit_collection& cunit::optional(unit * a) {
        a = copy(a);
        a->next = a->prev = a;
        return (unit_collection&)(*nodes.alloc<unit_collection>()).set_child(a).set_t(u_optional).init(this);
    }

    unit& cunit::rule(const string_t & s, coll_t t, uint32 attr) {
        auto f = rules.find(s);
        if (f == rules.end()) {
            auto& rule = (*nodes.alloc<unit_rule>())
                .set_s(str(s))
                .set_attr(attr)
                .set_child(nullptr)
                .set_t(u_rule)
                .init(this);
            nga_rule r;
            r.id = rules.size();
            r.status = nullptr;
            r.u = to_rule(&rule);
            r.recursive = 0;
            rulesMap.insert(std::make_pair(r.u->s, t));
            rules.insert(std::make_pair(s, r));
            return *r.u;
        }
        return *f->second.u;
    }

    template<class T>
    static void unit_recursion(unit * node, unit * parent, std::ostream & os, T f) {
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

    void print(unit * node, unit * parent, std::ostream & os) {
        if (node == nullptr)
            return;
        auto rec = [&](auto n, auto p, auto & os) { print(n, p, os); };
        auto type = node->t;
        switch (type) {
        case u_none:
            break;
        case u_token: {
            auto token = to_token(node);
            if (token->type == l_keyword) {
                os << KEYWORD_STRING(token->value.keyword);
            }
            else if (token->type == l_operator) {
                os << "'" << OP_STRING(token->value.op) << "'";
            }
            else {
                os << "#" << LEX_STRING(token->type) << "#";
            }
        }
                      break;
        case u_token_ref: {
            auto t = to_ref(node);
            if (t->skip)
                os << "(";
            rec(t->child, node, os);
            if (t->skip)
                os << ")";
        }
                          break;
        case u_rule: {
            auto rule = to_rule(node);
            os << rule->s << " => ";
            rec(rule->child, node, os);
        }
                     break;
        case u_rule_ref: {
            os << to_rule(to_ref(node)->child)->s;
        }
                         break;
        case u_sequence:
        case u_branch:
        case u_optional: {
            auto co = to_collection(node);
            if (node->t == u_branch)
                os << "( ";
            else if (node->t == u_optional)
                os << "[ ";
            unit_recursion(co->child, node, os, rec);
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
                auto p = to_collection(parent);
                if (node->next != p->child)
                    os << (IS_SEQ(p->t) ? " " : " | ");
            }
        }
    }

    void cunit::gen(unit * root) {
        gen_nga();
        check_nga();
        gen_pda(root);
    }

    void cunit::gen_nga() {
        for (auto& rule : rules) {
            current_rule = to_rule(rule.second.u);
#if SHOW_RULE
            print(current_rule, nullptr, std::cout);
            std::cout << std::endl;
#endif
            assert(current_rule->child);
            rule.second.status = delete_epsilon(conv_nga(current_rule->child));
        }
        current_rule = nullptr;
    }

    template <class T>
    static std::vector<T*> get_children(T * node) {
        std::vector<T*> v;
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

    bool get_first_set(unit * node, nga_rule & rule) {
        if (node == nullptr)
            return true;
        switch (node->t) {
        case u_none:
            break;
        case u_token_ref: {
            rule.tokensFirstset.insert(to_token(to_ref(node)->child));
            return false;
        }
        case u_rule_ref: {
            rule.rulesFirstset.insert(to_rule(to_ref(node)->child));
            return false;
        }
        case u_sequence: {
            for (auto& u : get_children(to_collection(node)->child)) {
                if (!get_first_set(u, rule)) {
                    return false;
                }
            }
            return true;
        }
        case u_branch: {
            auto zero = false;
            for (auto& u : get_children(to_collection(node)->child)) {
                if (get_first_set(u, rule) && !zero) {
                    zero = true;
                }
            }
            return zero;
        }
        case u_optional: {
            for (auto& u : get_children(to_collection(node)->child)) {
                get_first_set(u, rule);
            }
            return true;
        }
        default:
            break;
        }
        return false;
    }

    void cunit::check_nga() {
        for (auto& rule : rules) {
            nga_rule& r = rule.second;
            if (get_first_set(to_rule(r.u)->child, r)) {
                error("generate epsilon: " + print_unit(r.u));
            }
        }
        auto size = rules.size();
        std::vector<nga_rule*> rules_list(size);
        std::vector<std::vector<bool>> dep(size);
        std::unordered_map<unit*, int> ids;
        for (auto& rule : rules) {
            rules_list[rule.second.id] = &rule.second;
            ids.insert(std::make_pair(rule.second.u, rule.second.id));
        }
        for (size_t i = 0; i < size; ++i) {
            dep[i].resize(size);
            for (auto& r : rules_list[i]->rulesFirstset) {
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
            if (size >= MAX_SIZE) error("exceed max dep size");
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
                        }
                    }
                }
                if (indep == -1) {
                    error("missing most independent rule");
                }
                for (auto& r : rules_list[indep]->rulesFirstset) {
                    auto& a = rules_list[indep]->tokensFirstset;
                    auto& b = rules_list[ids[r]]->tokensList;
                    a.insert(b.begin(), b.end());
                }
                {
                    auto& a = rules_list[indep]->tokensList;
                    auto& b = rules_list[indep]->tokensFirstset;
                    a.insert(b.begin(), b.end());
                    auto& c = rules_list[indep]->rulesFirstset;
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
    static void nga_recursion(unit * u, std::vector<nga_edge*> & v, T f) {
        auto node = to_collection(u)->child;
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

    nga_edge* cunit::conv_nga(unit * u) {
        if (u == nullptr)
            return nullptr;
        auto rec = [&](auto u) { return conv_nga(u); };
        switch (u->t) {
        case u_token_ref:
        case u_rule_ref: {
            auto enga = u->builder->enga(u, u);
            enga->skip = to_ref(u)->skip;
            enga->marked = to_ref(u)->marked;
            return enga;
        }
        case u_sequence: {
            auto enga = u->builder->enga(u, false);
            enga->data = u;
            std::vector<nga_edge*> edges;
            nga_recursion(u, edges, rec);
            for (auto& edge : edges) {
                if (enga->begin != nullptr) {
                    u->builder->connect(enga->end, edge->begin);
                    enga->end = edge->end;
                }
                else {
                    enga->begin = edge->begin;
                    enga->end = edge->end;
                }
            }
            return enga;
        }
        case u_branch: {
            auto enga = u->builder->enga(u, true);
            enga->data = u;
            std::vector<nga_edge*> edges;
            nga_recursion(u, edges, rec);
            for (auto& edge : edges) {
                u->builder->connect(enga->begin, edge->begin);
                u->builder->connect(edge->end, enga->end);
            }
            return enga;
        }
        case u_optional: {
            std::vector<nga_edge*> edges;
            nga_recursion(u, edges, rec);
            auto& edge = edges.front();
            return u->builder->connect(edge->begin, edge->end);
        }
        default:
            break;
        }
        assert(!"not supported");
        return nullptr;
    }

    nga_edge* cunit::enga(unit * node, bool init) {
        auto _enga = nodes.alloc<nga_edge>();
        _enga->data = nullptr;
        _enga->skip = false;
        _enga->marked = false;
        if (init) {
            _enga->begin = status();
            _enga->end = status();
            _enga->begin->label = label(node, true);
            _enga->end->label = label(node, false);
        }
        else {
            _enga->begin = _enga->end = nullptr;
        }
        return _enga;
    }

    nga_edge* cunit::enga(unit * node, unit * u) {
        auto begin = status();
        auto end = status();
        begin->label = label(node, true);
        end->label = label(node, false);
        auto _enga = connect(begin, end);
        _enga->data = u;
        return _enga;
    }

    nga_edge* cunit::connect(nga_status * a, nga_status * b, bool is_pda) {
        auto new_edge = is_pda ? nodes.alloc<pda_edge>() : nodes.alloc<nga_edge>();
        new_edge->begin = a;
        new_edge->end = b;
        new_edge->data = nullptr;
        add_edge(a->out, new_edge);
        add_edge(b->in, new_edge);
        return new_edge;
    }

    void cunit::disconnect(nga_status * status) {
        auto ins = get_children(status->in);
        for (auto& edge : ins) {
            remove_edge(edge->edge->end->in, edge);
            nodes.free(edge);
        }
        auto outs = get_children(status->out);
        for (auto& edge : outs) {
            remove_edge(edge->edge->end->in, edge);
            nodes.free(edge);
        }
        nodes.free(status);
    }

    nga_status* cunit::status() {
        auto _status = nodes.alloc<nga_status>();
        _status->final = false;
        _status->label = nullptr;
        _status->in = _status->out = nullptr;
        return _status;
    }

    pda_status* cunit::status(const char* label, int rule, bool final) {
        auto _status = nodes.alloc<pda_status>();
        _status->label = label;
        _status->final = final;
        _status->in = _status->out = nullptr;
        _status->rule = rule;
        return _status;
    }

    void cunit::add_edge(nga_edge_list * &list, nga_edge * edge) {
        if (list == nullptr) {
            list = nodes.alloc<nga_edge_list>();
            list->edge = edge;
            list->prev = list->next = list;
        }
        else {
            auto new_edge = nodes.alloc<nga_edge_list>();
            new_edge->edge = edge;
            new_edge->prev = list->prev;
            new_edge->next = list;
            list->prev->next = new_edge;
            list->prev = new_edge;
        }
    }

    void cunit::remove_edge(nga_edge_list * &list, nga_edge_list * edge) {
        if (list == nullptr) {
            error("remove from empty edge list");
        }
        else if (list->next == list) {
            assert(list->edge == edge->edge);
            list = nullptr;
            nodes.free(list);
        }
        else {
            if (list->edge == edge->edge) {
                list->prev->next = list->next;
                list->next->prev = list->prev;
                list = list->prev;
                nodes.free(edge);
            }
            else {
                auto node = list->next;
                while (node != list) {
                    if (node->edge == edge->edge) {
                        node->prev->next = node->next;
                        node->next->prev = node->prev;
                        nodes.free(node);
                        return;
                    }
                    node = node->next;
                }
                error("remove nothing from edge list");
            }
        }
    }

    const char* cunit::label(unit * focused, bool front) {
        std::stringstream ss;
        label(current_rule, nullptr, focused, front, ss);
#if SHOW_LABEL
        ATLTRACE("%s\n", ss.str().c_str());
#endif
        labels.emplace_back(ss.str());
        return labels.back().c_str();
    }

    template<class T>
    static void
        label_recursion(unit * node, unit * parent, unit * focused, bool front, std::ostream & os, T f) {
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

    void cunit::label(unit * node, unit * parent, unit * focused, bool front, std::ostream & os) {
        if (node == nullptr)
            return;
        auto rec = [this](auto _1, auto _2, auto _3, auto _4, auto & _5) {
            label(_1, _2, _3, _4, _5);
        };
        if (front && node == focused) {
            os << "@ ";
        }
        switch (node->t) {
        case u_none:
            break;
        case u_token: {
            auto token = to_token(node);
            if (token->type == l_keyword) {
                os << KEYWORD_STRING(token->value.keyword);
            }
            else if (token->type == l_operator) {
                os << "'" << OP_STRING(token->value.op) << "'";
            }
            else {
                os << "#" << LEX_STRING(token->type) << "#";
            }
        }
                      break;
        case u_token_ref: {
            auto t = to_ref(node);
            if (t->skip)
                os << "(";
            rec(to_ref(node)->child, node, focused, front, os);
            if (t->skip)
                os << ")";
        }
                          break;
        case u_rule: {
            auto _rule = to_rule(node);
            os << _rule->s << " => ";
            rec(_rule->child, node, focused, front, os);
        }
                     break;
        case u_rule_ref: {
            os << to_rule(to_ref(node)->child)->s;
        }
                         break;
        case u_sequence:
        case u_branch:
        case u_optional: {
            auto co = to_collection(node);
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
                auto p = to_collection(parent);
                if (node->next != p->child)
                    os << (IS_SEQ(p->t) ? " " : " | ");
            }
        }
    }

    template<class T>
    static bool has_filter_in_edges(nga_edge_list * node, const T & f) {
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
    static std::vector<nga_edge_list*> get_filter_out_edges(nga_status * status, const T & f) {
        std::vector<nga_edge_list*> v;
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
    std::vector<nga_status*> get_closure(nga_status * status, const T & f) {
        std::vector<nga_status*> v;
        std::queue<nga_status*> queue;
        std::unordered_set<nga_status*> set;
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

    string_t cunit::print_unit(unit * u) {
        std::stringstream ss;
        print(u, nullptr, ss);
        return ss.str();
    }

    void cunit::error(const string_t & str) {
        throw cexception(ex_unit, str);
    }

    nga_status* cunit::delete_epsilon(nga_edge * edge) {
        edge->end->final = true;
        auto nga_status_list = get_closure(edge->begin, [](auto it) { return true; });
#if SHOW_CLOSURE
        for (auto& c : nga_status_list) {
            ATLTRACE("%s\n", c->label);
        }
#endif
        // TODO: Complete Delete Epsilon
        std::vector<nga_status*> available_status;
        std::vector<const char*> available_labels;
        std::unordered_map<size_t, size_t> available_labels_map;
        available_labels.emplace_back(nga_status_list[0]->label);
        available_labels_map.insert(std::make_pair(std::hash<string_t>{}(string_t(nga_status_list[0]->label)), available_status.size()));
        available_status.push_back(nga_status_list[0]);
        for (auto _status = nga_status_list.begin() + 1; _status != nga_status_list.end(); _status++) {
            auto& status = *_status;
            if (has_filter_in_edges(status->in, [](auto it) { return it->data != nullptr; })
                && available_labels_map.find(std::hash<string_t>{}(status->label)) ==
                available_labels_map.end()) {
                available_labels.emplace_back(status->label);
                available_labels_map.insert(std::make_pair(std::hash<string_t>{}(status->label), available_status.size()));
                available_status.push_back(status);
            }
        }
        for (auto& status : available_status) {
            auto epsilon_closure = get_closure(status, [](auto it) { return it->data == nullptr; });
            for (auto& epsilon : epsilon_closure) {
                if (epsilon == status)
                    continue;
                if (epsilon->final)
                    status->final = true;
                auto out_edges = get_filter_out_edges(epsilon, [](auto it) { return it->data != nullptr; });
                for (auto& out_edge : out_edges) {
                    auto idx = available_labels_map.at(std::hash<string_t>{}(out_edge->edge->end->label));
                    auto e = connect(status, available_status[idx]);
                    e->data = out_edge->edge->data;
                    e->skip = out_edge->edge->skip;
                    e->marked = out_edge->edge->marked;
                }
            }
        }
        for (auto& status : nga_status_list) {
            auto out_edges = get_filter_out_edges(status, [](auto it) { return it->data == nullptr; });
            for (auto& out_edge : out_edges) {
                remove_edge(out_edge->edge->end->in, out_edge);
                remove_edge(status->out, out_edge);
            }
        }
        for (auto& status : nga_status_list) {
            if (std::find(available_status.begin(), available_status.end(), status) == available_status.end()) {
                disconnect(status);
            }
        }
        return edge->begin;
    }

    bool not_left_recursive_status(nga_status * status, unit * rule) {
        if (status->in)
            return true;
        return status->out&& has_filter_in_edges(status->out,
            [rule](auto it) { return it->data && !(it->data->t == u_rule_ref && to_ref(it->data)->child == rule); });
    }

    bool is_left_resursive_edge(nga_edge * edge, unit * rule) {
        return !edge->begin->in&& edge->data&& edge->data->t == u_rule_ref && to_ref(edge->data)->child == rule;
    }

    void cunit::gen_pda(unit * root) {
        auto size = rules.size();
        pda_status* init_status = nullptr;
        struct status_t {
            nga_status* nga;
            pda_status* pda;
        };
        std::vector<status_t> status_list;
        std::vector<nga_rule*> rules_list(size);
        std::vector<std::vector<nga_edge*>> adj(size);
        std::unordered_map<unit*, int> ids;
        std::unordered_map<pda_edge*, std::unordered_set<unit*>> LA;
        std::unordered_map<pda_edge*, pda_status*> prev;
        for (auto& rule : rules) {
            rules_list[rule.second.id] = &rule.second;
            ids.insert(std::make_pair(rule.second.u, rule.second.id));
        }
        for (auto& rule : rules) {
            nga_rule& r = rule.second;
            auto closure = get_closure(r.status, [](auto it) { return true; });
            for (auto& s : closure) {
                auto outs = get_filter_out_edges(s, [](auto it) { return it->data&& it->data->t == u_rule_ref; });
                for (auto& o : outs) {
                    adj[ids[to_ref(o->edge->data)->child]].push_back(o->edge);
                }
                status_list.push_back({ s, status(s->label, ids[r.u], s->final) });
            }
        }
        // GENERATE PDA STATUS
        for (size_t i = 0; i < status_list.size(); ++i) {
            auto& s = status_list[i];
            auto& nga_status = s.nga;
            auto& pda_status = s.pda;
            auto& ru = rules_list[pda_status->rule];
            auto& r = ru->u;
            if (not_left_recursive_status(nga_status, r)) {
                auto is_init = root == r;
                if (!nga_status->in && is_init) {
                    init_status = pda_status; // INITIAL STATE
                }
                auto outs = get_children(nga_status->out);
                std::unordered_set<unit*> token_set;
                for (auto& o : outs) {
                    auto node = o->edge->data;
                    if (node->t == u_rule_ref) {
                        if (!is_left_resursive_edge(o->edge, r)) {
                            auto rr = to_ref(o->edge->data)->child;
                            auto nga = rules_list[ids[rr]]->status;
                            if (not_left_recursive_status(nga, r)) {
                                // SHIFT
                                auto& edge = *(pda_edge*)connect(
                                    pda_status, std::find_if(
                                        status_list.begin(),
                                        status_list.end(),
                                        [nga](auto it) { return it.nga == nga; })->pda,
                                    true);
                                edge.data = node;
                                edge.type = e_shift;
                                decltype(token_set) res;
                                auto& fs = rules_list[ids[rr]]->tokensFirstset;
                                res.insert(fs.begin(), fs.end());
                                LA.insert(std::make_pair(&edge, res));
                            }
                        }
                    }
                    else if (node->t == u_token_ref) {
                        // MOVE
                        auto& edge = *(pda_edge*)connect(
                            pda_status, std::find_if(
                                status_list.begin(),
                                status_list.end(),
                                [o](auto it) { return it.nga == o->edge->end; })->pda,
                            true);
                        edge.data = node;
                        auto n = to_ref(node);
                        edge.type = n->skip ? e_pass : e_move;
                        edge.marked = n->marked;
                        token_set.insert(n->child);
                        decltype(token_set) res{ n->child };
                        LA.insert(std::make_pair(&edge, res));
                    }
                }
                // FINAL
                if (nga_status->final) {
                    for (auto& o : adj[pda_status->rule]) {
                        auto& edge = *(pda_edge*)connect(
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
                            for (auto& _o : _outs) {
                                auto _d = _o->edge->data;
                                if (_d->t == u_token_ref) {
                                    res.insert(to_ref(_d)->child);
                                }
                                else if (_d->t == u_rule_ref) {
                                    auto _r = rules_list[ids[to_ref(_d)->child]]->tokensFirstset;
                                    res.insert(_r.begin(), _r.end());
                                }
                                else {
                                    error("invalid edge type: " + print_unit(_d));
                                }
                            }
                            LA.insert(std::make_pair(&edge, res));
                        }
                        else {
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
                        auto& edge = *(pda_edge*)connect(pda_status, pda_status, true);
                        edge.type = e_finish;
                        LA.insert(std::make_pair(&edge, decltype(token_set)()));
                    }
                }
            }
        }
        // GENERATE PDA TABLE
        {
            auto closure = get_closure(init_status, [](auto it) { return true; });
            std::unordered_map<pda_status*, int> pids;
            for (auto& c : closure) {
                pids.insert(std::make_pair((pda_status*)c, pids.size()));
            }
            for (size_t i = 0; i < closure.size(); ++i) {
                auto c = (pda_status*)closure[i];
                pda_rule pda{};
                pda.id = i;
                pda.rule = c->rule;
                pda.final = c->final;
                pda.coll = rulesMap[to_rule(rules_list[pda.rule]->u)->s];
                pda.label = c->label;
                pdas.push_back(pda);
            }
            for (size_t i = 0; i < closure.size(); ++i) {
                auto& p = pdas[i];
                auto outs = get_filter_out_edges(closure[i], [](auto it) { return true; });
                for (auto& o : outs) {
                    auto edge = (pda_edge*)o->edge;
                    pda_trans trans{};
                    trans.jump = pids[(pda_status*)edge->end];
                    trans.type = edge->type;
                    trans.marked = edge->marked;
                    auto v = prev.find(edge);
                    if (v != prev.end()) {
                        trans.status = pids[v->second];
                        trans.label = closure[trans.status]->label;
                    }
                    else {
                        trans.status = -1;
                    }
                    std::copy(std::begin(LA[edge]), std::end(LA[edge]), std::back_inserter(trans.LA));
                    p.trans.push_back(trans);
                }
            }
        }
    }

    const std::vector<pda_rule>& cunit::get_pda() const {
        return pdas;
    }

    void print(nga_status * node, std::ostream & os) {
        if (node == nullptr)
            return;
        auto nga_status_list = get_closure(node, [](auto it) { return true; });
        std::unordered_map<nga_status*, size_t> status_map;
        for (size_t i = 0; i < nga_status_list.size(); ++i) {
            status_map.insert(std::make_pair(nga_status_list[i], i));
        }
        for (auto status : nga_status_list) {
            os << "Status #" << status_map[status];
            if (status->final)
                os << " [FINAL]";
            os << " - " << status->label << std::endl;
            auto outs = get_filter_out_edges(status, [](auto it) { return true; });
            for (auto& out : outs) {
                os << "  To #" << status_map[out->edge->end] << ":  ";
                if (out->edge->data)
                    print(out->edge->data, nullptr, os);
                else
                    os << "EPSILON";
                os << std::endl;
            }
        }
    }

    void dump_first_set(nga_rule * rule, std::ostream & os) {
        os << "-- Tokens: ";
        for (auto& t : rule->tokensList) {
            print(t, nullptr, os);
            os << " ";
        }
        os << std::endl;
        os << "-- First-set tokens: ";
        for (auto& t : rule->tokensFirstset) {
            print(t, nullptr, os);
            os << " ";
        }
        os << std::endl;
        os << "-- First-set rules: ";
        for (auto& r : rule->rulesFirstset) {
            os << to_rule(r)->s;
            os << " ";
        }
        os << std::endl;
    }

    std::tuple<pda_edge_t, string_t, int> pda_edge_string[] = {
        std::make_tuple(e_shift, "shift", 2),
        std::make_tuple(e_pass, "pass", 10),
        std::make_tuple(e_move, "move", 1),
        std::make_tuple(e_left_recursion, "recursion", 3),
        std::make_tuple(e_left_recursion_not_greed, "recursion", 5),
        std::make_tuple(e_reduce, "reduce", 4),
        std::make_tuple(e_reduce_exp, "reduce", 4),
        std::make_tuple(e_finish, "finish", 0),
    };

    const string_t& pda_edge_str(pda_edge_t type) {
        assert(type >= e_shift && type <= e_finish);
        return std::get<1>(pda_edge_string[type]);
    }

    const int& pda_edge_priority(pda_edge_t type) {
        assert(type >= e_shift && type <= e_finish);
        return std::get<2>(pda_edge_string[type]);
    }

    void cunit::dump(std::ostream & os) {
        os << "==== RULE ====" << std::endl;
        for (auto& k : rules) {
            print(k.second.u, nullptr, os);
            os << std::endl;
        }
        os << "==== NGA  ====" << std::endl;
        for (auto& k : rules) {
            os << "** Rule: ";
            print(k.second.u, nullptr, os);
            std::cout << std::endl;
            dump_first_set(&k.second, os);
            print(k.second.status, os);
            os << std::endl;
        }
        os << "==== PDA  ====" << std::endl;
        os << "** [Initial] State: " << pdas[0].label << std::endl;
        os << std::endl;
        for (auto& pda : pdas) {
            os << "**" << (pda.final ? " [FINAL]" : "") << " State #" << pda.id << ": " << pda.label << std::endl;
            for (auto& trans : pda.trans) {
                os << "    --> __________________" << std::endl;
                os << "    --> #" << trans.jump << ": " << pdas[trans.jump].label << std::endl;
                os << "    -->     Type: " << pda_edge_str(trans.type) << std::endl;
                if (trans.type == e_reduce) {
                    os << "    -->     Reduce: " << trans.label << std::endl;
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
};