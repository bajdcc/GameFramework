//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <cassert>
#include "cjsgen.h"
#include "cjsast.h"
#include "cjsruntime.h"

#define DEBUG_MODE 0
#define PRINT_CODE 1
#define DUMP_CODE 1
#define PRINT_AST 0

#define AST_IS_KEYWORD(node) ((node)->flag == a_keyword)
#define AST_IS_KEYWORD_K(node, k) ((node)->data._keyword == (k))
#define AST_IS_KEYWORD_N(node, k) (AST_IS_KEYWORD(node) && AST_IS_KEYWORD_K(node, k))
#define AST_IS_OP(node) ((node)->flag == a_operator)
#define AST_IS_OP_K(node, k) ((node)->data._op == (k))
#define AST_IS_OP_N(node, k) (AST_IS_OP(node) && AST_IS_OP_K(node, k))
#define AST_IS_ID(node) ((node)->flag == a_literal)
#define AST_IS_COLL(node) ((node)->flag == a_collection)
#define AST_IS_COLL_K(node, k) ((node)->data._coll == (k))
#define AST_IS_COLL_N(node, k) (AST_IS_COLL(node) && AST_IS_COLL_K(node, k))

namespace clib {

    cjsgen::cjsgen() {
        tmp.emplace_back();
        ast.emplace_back();
        codes.push_back(std::make_shared<js_sym_code_t>());
    }

    void copy_info(js_sym_t::ref dst, js_ast_node *src) {
        dst->line = src->line;
        dst->column = src->column;
        dst->start = src->start;
        dst->end = src->end;
    }

    void copy_info(js_sym_t::ref dst, js_sym_t::ref src) {
        dst->line = src->line;
        dst->column = src->column;
        dst->start = src->start;
        dst->end = src->end;
    }

    int cjs_consts::get_number(double n) {
        auto f = numbers.find(n);
        if (f == numbers.end()) {
            auto idx = index++;
            numbers.insert({n, idx});
            return idx;
        }
        return f->second;
    }

    int cjs_consts::get_string(const std::string &str, get_string_t type) {
        if (type == gs_name) {
            auto f = names.find(str);
            if (f == names.end()) {
                auto idx = (int) names.size();
                names.insert({str, idx});
                return idx;
            }
            return f->second;
        }
        if (type == gs_string) {
            std::string s;
            if (str.front() == str.back() && (str.front() == '"' || str.front() == '\'')) {
                s = std::string(str.c_str() + 1);
                s.pop_back();
            } else {
                s = str;
            }
            auto f = strings.find(s);
            if (f == strings.end()) {
                auto idx = index++;
                strings.insert({s, idx});
                return idx;
            }
            return f->second;
        }
        if (type == gs_global) {
            auto f = globals.find(str);
            if (f == globals.end()) {
                auto idx = (int) globals.size();
                globals.insert({str, idx});
                return idx;
            }
            return f->second;
        }
        if (type == gs_deref) {
            auto f = derefs.find(str);
            if (f == derefs.end()) {
                auto idx = (int) derefs.size();
                derefs.insert({str, idx});
                return idx;
            }
            return f->second;
        }
        if (type == gs_regex) {
            auto f = regexes.find(str);
            if (f == regexes.end()) {
                auto idx = index++;
                regexes.insert({str, idx});
                return idx;
            }
            return f->second;
        }
        assert(!"invalid type");
        return -1;
    }

    int cjs_consts::get_function(std::shared_ptr<js_sym_code_t> code) {
        auto idx = index++;
        functions.insert({idx, code});
        return idx;
    }

    std::string cjs_consts::get_desc(int n) const {
        auto f = functions.find(n);
        if (f != functions.end()) {
            auto func = f->second.lock();
            return func->name ? func->name->data._identifier : LAMBDA_ID;
        }
        for (const auto &x : strings) {
            if (x.second == n)
                return x.first;
        }
        for (const auto &x : numbers) {
            if (x.second == n) {
                std::stringstream ss;
                ss << x.first;
                return ss.str();
            }
        }
        for (const auto &x : regexes) {
            if (x.second == n)
                return x.first;
        }
        return "";
    }

    js_runtime_t cjs_consts::get_type(int n) const {
        return consts.at(n);
    }

    char *cjs_consts::get_data(int n) const {
        return consts_data.at(n);
    }

    const char *cjs_consts::get_name(int n) const {
        return names_data.at(n);
    }

    const char *cjs_consts::get_global(int n) const {
        return globals_data.at(n);
    }

    void cjs_consts::dump(const std::string *text) const {
        auto i = 0;
        for (const auto &x : names_data) {
            fprintf(stdout, "C [#%03d] [NAME  ] %s\n", i++, x);
        }
        i = 0;
        for (const auto &x : globals_data) {
            fprintf(stdout, "C [#%03d] [GLOBAL] %s\n", i++, x);
        }
        i = 0;
        for (const auto &x : derefs_data) {
            fprintf(stdout, "C [#%03d] [DEREF ] %s\n", i++, x);
        }
        i = 0;
        for (const auto &x : consts_data) {
            switch (consts[i]) {
                case r_string:
                    fprintf(stdout, "C [#%03d] [STRING] %s\n", i, ((std::string *) x)->c_str());
                    break;
                case r_number:
                    fprintf(stdout, "C [#%03d] [NUMBER] %s\n", i, jsv_number::number_to_string(*(double *) x).c_str());
                    break;
                case r_function: {
                    auto f = functions.at(i).lock();
                    fprintf(stdout, "C [#%03d] [FUNC  ] %s | %s\n", i, f->debugName.c_str(),
                            text->substr(f->start, f->end - f->start).c_str());
                }
                    break;
                default:
                    break;
            }
            i++;
        }
    }

    void cjs_consts::save() {
        consts.resize(index);
        std::fill(consts.begin(), consts.end(), r__end);
        consts_data.resize(index);
        std::fill(consts_data.begin(), consts_data.end(), nullptr);
        names_data.resize(names.size());
        std::fill(names_data.begin(), names_data.end(), nullptr);
        globals_data.resize(globals.size());
        std::fill(globals_data.begin(), globals_data.end(), nullptr);
        derefs_data.resize(derefs.size());
        std::fill(derefs_data.begin(), derefs_data.end(), nullptr);
        for (const auto &x : strings) {
            consts[x.second] = r_string;
            consts_data[x.second] = (char *) &x.first;
        }
        for (const auto &x : numbers) {
            consts[x.second] = r_number;
            consts_data[x.second] = (char *) &x.first;
        }
        for (const auto &x : functions) {
            consts[x.first] = r_function;
            consts_data[x.first] = (char *) &x.second;
        }
        for (const auto &x : regexes) {
            consts[x.second] = r_regex;
            consts_data[x.second] = (char *) &x.first;
        }
        for (const auto &x : names) {
            names_data[x.second] = x.first.c_str();
        }
        for (const auto &x : globals) {
            globals_data[x.second] = x.first.c_str();
        }
        for (const auto &x : derefs) {
            derefs_data[x.second] = x.first.c_str();
        }
    }

    const std::vector<char *> &cjs_consts::get_consts_data() const {
        return consts_data;
    }

    const std::vector<const char *> &cjs_consts::get_names_data() const {
        return names_data;
    }

    const std::vector<const char *> &cjs_consts::get_globals_data() const {
        return globals_data;
    }

    const std::vector<const char *> &cjs_consts::get_derefs_data() const {
        return derefs_data;
    }

    bool cjsgen::gen_code(js_ast_node *node, const std::string *str, const std::string &name) {
        filename = name;
        text = str;
        gen_rec(node, 0);
        if (tmp.front().empty())
            return false;
        tmp.front().front()->set_parent(nullptr);
        tmp.front().front()->gen_rvalue(*this);
#if PRINT_AST && DEBUG_MODE
        print(tmp.front().front(), 0, std::cout);
#endif
        decltype(codes) _codes(1 + funcs.size());
        _codes[0] = codes.front();
        std::copy(funcs.begin(), funcs.end(), _codes.begin() + 1);
        for (auto &c : _codes) {
            c->consts.save();
            c->text = text->substr(c->start, c->end - c->start);
        }
#if DUMP_CODE && DEBUG_MODE
        dump();
#endif
        return true;
    }

    cjs_code_result::ref cjsgen::get_code() const {
        if (codes.empty())
            return nullptr;
        auto result = std::make_unique<cjs_code_result>();
        result->code = codes.front();
        result->funcs = funcs;
        return std::move(result);
    }

    template<class T>
    static void gen_recursion(js_ast_node *node, int level, T f) {
        if (node == nullptr)
            return;
        auto i = node;
        if (i->next == i) {
            f(i, level);
            return;
        }
        f(i, level);
        i = i->next;
        while (i != node) {
            f(i, level);
            i = i->next;
        }
    }

    template<class T>
    static std::vector<T *> gen_get_children(T *node) {
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

    template<class T>
    static std::vector<T *> gen_get_children_reverse(T *node) {
        std::vector<T *> v;
        if (node == nullptr)
            return v;
        node = node->prev;
        auto i = node;
        if (i->next == i) {
            v.push_back(i);
            return v;
        }
        v.push_back(i);
        i = i->prev;
        while (i != node) {
            v.push_back(i);
            i = i->prev;
        }
        return v;
    }

    void cjsgen::gen_rec(js_ast_node *node, int level) {
        if (node == nullptr)
            return;
        auto rec = [this](auto n, auto l) { this->gen_rec(n, l); };
        auto type = (js_ast_t) node->flag;
        if (type == a_collection) {
            if ((node->attr & a_exp) && node->child == node->child->next) {
                gen_recursion(node->child, level, rec);
                return;
            }
        }
        tmp.emplace_back();
        ast.emplace_back();
        switch (type) {
            case a_root: {
                gen_recursion(node->child, level, rec);
            }
                break;
            case a_collection: {
                auto children = (node->attr & ((uint16_t) a_reverse)) ?
                                gen_get_children_reverse(node->child) :
                                gen_get_children(node->child);
                gen_coll(children, level + 1, node);
            }
                break;
            case a_keyword:
            case a_operator:
            case a_literal:
            case a_string:
            case a_number:
            case a_regex:
                ast.back().push_back(node);
                break;
            default:
                break;
        }
        auto &tmps = tmp.back();
        if (!tmps.empty()) {
            //assert(tmps.size() == 1);
            auto &top = tmp[tmp.size() - 2];
            for (auto &t : tmps) {
                top.push_back(t);
            }
        }
        tmp.pop_back();
        if (!ast.back().empty()) {
            auto &top = ast[ast.size() - 2];
            std::copy(ast.back().begin(), ast.back().end(), std::back_inserter(top));
        }
        ast.pop_back();
    }

    void cjsgen::gen_coll(const std::vector<js_ast_node *> &nodes, int level, js_ast_node *node) {
        if (!gen_before(nodes, level, node)) {
            return;
        }
        for (auto &n : nodes) {
            gen_rec(n, level);
        }
        gen_after(nodes, level, node);
    }

    bool cjsgen::gen_before(const std::vector<js_ast_node *> &nodes, int level, js_ast_node *node) {
        switch (node->data._coll) {
            case c_prefixExpression: {
                if (AST_IS_COLL_K(nodes.front(), c_prefixExpressionList)) {
                    gen_rec(nodes[1], level); // gen exp first
                    nodes[0]->attr |= (uint16_t) a_reverse;
                    gen_rec(nodes[0], level); // then prefix
                    nodes[0]->attr &= (uint16_t) ~((uint16_t) a_reverse);
                    gen_after(nodes, level, node);
                    return false;
                }
            }
                break;
            case c_tryStatement: {
                auto stmt = std::make_shared<js_sym_stmt_try_t>();
                tmp.back().push_back(stmt);
            }
            default:
                break;
        }
        return true;
    }

    void cjsgen::gen_after(const std::vector<js_ast_node *> &nodes, int level, js_ast_node *node) {
        auto &asts = ast.back();
        auto &tmps = tmp.back();
        switch (node->data._coll) {
            case c_block: {
                auto block = std::make_shared<js_sym_block_t>();
                copy_info(block, asts.front());
                block->end = asts.back()->end;
                for (const auto &s : tmps) {
                    block->stmts.push_back(to_stmt(s));
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(block);
            }
                break;
            case c_variableDeclarationList: {
                auto stmt = std::make_shared<js_sym_stmt_var_t>();
                copy_info(stmt, tmps.front());
                for (const auto &s : tmps) {
                    assert(s->get_type() == s_id);
                    stmt->vars.push_back(std::dynamic_pointer_cast<js_sym_id_t>(s));
                    stmt->end = s->end;
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(stmt);
            }
                break;
            case c_variableDeclaration: {
                auto r = std::make_shared<js_sym_var_t>(asts.front());
                copy_info(r, asts.front());
                auto id = std::make_shared<js_sym_id_t>();
                id->ids.push_back(r);
                copy_info(id, r);
                if (!tmps.empty()) {
                    id->init = to_exp(tmps.front());
                    copy_info(id->init, tmps.front());
                    id->end = id->init->end;
                    id->parse();
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(id);
            }
                break;
            case c_emptyStatement: {
                auto empty = std::make_shared<js_sym_stmt_t>();
                copy_info(empty, asts.front());
                asts.clear();
                tmps.push_back(empty);
            }
                break;
            case c_expressionStatement: {
                if (tmps.front()->get_type() == s_expression_seq) {
                    auto stmt = std::make_shared<js_sym_stmt_exp_t>();
                    copy_info(stmt, tmps.front());
                    stmt->seq = std::dynamic_pointer_cast<js_sym_exp_seq_t>(tmps.front());
                    asts.clear();
                    tmps.clear();
                    tmps.push_back(stmt);
                } else if (tmps.front()->get_base_type() == s_expression) {
                    auto seq = std::make_shared<js_sym_exp_seq_t>();
                    copy_info(seq, tmps.front());
                    seq->exps.push_back(to_exp(tmps.front()));
                    auto stmt = std::make_shared<js_sym_stmt_exp_t>();
                    copy_info(stmt, tmps.front());
                    stmt->seq = seq;
                    asts.clear();
                    tmps.clear();
                    tmps.push_back(stmt);
                }
            }
                break;
            case c_ifStatement: {
                auto _if = std::make_shared<js_sym_stmt_if_t>();
                copy_info(_if, asts.front());
                _if->end = tmps.back()->end;
                assert(tmps.front()->get_base_type() == s_expression);
                if (tmps.front()->get_type() == s_expression_seq) {
                    _if->seq = std::dynamic_pointer_cast<js_sym_exp_seq_t>(tmps.front());
                } else {
                    _if->seq = std::make_shared<js_sym_exp_seq_t>();
                    copy_info(_if->seq, tmps.front());
                    _if->seq->exps.push_back(to_exp(tmps.front()));
                }
                if (tmps.size() < 3) {
                    _if->true_stmt = to_stmt(tmps.back());
                } else {
                    _if->true_stmt = to_stmt(tmps[1]);
                    _if->false_stmt = to_stmt(tmps[2]);
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(_if);
            }
                break;
            case c_doStatement: {
                auto _while = std::make_shared<js_sym_stmt_while_t>();
                _while->do_while = true;
                copy_info(_while, asts.front());
                _while->end = tmps.back()->end;
                assert(tmps.back()->get_base_type() == s_expression);
                if (tmps.back()->get_type() == s_expression_seq) {
                    _while->seq = std::dynamic_pointer_cast<js_sym_exp_seq_t>(tmps.back());
                } else {
                    _while->seq = std::make_shared<js_sym_exp_seq_t>();
                    copy_info(_while->seq, tmps.back());
                    _while->seq->exps.push_back(to_exp(tmps.back()));
                }
                _while->stmt = to_stmt(tmps.front());
                asts.clear();
                tmps.clear();
                tmps.push_back(_while);
            }
                break;
            case c_whileStatement: {
                auto _while = std::make_shared<js_sym_stmt_while_t>();
                copy_info(_while, asts.front());
                _while->end = tmps.back()->end;
                assert(tmps.front()->get_base_type() == s_expression);
                if (tmps.front()->get_type() == s_expression_seq) {
                    _while->seq = std::dynamic_pointer_cast<js_sym_exp_seq_t>(tmps.front());
                } else {
                    _while->seq = std::make_shared<js_sym_exp_seq_t>();
                    copy_info(_while->seq, tmps.front());
                    _while->seq->exps.push_back(to_exp(tmps.front()));
                }
                _while->stmt = to_stmt(tmps.back());
                asts.clear();
                tmps.clear();
                tmps.push_back(_while);
            }
                break;
            case c_forStatement: {
                auto _for = std::make_shared<js_sym_stmt_for_t>();
                copy_info(_for, asts.front());
                _for->end = tmps.back()->end;
                auto semi1 = AST_IS_KEYWORD_K(asts[1], K_VAR) ? asts[2] : asts[1];
                auto semi2 = AST_IS_KEYWORD_K(asts[1], K_VAR) ? asts[3] : asts[2];
                for (const auto &t : tmps) {
                    if (t->start < semi1->start) {
                        if (AST_IS_KEYWORD_K(asts[1], K_VAR)) {
                            assert(t->get_type() == s_statement_var);
                            _for->vars = std::dynamic_pointer_cast<js_sym_stmt_var_t>(t);
                            _for->vars->start = asts[1]->start;
                        } else {
                            _for->exp = to_exp(t);
                        }
                    } else if (t->start < semi2->start) {
                        _for->cond = to_exp(t);
                    } else if (t->get_base_type() == s_statement) {
                        _for->body = to_stmt(t);
                    } else {
                        _for->iter = to_exp(t);
                    }
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(_for);
            }
                break;
            case c_forInStatement: {
                auto _for_in = std::make_shared<js_sym_stmt_for_in_t>();
                copy_info(_for_in, asts.front());
                _for_in->end = tmps.back()->end;
                assert(tmps.size() >= 3);
                if (tmps[0]->get_base_type() == s_expression) {
                    _for_in->exp = to_exp(tmps[0]);
                } else {
                    assert(tmps[0]->get_type() == s_statement_var);
                    auto var = std::dynamic_pointer_cast<js_sym_stmt_var_t>(tmps[0]);
                    assert(var->vars.size() == 1);
                    _for_in->vars = var->vars.front();
                    _for_in->vars->start = asts[1]->start;
                }
                assert(tmps[1]->get_base_type() == s_expression);
                _for_in->iter = to_exp(tmps[1]);
                _for_in->body = to_stmt(tmps[2]);
                asts.clear();
                tmps.clear();
                tmps.push_back(_for_in);
            }
                break;
            case c_continueStatement:
            case c_breakStatement: {
                auto _ctrl = std::make_shared<js_sym_stmt_control_t>();
                _ctrl->keyword = asts[0]->data._keyword;
                copy_info(_ctrl, asts.front());
                if (!tmps.empty()) {
                    _ctrl->end = tmps.back()->end;
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(_ctrl);
            }
                break;
            case c_returnStatement: {
                auto stmt = std::make_shared<js_sym_stmt_return_t>();
                copy_info(stmt, asts.front());
                if (!tmps.empty()) {
                    js_sym_exp_seq_t::ref seq;
                    if (tmps.back()->get_type() == s_expression_seq) {
                        seq = std::dynamic_pointer_cast<js_sym_exp_seq_t>(tmps.back());
                    } else {
                        seq = std::make_shared<js_sym_exp_seq_t>();
                        copy_info(seq, tmps.front());
                        seq->exps.push_back(to_exp(tmps.front()));
                    }
                    stmt->seq = seq;
                    stmt->end = seq->end;
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(stmt);
            }
                break;
            case c_switchStatement: {
                auto stmt = std::make_shared<js_sym_stmt_switch_t>();
                copy_info(stmt, asts.front());
                stmt->end = asts.back()->end;
                stmt->exp = to_exp(tmps.front());
                std::transform(tmps.begin() + 1, tmps.end(),
                               std::back_inserter(stmt->cases),
                               [](const auto &s) {
                                   assert(s->get_type() == s_case);
                                   return std::dynamic_pointer_cast<js_sym_case_t>(s);
                               });
                std::unordered_map<std::string, js_ast_node_index *> cond;
                for (const auto &s : stmt->cases) {
                    auto str = s->exp ? get_code_text(s->exp.get()) : "default";
                    auto f = cond.find(str);
                    if (f != cond.end()) {
                        error(f->second, "conflict case: " + str);
                    }
                    cond.insert({str, s.get()});
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(stmt);
            }
                break;
            case c_caseClause: {
                auto exp = std::make_shared<js_sym_case_t>();
                copy_info(exp, asts.front());
                exp->end = tmps.size() > 1 ? tmps.back()->end : asts.back()->end;
                exp->exp = to_exp(tmps.front());
                std::transform(tmps.begin() + 1, tmps.end(),
                               std::back_inserter(exp->stmts),
                               [](const auto &s) {
                                   assert(s->get_base_type() == s_statement);
                                   return std::dynamic_pointer_cast<js_sym_stmt_t>(s);
                               });
                asts.clear();
                tmps.clear();
                tmps.push_back(exp);
            }
                break;
            case c_defaultClause: {
                auto exp = std::make_shared<js_sym_case_t>();
                copy_info(exp, asts.front());
                exp->end = !tmps.empty() ? tmps.back()->end : asts.back()->end;
                std::transform(tmps.begin(), tmps.end(),
                               std::back_inserter(exp->stmts),
                               [](const auto &s) {
                                   assert(s->get_base_type() == s_statement);
                                   return std::dynamic_pointer_cast<js_sym_stmt_t>(s);
                               });
                asts.clear();
                tmps.clear();
                tmps.push_back(exp);
            }
                break;
            case c_throwStatement: {
                auto stmt = std::make_shared<js_sym_stmt_throw_t>();
                copy_info(stmt, asts.front());
                if (!tmps.empty()) {
                    js_sym_exp_seq_t::ref seq;
                    if (tmps.back()->get_type() == s_expression_seq) {
                        seq = std::dynamic_pointer_cast<js_sym_exp_seq_t>(tmps.back());
                    } else {
                        seq = std::make_shared<js_sym_exp_seq_t>();
                        copy_info(seq, tmps.front());
                        seq->exps.push_back(to_exp(tmps.front()));
                    }
                    stmt->seq = seq;
                    stmt->end = seq->end;
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(stmt);
            }
                break;
            case c_tryStatement: {
                assert(tmps.front()->get_type() == s_statement_try);
                auto stmt = std::dynamic_pointer_cast<js_sym_stmt_try_t>(tmps.front());
                copy_info(stmt, asts.front());
                stmt->try_body = to_stmt(tmps.back());
                if (stmt->finally_body)
                    stmt->end = stmt->finally_body->end;
                else
                    stmt->end = stmt->catch_body->end;
                asts.clear();
                tmps.pop_back();
            }
                break;
            case c_catchProduction: {
                assert((*(tmp.rbegin() + 1)).front()->get_type() == s_statement_try);
                auto stmt = std::dynamic_pointer_cast<js_sym_stmt_try_t>((*(tmp.rbegin() + 1)).front());
                if (!asts.empty()) {
                    stmt->var = primary_node(asts.front());
                    copy_info(stmt->var, asts.front());
                    asts.clear();
                }
                stmt->catch_body = to_stmt(tmps.front());
                tmps.clear();
            }
                break;
            case c_finallyProduction: {
                assert((*(tmp.rbegin() + 1)).front()->get_type() == s_statement_try);
                auto stmt = std::dynamic_pointer_cast<js_sym_stmt_try_t>((*(tmp.rbegin() + 1)).front());
                stmt->finally_body = to_stmt(tmps.front());
                tmps.clear();
            }
                break;
            case c_functionStatement: {
                auto stmt = std::make_shared<js_sym_stmt_exp_t>();
                auto seq = std::make_shared<js_sym_exp_seq_t>();
                stmt->seq = seq;
                seq->exps.push_back(to_exp(tmps.front()));
                copy_info(seq, tmps.front());
                copy_info(stmt, tmps.front());
                tmps.clear();
                tmps.push_back(stmt);
            }
                break;
            case c_functionDeclaration: {
                auto code = std::make_shared<js_sym_code_t>();
                copy_info(code, asts[0]);
                code->name = asts[1];
                code->end = asts.back()->end;
                asts.pop_back();
                if (asts.size() >= 2 && (AST_IS_OP_N(*(asts.rbegin() + 1), T_ELLIPSIS))) {
                    asts.erase(asts.begin() + (asts.size() - 2));
                    code->rest = true;
                }
                decltype(code->args) _asts;
                std::transform(asts.begin() + 2, asts.end(),
                               std::back_inserter(_asts),
                               [this](const auto &x) { return this->primary_node(x); });
                std::transform(asts.begin() + 2, asts.end(),
                               std::back_inserter(code->args_str),
                               [](const auto &x) { return x->data._identifier; });
                std::unordered_set<std::string> arg_set;
                for (const auto &s : _asts) {
                    if (arg_set.find(s->node->data._identifier) != arg_set.end()) {
                        error(s, "conflict arg");
                    }
                    arg_set.insert(s->node->data._identifier);
                }
                code->args = std::move(_asts);
                if (!tmps.empty()) {
                    code->body = tmps.front();
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(code);
            }
                break;
            case c_sourceElements: {
                auto block = std::make_shared<js_sym_block_t>();
                if (!tmps.empty()) {
                    copy_info(block, tmps.front());
                }
                for (const auto &s : tmps) {
                    block->stmts.push_back(to_stmt(s));
                    block->end = s->end;
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(block);
            }
                break;
            case c_arrayLiteral: {
                auto array = std::make_shared<js_sym_array_t>();
                copy_info(array, asts.front());
                array->end = asts.back()->end;
                if (AST_IS_COLL_K(nodes[1], c_elementList)) {
                    auto i = 0;
                    auto head = true;
                    auto n = gen_get_children(nodes[1]->child);
                    for (const auto &s : n) {
                        if (AST_IS_COLL_K(s, c_commaList)) {
                            auto k = gen_get_children(s->child);
                            if (!head)
                                k.pop_back();
                            for (size_t j = 0; j < k.size(); j++) {
                                array->exps.push_back(nullptr);
                            }
                        } else {
                            if (AST_IS_OP_N(s->child, T_ELLIPSIS))
                                array->rests.push_back(i);
                            assert(tmps[i]->get_base_type() == s_expression);
                            array->exps.push_back(to_exp(tmps[i++]));
                        }
                        head = false;
                    }
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(array);
            }
                break;
            case c_objectLiteral: {
                auto obj = std::make_shared<js_sym_object_t>();
                copy_info(obj, asts.front());
                obj->end = asts.back()->end;
                for (const auto &s : tmps) {
                    obj->is_pair.push_back(s->get_type() == s_object_pair);
                    if (s->get_type() == s_object_pair)
                        obj->pairs.push_back(std::dynamic_pointer_cast<js_sym_object_pair_t>(s));
                    else
                        obj->rests.push_back(to_exp(s));
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(obj);
            }
                break;
            case c_expressionSequence: {
                if (tmps.size() > 1) {
                    auto seq = std::make_shared<js_sym_exp_seq_t>();
                    if (!tmps.empty()) {
                        copy_info(seq, tmps.front());
                    }
                    for (const auto &s : tmps) {
                        seq->exps.push_back(to_exp(s));
                        seq->end = s->end;
                    }
                    asts.clear();
                    tmps.clear();
                    tmps.push_back(seq);
                }
            }
                break;
            case c_propertyExpressionAssignment: {
                auto p = std::make_shared<js_sym_object_pair_t>();
                if (tmps.size() == 2) {
                    copy_info(p, tmps.front());
                    p->end = tmps.back()->end;
                    p->key = to_exp(tmps.front());
                    p->value = to_exp(tmps.back());
                } else {
                    if (tmps.front()->get_type() == s_var_id) {
                        auto id = std::dynamic_pointer_cast<js_sym_var_id_t>(tmps.front());
                        id->node->flag = a_string;
                        auto new_id = std::make_shared<js_sym_var_t>(id->node);
                        copy_info(new_id, tmps.front());
                        tmps.front() = new_id;
                    }
                    copy_info(p, asts.front());
                    p->end = tmps.back()->end;
                    if (AST_IS_ID(asts.front())) {
                        asts.front()->flag = a_string;
                    }
                    p->key = to_exp(primary_node(asts.front()));
                    copy_info(p->key, asts.front());
                    p->value = to_exp(tmps.back());
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(p);
            }
                break;
            case c_propertyShorthand: {
                if (!tmps.empty() && !asts.empty()) {
                    tmps[0]->start = asts[0]->start;
                }
                asts.clear();
            }
                break;
            case c_anonymousFunctionDecl: {
                auto code = std::make_shared<js_sym_code_t>();
                copy_info(code, asts[0]);
                code->end = asts.back()->end;
                asts.pop_back();
                if (asts.size() >= 2 && (AST_IS_OP_N(*(asts.rbegin() + 1), T_ELLIPSIS))) {
                    asts.erase(asts.begin() + (asts.size() - 2));
                    code->rest = true;
                }
                decltype(code->args) _asts;
                std::transform(asts.begin() + 1, asts.end(),
                               std::back_inserter(_asts),
                               [this](const auto &x) { return this->primary_node(x); });
                std::transform(asts.begin() + 1, asts.end(),
                               std::back_inserter(code->args_str),
                               [this](const auto &x) { return x->data._identifier; });
                std::unordered_set<std::string> arg_set;
                for (const auto &s : _asts) {
                    if (arg_set.find(s->node->data._identifier) != arg_set.end()) {
                        error(s, "conflict arg");
                    }
                    arg_set.insert(s->node->data._identifier);
                }
                code->args = std::move(_asts);
                if (!tmps.empty()) {
                    code->body = tmps.front();
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(code);
            }
                break;
            case c_arrowFunction: {
                auto code = std::make_shared<js_sym_code_t>();
                code->arrow = true;
                copy_info(code, asts[0]);
                code->end = tmps.back()->end;
                if (asts.size() >= 2 && (AST_IS_OP_N(*(asts.rbegin() + 1), T_ELLIPSIS))) {
                    asts.erase(asts.begin() + (asts.size() - 2));
                    code->rest = true;
                }
                decltype(code->args) _asts;
                if (AST_IS_ID(asts.front())) { // single ID
                    _asts.push_back(primary_node(asts.front()));
                    code->args_str.emplace_back(asts.front()->data._identifier);
                } else {
                    asts.erase(asts.begin());
                    auto i = asts.begin();
                    for (; i != asts.end(); i++) {
                        if (AST_IS_OP_N(*i, T_RPARAN)) {
                            i++;
                            break;
                        }
                        _asts.push_back(primary_node(*i));
                        code->args_str.emplace_back((*i)->data._identifier);
                    }
                    asts.erase(asts.begin(), i);
                }
                std::unordered_set<std::string> arg_set;
                for (const auto &s : _asts) {
                    if (arg_set.find(s->node->data._identifier) != arg_set.end()) {
                        error(s, "conflict arg");
                    }
                    arg_set.insert(s->node->data._identifier);
                }
                code->args = std::move(_asts);
                if (!tmps.empty()) {
                    code->body = tmps.front();
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(code);
            }
                break;
            case c_memberIndexExpression: {
                auto exp = to_exp((tmp.rbegin() + 2)->front());
                if (exp->get_type() != s_member_index) {
                    auto t = std::make_shared<js_sym_member_index_t>(exp);
                    copy_info(t, exp);
                    tmps.front()->start = asts.front()->start;
                    tmps.front()->line = asts.front()->line;
                    tmps.front()->column = asts.front()->column;
                    tmps.front()->end = asts.back()->end;
                    t->indexes.push_back(to_exp(tmps.front()));
                    t->end = tmps.front()->end;
                    (tmp.rbegin() + 2)->back() = t;
                } else {
                    auto t = std::dynamic_pointer_cast<js_sym_member_index_t>(exp);
                    tmps.front()->start = asts.front()->start;
                    tmps.front()->line = asts.front()->line;
                    tmps.front()->column = asts.front()->column;
                    tmps.front()->end = asts.back()->end;
                    t->indexes.push_back(to_exp(tmps.front()));
                    t->end = tmps.front()->end;
                }
                tmps.clear();
                asts.clear();
            }
                break;
            case c_memberDotExpression: {
                auto exp = to_exp((tmp.rbegin() + 2)->front());
                if (exp->get_type() != s_member_dot) {
                    auto t = std::make_shared<js_sym_member_dot_t>(exp);
                    copy_info(t, exp);
                    t->dots.push_back(asts.front());
                    t->end = asts.front()->end;
                    (tmp.rbegin() + 2)->back() = t;
                } else {
                    auto t = std::dynamic_pointer_cast<js_sym_member_dot_t>(exp);
                    t->dots.push_back(asts.front());
                    t->end = asts.front()->end;
                }
                asts.clear();
            }
                break;
            case c_argumentsExpression: {
                auto exp = to_exp((tmp.rbegin() + 2)->front());
                if (exp->get_type() == s_member_dot) { // a.b(...)
                    auto old = std::dynamic_pointer_cast<js_sym_member_dot_t>(exp);
                    if (old->dots.size() > 1) { // a.b.c()
                        auto t = std::make_shared<js_sym_call_method_t>();
                        copy_info(t, exp);
                        t->method = old->dots.back();
                        old->dots.pop_back();
                        old->end = old->dots.back()->end;
                        t->obj = old;
                        t->end = asts.back()->end;
                        if (asts.empty())
                            for (const auto &s : tmps)
                                t->args.push_back(to_exp(s));
                        else {
                            size_t i = 0;
                            for (const auto &s : tmps) {
                                if (i < asts.size() && s->start > asts[i]->start) {
                                    t->rests.push_back(t->args.size());
                                    i++;
                                }
                                t->args.push_back(to_exp(s));
                            }
                        }
                        (tmp.rbegin() + 2)->back() = t;
                    } else { // a.b()
                        auto t = std::make_shared<js_sym_call_method_t>();
                        copy_info(t, exp);
                        t->method = old->dots.back();
                        t->obj = old->exp;
                        t->end = asts.back()->end;
                        if (asts.empty())
                            for (const auto &s : tmps)
                                t->args.push_back(to_exp(s));
                        else {
                            size_t i = 0;
                            for (const auto &s : tmps) {
                                if (i < asts.size() && s->start > asts[i]->start) {
                                    t->rests.push_back(t->args.size());
                                    i++;
                                }
                                t->args.push_back(to_exp(s));
                            }
                        }
                        (tmp.rbegin() + 2)->back() = t;
                    }
                } else { // a(...)
                    auto t = std::make_shared<js_sym_call_function_t>();
                    copy_info(t, exp);
                    t->obj = exp;
                    t->end = asts.back()->end;
                    if (asts.empty())
                        for (const auto &s : tmps)
                            t->args.push_back(to_exp(s));
                    else {
                        size_t i = 0;
                        for (const auto &s : tmps) {
                            if (i < asts.size() && s->start > asts[i]->start) {
                                t->rests.push_back(t->args.size());
                                i++;
                            }
                            t->args.push_back(to_exp(s));
                        }
                    }
                    (tmp.rbegin() + 2)->back() = t;
                }
                asts.clear();
                tmps.clear();
            }
                break;
            case c_postIncrementExpression:
            case c_postDecreaseExpression: {
                auto exp = to_exp((tmp.rbegin() + 2)->front());
                auto t = std::make_shared<js_sym_sinop_t>(exp, asts.front());
                copy_info(t, exp);
                t->end = asts.front()->end;
                (tmp.rbegin() + 2)->back() = t;
                asts.clear();
            }
                break;
            case c_newExpression: {
                auto exp = std::make_shared<js_sym_new_t>();
                if (tmps.size() == 1 && tmps.front()->get_type() == s_call_function) {
                    auto call = std::dynamic_pointer_cast<js_sym_call_function_t>(tmps.front());
                    copy_info(exp, asts.front());
                    exp->end = call->end;
                    exp->obj = call->obj;
                    exp->args = call->args;
                    exp->rests = call->rests;
                } else {
                    copy_info(exp, asts.front());
                    exp->end = tmps.back()->end;
                    assert(tmps.front()->get_base_type() == s_expression);
                    exp->obj = to_exp(tmps.front());
                    if (asts.size() <= 1) {
                        std::transform(tmps.begin() + 1, tmps.end(),
                                       std::back_inserter(exp->args),
                                       [](const auto &s) {
                                           assert(s->get_base_type() == s_expression);
                                           return std::dynamic_pointer_cast<js_sym_exp_t>(s);
                                       });
                    } else {
                        size_t i = 1;
                        for (auto s = tmps.begin() + 1; s != tmps.end(); s++) {
                            if (i < asts.size() && (*s)->start > asts[i]->start) {
                                exp->rests.push_back(exp->args.size());
                                i++;
                            }
                            exp->args.push_back(to_exp(*s));
                        }
                    }
                }
                asts.clear();
                tmps.clear();
                tmps.push_back(exp);
            }
                break;
            case c_deleteExpression:
            case c_voidExpression:
            case c_typeofExpression:
            case c_preIncrementExpression:
            case c_preDecreaseExpression:
            case c_unaryPlusExpression:
            case c_unaryMinusExpression:
            case c_bitNotExpression:
            case c_notExpression: {
                auto &op = asts[0];
                auto &_exp = (tmp.rbegin() + 2)->front();
                auto exp = to_exp(_exp);
                auto unop = std::make_shared<js_sym_unop_t>(exp, op);
                copy_info(unop, exp);
                unop->start = op->start;
                unop->line = op->line;
                unop->column = op->column;
                (tmp.rbegin() + 2)->front() = unop;
                asts.clear();
            }
                break;
            case c_instanceofExpression:
            case c_powerExpression:
            case c_multiplicativeExpression:
            case c_additiveExpression:
            case c_bitShiftExpression:
            case c_relationalExpression:
            case c_equalityExpression:
            case c_bitAndExpression:
            case c_bitXOrExpression:
            case c_bitOrExpression:
            case c_logicalAndExpression:
            case c_logicalOrExpression:
            case c_ternaryExpression: {
                size_t tmp_i = 0;
                auto exp1 = to_exp(tmps[tmp_i++]);
                auto exp2 = to_exp(tmps[tmp_i++]);
                for (size_t i = 0; i < asts.size(); ++i) {
                    auto &a = asts[i];
                    if (AST_IS_OP(a) || AST_IS_KEYWORD(a)) {
                        if (node->data._coll == c_ternaryExpression &&
                            AST_IS_OP_K(a, T_QUERY)) { // triop
                            auto exp3 = to_exp(tmps[tmp_i++]);
                            auto t = std::make_shared<js_sym_triop_t>(exp1, exp2, exp3, a, asts[i + 1]);
                            copy_info(t, exp1);
                            t->end = exp3->end;
                            exp1 = t;
                            if (tmp_i < tmps.size())
                                exp2 = to_exp(tmps[tmp_i++]);
                            i++;
                        } else { // binop
                            auto t = std::make_shared<js_sym_binop_t>(exp1, exp2, a);
                            copy_info(t, exp1);
                            t->end = exp2->end;
                            exp1 = t;
                            if (tmp_i < tmps.size())
                                exp2 = to_exp(tmps[tmp_i++]);
                        }
                    } else {
                        error(a, "invalid binop: coll");
                    }
                }
                tmps.clear();
                tmps.push_back(exp1);
                asts.clear();
            }
                break;
            case c_assignmentExpression:
            case c_assignmentOperatorExpression: {
                size_t tmp_i = 0;
                std::reverse(asts.begin(), asts.end());
                std::reverse(tmps.begin(), tmps.end());
                auto exp1 = to_exp(tmps[tmp_i++]);
                auto exp2 = to_exp(tmps[tmp_i++]);
                for (auto &a : asts) {
                    if (AST_IS_OP(a)) {
                        auto t = std::make_shared<js_sym_binop_t>(exp2, exp1, a);
                        copy_info(t, exp2);
                        t->end = exp1->end;
                        exp1 = t;
                        if (tmp_i < tmps.size())
                            exp2 = to_exp(tmps[tmp_i++]);
                    } else {
                        error(a, "invalid binop: coll");
                    }
                }
                tmps.clear();
                tmps.push_back(exp1);
                asts.clear();
            }
                break;
            case c_literal:
            case c_literalExpression:
            case c_identifierExpression:
            case c_thisExpression: {
                if (tmps.empty()) {
                    auto pri = primary_node(asts[0]);
                    copy_info(pri, asts[0]);
                    tmps.push_back(pri);
                    asts.clear();
                }
            }
                break;
            case c_parenthesizedExpression: {
                auto exp = tmps.front();
                if (exp->get_type() != s_code) {
                    exp->start = asts.front()->start;
                    exp->end = asts.back()->end;
                }
                asts.clear();
            }
                break;
            default:
                break;
        }
    }

    void cjsgen::error(js_ast_node *node, const std::string &str, bool info) const {
        std::stringstream ss;
        ss << "[" << node->line << ":" << node->column << ":" << node->start << ":" << node->end << "] ";
        ss << str;
        if (info) {
            cjsast::print(node, 0, *text, ss);
        }
        throw cjs_exception(ss.str());
    }

    void cjsgen::error(const js_sym_t::ref &s, const std::string &str) const {
        std::stringstream ss;
        ss << "[" << s->line << ":" << s->column << ":" << s->start << ":" << s->end << "] ";
        ss << str;
        throw cjs_exception(ss.str());
    }

    js_sym_exp_t::ref cjsgen::to_exp(const js_sym_t::ref &s) {
        if (s->get_base_type() != s_expression)
            error(s, "need expression: " + s->to_string());
        return std::dynamic_pointer_cast<js_sym_exp_t>(s);
    }

    js_sym_stmt_t::ref cjsgen::to_stmt(const js_sym_t::ref &s) {
        if (s->get_base_type() != s_statement)
            error(s, "need statement: " + s->to_string());
        return std::dynamic_pointer_cast<js_sym_stmt_t>(s);
    }

    js_sym_t::ref cjsgen::find_symbol(js_ast_node *node) {
        return nullptr;
    }

    js_sym_var_t::ref cjsgen::primary_node(js_ast_node *node) {
        switch (node->flag) {
            case a_literal: {
                auto sym = find_symbol(node);
                return std::make_shared<js_sym_var_id_t>(node, sym);
            }
            case a_string:
            case a_regex:
            case a_number:
                return std::make_shared<js_sym_var_t>(node);
            case a_keyword: {
                if (AST_IS_KEYWORD_K(node, K_TRUE) || AST_IS_KEYWORD_K(node, K_FALSE) ||
                    AST_IS_KEYWORD_K(node, K_NULL) || AST_IS_KEYWORD_K(node, K_UNDEFINED) ||
                    AST_IS_KEYWORD_K(node, K_THIS))
                    return std::make_shared<js_sym_var_t>(node);
                else
                    error(node, "invalid var keyword type: ", true);
            }
                break;
            default:
                break;
        }
        return std::make_shared<js_sym_var_t>(node);
    }

    void cjsgen::print(const js_sym_t::ref &node, int level, std::ostream &os) {
        if (node == nullptr)
            return;
        auto type = node->get_type();
        os << std::setfill(' ') << std::setw(level) << "";
        switch (type) {
            case s_sym:
                break;
            case s_id:
                os << "id"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_id_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "id" << std::endl;
                    for (const auto &s : n->ids) {
                        print(s, level + 2, os);
                    }
                    if (n->init) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "init" << std::endl;
                        print(n->init, level + 2, os);
                    }
                }
                break;
            case s_function:
                break;
            case s_var:
                os << "var"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_var_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << cjsast::to_string(n->node) << std::endl;
                }
                break;
            case s_var_id:
                os << "var_id"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_var_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    switch (n->clazz) {
                        case js_sym_var_t::local:
                            os << "[LOCAL] ";
                            break;
                        case js_sym_var_t::fast:
                            os << "[FAST] ";
                            break;
                        case js_sym_var_t::closure:
                            os << "[CLOSURE] ";
                            break;
                        case js_sym_var_t::global:
                            os << "[GLOBAL] ";
                            break;
                        default:
                            break;
                    }
                    os << n->node->data._string << std::endl;
                }
                break;
            case s_expression:
                break;
            case s_unop:
                os << "unop"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_unop_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "op: " << js_lexer_string(n->op->data._op)
                       << " " << "[" << n->op->line << ":"
                       << n->op->column << ":"
                       << n->op->start << ":"
                       << n->op->end << "]" << std::endl;
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp" << std::endl;
                    print(n->exp, level + 2, os);
                }
                break;
            case s_sinop:
                os << "sinop"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_sinop_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp" << std::endl;
                    print(n->exp, level + 2, os);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "op: " << js_lexer_string(n->op->data._op)
                       << " " << "[" << n->op->line << ":"
                       << n->op->column << ":"
                       << n->op->start << ":"
                       << n->op->end << "]" << std::endl;
                }
                break;
            case s_binop:
                os << "binop"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_binop_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp1" << std::endl;
                    print(n->exp1, level + 2, os);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "op: " << js_lexer_string(n->op->data._op)
                       << " " << "[" << n->op->line << ":"
                       << n->op->column << ":"
                       << n->op->start << ":"
                       << n->op->end << "]" << std::endl;
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp2" << std::endl;
                    print(n->exp2, level + 2, os);
                }
                break;
            case s_triop:
                os << "binop"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_triop_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp1" << std::endl;
                    print(n->exp1, level + 2, os);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "op1: " << js_lexer_string(n->op1->data._op)
                       << " " << "[" << n->op1->line << ":"
                       << n->op1->column << ":"
                       << n->op1->start << ":"
                       << n->op1->end << "]" << std::endl;
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp2" << std::endl;
                    print(n->exp2, level + 2, os);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "op2: " << js_lexer_string(n->op2->data._op)
                       << " " << "[" << n->op2->line << ":"
                       << n->op2->column << ":"
                       << n->op2->start << ":"
                       << n->op2->end << "]" << std::endl;
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp3" << std::endl;
                    print(n->exp3, level + 2, os);
                }
                break;
            case s_member_dot:
                os << "member_dot"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_member_dot_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp" << std::endl;
                    print(n->exp, level + 2, os);
                    for (const auto &s : n->dots) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "dot: " << s->data._identifier
                           << " " << "[" << s->line << ":"
                           << s->column << ":"
                           << s->start << ":"
                           << s->end << "]" << std::endl;
                    }
                }
                break;
            case s_member_index:
                os << "member_index"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_member_index_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "exp" << std::endl;
                    print(n->exp, level + 2, os);
                    for (const auto &s : n->indexes) {
                        print(s, level + 1, os);
                    }
                }
                break;
            case s_expression_seq:
                os << "exp_seq"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_exp_seq_t>(node);
                    for (const auto &s : n->exps) {
                        print(s, level + 1, os);
                    }
                }
                break;
            case s_array:
                os << "array"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_array_t>(node);
                    for (const auto &s : n->exps) {
                        if (s)
                            print(s, level + 1, os);
                        else {
                            os << std::setfill(' ') << std::setw(level + 1) << "";
                            os << "empty" << std::endl;
                        }
                    }
                }
                break;
            case s_object:
                os << "object"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_object_t>(node);
                    for (const auto &s : n->pairs) {
                        print(s, level + 1, os);
                    }
                }
                break;
            case s_object_pair:
                os << "pair"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_object_pair_t>(node);
                    print(n->key, level + 1, os);
                    print(n->value, level + 1, os);
                }
                break;
            case s_call_method:
                os << "call_method"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_call_method_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "obj" << std::endl;
                    print(n->obj, level + 2, os);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "method: " << n->method->data._identifier
                       << " " << "[" << n->method->line << ":"
                       << n->method->column << ":"
                       << n->method->start << ":"
                       << n->method->end << "]" << std::endl;
                    if (!n->args.empty()) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "args" << std::endl;
                        for (const auto &s : n->args) {
                            print(s, level + 2, os);
                        }
                    }
                }
                break;
            case s_call_function:
                os << "call_function"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_call_function_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "obj" << std::endl;
                    print(n->obj, level + 2, os);
                    if (!n->args.empty()) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "args" << std::endl;
                        for (const auto &s : n->args) {
                            print(s, level + 2, os);
                        }
                    }
                }
                break;
            case s_new:
                os << "new"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_new_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "obj" << std::endl;
                    print(n->obj, level + 2, os);
                    if (!n->args.empty()) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "args" << std::endl;
                        for (const auto &s : n->args) {
                            print(s, level + 2, os);
                        }
                    }
                }
                break;
            case s_ctrl:
                break;
            case s_statement:
                break;
            case s_statement_var:
                os << "statement_var"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_var_t>(node);
                    for (const auto &s : n->vars) {
                        print(s, level + 1, os);
                    }
                }
                break;
            case s_statement_exp:
                os << "statement_exp"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_exp_t>(node);
                    print(n->seq, level + 1, os);
                }
                break;
            case s_statement_return:
                os << "statement_return"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_return_t>(node);
                    if (n->seq)
                        print(n->seq, level + 1, os);
                }
                break;
            case s_statement_throw:
                os << "statement_throw"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_throw_t>(node);
                    if (n->seq)
                        print(n->seq, level + 1, os);
                }
                break;
            case s_statement_control: {
                auto n = std::dynamic_pointer_cast<js_sym_stmt_control_t>(node);
                os << "statement_" << js_lexer_string(js_lexer_t(n->keyword))
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                if (n->label)
                    print(n->label, level + 1, os);
            }
                break;
            case s_statement_if:
                os << "if"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_if_t>(node);
                    print(n->seq, level + 1, os);
                    print(n->true_stmt, level + 1, os);
                    if (n->false_stmt)
                        print(n->false_stmt, level + 1, os);
                }
                break;
            case s_statement_while:
                os << "while"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_while_t>(node);
                    print(n->seq, level + 1, os);
                    print(n->stmt, level + 1, os);
                }
                break;
            case s_statement_for:
                os << "for"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_for_t>(node);
                    if (n->exp)
                        print(n->exp, level + 1, os);
                    else if (n->vars)
                        print(n->vars, level + 1, os);
                    if (n->cond)
                        print(n->cond, level + 1, os);
                    if (n->iter)
                        print(n->iter, level + 1, os);
                    print(n->body, level + 1, os);
                }
                break;
            case s_statement_for_in:
                os << "for in"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_for_in_t>(node);
                    if (n->exp)
                        print(n->exp, level + 1, os);
                    else
                        print(n->vars, level + 1, os);
                    print(n->iter, level + 1, os);
                    print(n->body, level + 1, os);
                }
                break;
            case s_case:
                os << "case"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_case_t>(node);
                    if (n->exp)
                        print(n->exp, level + 1, os);
                    for (const auto &s : n->stmts) {
                        print(s, level + 1, os);
                    }
                }
                break;
            case s_statement_switch:
                os << "switch"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_switch_t>(node);
                    print(n->exp, level + 1, os);
                    for (const auto &s : n->cases) {
                        print(s, level + 1, os);
                    }
                }
                break;
            case s_statement_try:
                os << "try"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_stmt_try_t>(node);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    os << "try" << std::endl;
                    print(n->try_body, level + 2, os);
                    os << std::setfill(' ') << std::setw(level + 1) << "";
                    if (n->var) {
                        os << "var" << std::endl;
                        print(n->var, level + 2, os);
                    }
                    if (n->catch_body) {
                        os << "catch" << std::endl;
                        print(n->catch_body, level + 2, os);
                    }
                    if (n->finally_body) {
                        os << "finally" << std::endl;
                        print(n->finally_body, level + 2, os);
                    }
                }
                break;
            case s_block:
                os << "block"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                for (const auto &s : std::dynamic_pointer_cast<js_sym_block_t>(node)->stmts) {
                    print(s, level + 1, os);
                }
                break;
            case s_code:
                os << "code"
                   << " " << "[" << node->line << ":"
                   << node->column << ":"
                   << node->start << ":"
                   << node->end << "]" << std::endl;
                {
                    auto n = std::dynamic_pointer_cast<js_sym_code_t>(node);
                    if (n->name) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "name: " << n->name->data._identifier
                           << " " << "[" << n->name->line << ":"
                           << n->name->column << ":"
                           << n->name->start << ":"
                           << n->name->end << "]" << std::endl;
                    }
                    if (!n->args.empty()) {
                        os << std::setfill(' ') << std::setw(level + 1) << "";
                        os << "args" << std::endl;
                        for (const auto &s : n->args) {
                            print(s, level + 2, os);
                        }
                    }
                    if (n->body)
                        print(n->body, level + 1, os);
                }
                break;
            default:
                break;
        }
    }

    void cjsgen::emit(js_ast_node_index *idx, js_ins_t i) {
        if (idx)
            codes.back()->codes.push_back({idx->line, idx->column, idx->start, idx->end, i, 0, 0, 0});
        else
            codes.back()->codes.push_back({0, 0, 0, 0, i, 0, 0, 0});
    }

    void cjsgen::emit(js_ast_node_index *idx, js_ins_t i, int a) {
        if (idx)
            codes.back()->codes.push_back({idx->line, idx->column, idx->start, idx->end, i, 1, a, 0});
        else
            codes.back()->codes.push_back({0, 0, 0, 0, i, 1, a, 0});
    }

    void cjsgen::emit(js_ast_node_index *idx, js_ins_t i, int a, int b) {
        if (idx)
            codes.back()->codes.push_back({idx->line, idx->column, idx->start, idx->end, i, 2, a, b});
        else
            codes.back()->codes.push_back({0, 0, 0, 0, i, 2, a, b});
    }

    int cjsgen::get_ins(int n, int op) const {
        if (op == 0)
            return codes.back()->codes.at(n).code;
        if (op == 1)
            return codes.back()->codes.at(n).op1;
        if (op == 2)
            return codes.back()->codes.at(n).op2;
        return 0;
    }

    int cjsgen::code_length() const {
        return (int) codes.back()->codes.size();
    }

    void cjsgen::edit(int code, int idx, int value) {
        switch (idx) {
            case 0:
                codes.back()->codes.at(code).code = value;
                break;
            case 1:
                codes.back()->codes.at(code).op1 = value;
                break;
            case 2:
                codes.back()->codes.at(code).op2 = value;
                break;
            case 3:
                codes.back()->codes.at(code).opnum = value;
                break;
            default:
                break;
        }
    }

    int cjsgen::load_number(double d) {
        return codes.back()->consts.get_number(d);
    }

    int cjsgen::load_string(const std::string &s, int type) {
        return codes.back()->consts.get_string(s, (cjs_consts::get_string_t) (type));
    }

    int cjsgen::push_function(std::shared_ptr<js_sym_code_t> code) {
        funcs.push_back(code);
        auto id = codes.back()->consts.get_function(code);
        codes.push_back(std::move(code));
        return id;
    }

    void cjsgen::pop_function() {
        codes.pop_back();
    }

    void cjsgen::enter(int type, js_sym_t::ref s) {
        codes.back()->scopes.emplace_back();
        codes.back()->scopes.back().type = (cjs_scope_t) type;
        if (s)
            codes.back()->scopes.back().sym = std::move(s);
    }

    void cjsgen::leave() {
        codes.back()->scopes.pop_back();
    }

    void cjsgen::push_rewrites(int index, int type) {
        if (type == K_BREAK || type == K_CONTINUE) {
            auto &scope = codes.back()->scopes;
            for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                if (s->type == sp_for ||
                    s->type == sp_for_each ||
                    s->type == sp_while ||
                    s->type == sp_do_while ||
                    s->type == sp_switch) {
                    s->rewrites[index] = type;
                    return;
                }
            }
            assert(!"invalid break/continue");
        } else {
            assert(!"invalid rewrites");
        }
    }

    const std::unordered_map<int, int> &cjsgen::get_rewrites() {
        return codes.back()->scopes.back().rewrites;
    }

    std::shared_ptr<js_sym_t> cjsgen::get_var(const std::string &name, int t) {
        auto type = (cjs_scope_query_t) t;
        switch (type) {
            case sq_local: {
                const auto &vars = codes.back()->scopes.back().vars;
                auto f = vars.find(name);
                if (f != vars.end()) {
                    return f->second.lock();
                }
                return nullptr;
            }
            case sq_local_func: {
                const auto &scope = codes.back()->scopes;
                for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                    const auto &vars = s->vars;
                    auto f = vars.find(name);
                    if (f != vars.end()) {
                        return f->second.lock();
                    }
                }
                const auto &args = codes.back()->args;
                for (const auto &arg : args) {
                    if (name == arg->node->data._identifier) {
                        return arg;
                    }
                }
                return nullptr;
            }
            case sq_all: {
                for (auto i = codes.rbegin() + 1; i != codes.rend(); i++) {
                    const auto &scope = (*i)->scopes;
                    for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                        const auto &vars = s->vars;
                        auto f = vars.find(name);
                        if (f != vars.end()) {
                            return f->second.lock();
                        }
                    }
                    const auto &args = (*i)->args;
                    for (const auto &arg : args) {
                        if (name == arg->node->data._identifier) {
                            return arg;
                        }
                    }
                }
                return nullptr;
            }
            default:
                break;
        }
        return nullptr;
    }

    void cjsgen::add_var(const std::string &name, std::shared_ptr<js_sym_t> id) {
        codes.back()->scopes.back().vars[name] = id;
    }

    void cjsgen::add_closure(std::shared_ptr<js_sym_var_id_t> c) {
        auto name = std::string(c->node->data._identifier);
        for (auto i = codes.rbegin(); i != codes.rend(); i++) {
            if ((*i)->closure_str.find(name) != (*i)->closure_str.end())
                return;
            auto has_find = false;
            const auto &scope = (*i)->scopes;
            for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                const auto &vars = s->vars;
                auto f = vars.find(name);
                if (f != vars.end()) {
                    has_find = true;
                    break;
                }
            }
            if (has_find) {
                break;
            }
            const auto &args = (*i)->args;
            for (const auto &arg : args) {
                if (name == arg->node->data._identifier) {
                    has_find = true;
                    break;
                }
            }
            if (has_find) {
                break;
            }
            (*i)->closure_str.insert(name);
            (*i)->closure.push_back(c);
        }
    }

    int cjsgen::get_func_level() const {
        return (int) codes.size();
    }

    std::string cjsgen::get_fullname(const std::string &name) const {
        if (codes.size() == 1)
            return name;
        std::stringstream ss;
        for (auto i = codes.begin() + 1; i != codes.end(); i++) {
            if ((*i)->name)
                ss << (*i)->name->data._identifier;
            else
                ss << LAMBDA_ID;
            ss << ".<locals>.";
        }
        ss << name;
        return ss.str();
    }

    std::string cjsgen::get_func_name() const {
        if (codes.empty())
            return "<global>";
        return codes.back()->fullName;
    }

    bool cjsgen::is_arrow_func() const {
        return codes.back()->arrow;
    }

    std::string cjsgen::get_code_text(js_ast_node_index *idx) const {
        assert(idx && text);
        assert(idx->start >= 0 && idx->start < (int) text->length());
        assert(idx->end >= 0 && idx->end < (int) text->length());
        assert(idx->start <= idx->end);
        return text->substr(idx->start, idx->end - idx->start);
    }

    std::string cjsgen::get_filename() const {
        return filename;
    }

    void cjsgen::gen_try(int t) {
        auto &scope = codes.back()->scopes;
        std::vector<js_sym_t::ref> f;
        if (t == K_RETURN) {
            for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                if ((s->type == sp_try ||
                    s->type == sp_catch) && s->sym) {
                    auto sym = s->sym;
                    if (sym) {
                        f.push_back(sym);
                    }
                }
                if (s->type == sp_finally)
                    break;
            }
        } else if (t == K_BREAK) {
            for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                if (s->type == sp_for ||
                    s->type == sp_for_each ||
                    s->type == sp_while ||
                    s->type == sp_do_while ||
                    s->type == sp_switch ||
                    s->type == sp_finally)
                    break;
                if ((s->type == sp_try ||
                     s->type == sp_catch) && s->sym) {
                    auto sym = s->sym;
                    if (sym) {
                        f.push_back(sym);
                    }
                }
            }
        } else if (t == K_CONTINUE) {
            for (auto s = scope.rbegin(); s != scope.rend(); s++) {
                if (s->type == sp_for ||
                    s->type == sp_for_each ||
                    s->type == sp_while ||
                    s->type == sp_do_while ||
                    s->type == sp_finally)
                    break;
                if ((s->type == sp_try ||
                     s->type == sp_catch) && s->sym) {
                    auto sym = s->sym;
                    if (sym) {
                        f.push_back(sym);
                    }
                }
            }
        }
        if (!f.empty()) {
            for (size_t i = 0; i < f.size(); i++) {
                emit(nullptr, EXIT_FINALLY);
            }
            enter(sp_finally);
            for (auto s = f.rbegin(); s != f.rend(); s++) {
                (*s)->gen_rvalue(*this);
            }
            leave();
        }
    }

    void cjsgen::error(js_ast_node_index *idx, const std::string &str) const {
        std::stringstream ss;
        ss << "[" << idx->line << ":" << idx->column << "] ";
        ss << str;
        ss << " (" << text->substr(idx->start, idx->end - idx->start) << ")";
        throw cjs_exception(ss.str());
    }

    void cjsgen::dump() const {
#if PRINT_CODE && DEBUG_MODE
        fprintf(stdout, "--== Main Function ==--\n");
        dump(codes.front(), true);
#else
        dump(codes.front(), false);
#endif
        for (const auto &c : funcs) {
#if PRINT_CODE && DEBUG_MODE
            fprintf(stdout, "--== Function: \"%s\" ==--\n", c->fullName.c_str());
            dump(c, true);
#else
            dump(c, false);
#endif
        }
    }

    void cjsgen::dump(js_sym_code_t::ref code, bool print) const {
        if (print)
            code->consts.dump(text);
        auto idx = 0;
        std::vector<int> jumps;
        {
            std::set<int, std::greater<>> jumps_set;
            for (const auto &c : code->codes) {
                switch (c.code) {
                    case JUMP_IF_TRUE_OR_POP:
                    case JUMP_IF_FALSE_OR_POP:
                    case POP_JUMP_IF_TRUE:
                    case POP_JUMP_IF_FALSE:
                    case JUMP_ABSOLUTE:
                        jumps_set.insert(c.op1);
                        break;
                    case FOR_ITER:
                    case JUMP_FORWARD:
                    case SETUP_FINALLY:
                        jumps_set.insert(idx + c.op1);
                        break;
                    default:
                        break;
                }
                idx++;
            }
            std::copy(jumps_set.begin(), jumps_set.end(), std::back_inserter(jumps));
        }
        idx = 0;
        char buf[256];
        for (auto &c : code->codes) {
            auto alt = text->substr(c.start, c.end - c.start);
            auto jmp = "  ";
            if (!jumps.empty() && jumps.back() == idx) {
                jmp = ">>";
                jumps.pop_back();
            }
            if (c.opnum == 0)
                snprintf(buf, sizeof(buf), "[%04d:%03d]  %s   %4d %-20s                   (%.100s)",
                         c.line, c.column, jmp, idx, js_ins_string(js_ins_t(c.code)),
                         c.line == 0 ? "..." : text->substr(c.start, c.end - c.start).c_str());
            else if (c.opnum == 1) {
                if (c.code == LOAD_CONST && c.line == 0) {
                    snprintf(buf, sizeof(buf), "[%04d:%03d]  %s   %4d %-20s %8d          (%.100s)",
                             c.line, c.column, jmp, idx, js_ins_string(js_ins_t(c.code)), c.op1,
                             code->consts.get_desc(c.op1).c_str());
                } else {
                    snprintf(buf, sizeof(buf), "[%04d:%03d]  %s   %4d %-20s %8d          (%.100s)",
                             c.line, c.column, jmp, idx, js_ins_string(js_ins_t(c.code)), c.op1,
                             c.line == 0 ? "..." : text->substr(c.start, c.end - c.start).c_str());
                }
            } else if (c.opnum == 2)
                snprintf(buf, sizeof(buf), "[%04d:%03d]  %s   %4d %-20s %8d %8d (%.100s)",
                         c.line, c.column, jmp, idx, js_ins_string(js_ins_t(c.code)), c.op1, c.op2,
                         c.line == 0 ? "..." : text->substr(c.start, c.end - c.start).c_str());
            if (print)
                fprintf(stdout, "C %s\n", buf);
            c.desc = buf;
            idx++;
        }
    }
}
