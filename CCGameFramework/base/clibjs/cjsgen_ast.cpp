//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <utility>
#include <cmath>
#include <cassert>
#include <sstream>
#include "cjsgen.h"
#include "cjsast.h"

namespace nlohmann {
    template<>
    struct adl_serializer<clib::cjs_consts> {
        static void to_json(json& j, const clib::cjs_consts& v) {
            v.to_json(j);
        }

        static void from_json(const json& j, clib::cjs_consts& v) {
            v.from_json(j);
        }
    };

    template<>
    struct adl_serializer<clib::cjs_code> {
        static void to_json(json& j, const clib::cjs_code& v) {
            j = nlohmann::json{
                {"line", v.line},
                {"column", v.column},
                {"start", v.start},
                {"end", v.end},
                {"code", v.code},
                {"opnum", v.opnum},
                {"op1", v.op1},
                {"op2", v.op2},
                {"desc", v.desc},
            };
        }

        static void from_json(const json& j, clib::cjs_code& v) {
            j.at("line").get_to(v.line);
            j.at("column").get_to(v.column);
            j.at("start").get_to(v.start);
            j.at("end").get_to(v.end);
            j.at("code").get_to(v.code);
            j.at("opnum").get_to(v.opnum);
            j.at("op1").get_to(v.op1);
            j.at("op2").get_to(v.op2);
            j.at("desc").get_to(v.desc);
        }
    };
}

namespace clib {

    js_symbol_t js_sym_t::get_type() const {
        return s_sym;
    }

    js_symbol_t js_sym_t::get_base_type() const {
        return s_sym;
    }

    std::string js_sym_t::to_string() const {
        return "";
    }

    int js_sym_t::gen_lvalue(ijsgen &gen) {
        return no_lvalue;
    }

    int js_sym_t::gen_rvalue(ijsgen &gen) {
        return 0;
    }

    int js_sym_t::gen_invoke(ijsgen &gen, js_sym_t::ref &list) {
        return 0;
    }

    int js_sym_t::set_parent(js_sym_t::ref node) {
        parent = node;
        return 0;
    }

    // ----

    js_symbol_t js_sym_exp_t::get_type() const {
        return s_expression;
    }

    js_symbol_t js_sym_exp_t::get_base_type() const {
        return s_expression;
    }

    // ----

    js_symbol_t js_sym_id_t::get_type() const {
        return s_id;
    }

    js_symbol_t js_sym_id_t::get_base_type() const {
        return s_id;
    }

    std::string js_sym_id_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_id_t::gen_lvalue(ijsgen &gen) {
        for (const auto &s : ids) {
            s->gen_lvalue(gen);
        }
        return js_sym_t::gen_lvalue(gen);
    }

    int js_sym_id_t::gen_rvalue(ijsgen &gen) {
        if (init) {
            init->gen_rvalue(gen);
        } else
            gen.emit(this, LOAD_UNDEFINED);
        for (const auto &s : ids) {
            s->gen_lvalue(gen);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_id_t::gen_rvalue_decl(ijsgen &gen) {
        gen.emit(this, LOAD_UNDEFINED);
        for (const auto &s : ids) {
            s->gen_lvalue(gen);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_id_t::set_parent(js_sym_t::ref node) {
        for (auto &s : ids) {
            s->set_parent(shared_from_this());
        }
        if (init)
            init->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    void js_sym_id_t::parse() {
        if (!init)return;
        auto i = init;
        while (i) {
            if (i->get_type() == s_binop) {
                auto b = std::dynamic_pointer_cast<js_sym_binop_t>(i);
                if (b->op->data._op == T_ASSIGN) {
                    auto d = b->exp1;
                    if (d->get_type() == s_var_id) {
                        auto old_id = std::dynamic_pointer_cast<js_sym_var_t>(b->exp1);
                        auto new_id = std::make_shared<js_sym_var_t>(old_id->node);
                        copy_info(new_id, old_id);
                        ids.push_back(new_id);
                        i = b->exp2;
                        init = i;
                        continue;
                    }
                }
            }
            break;
        }
    }

    // ----

    js_sym_var_t::js_sym_var_t(js_ast_node *node) : node(node) {

    }

    js_symbol_t js_sym_var_t::get_type() const {
        return s_var;
    }

    std::string js_sym_var_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_var_t::gen_lvalue(ijsgen &gen) {
        return gen_lvalue_decl(gen);
    }

    int js_sym_var_t::gen_lvalue_decl(ijsgen& gen, bool check)
    {
        switch (node->flag) {
        case a_literal:
            if (clazz == local) {
                gen.emit(this, STORE_NAME, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_name));
                if (parent.lock()->get_type() == s_id) {
                    if (check && gen.get_var(node->data._string, sq_local) != nullptr)
                        gen.error(this, "id conflict");
                    gen.add_var(node->data._string, shared_from_this());
                }
            }
            else if (clazz == fast) {
                gen.emit(this, STORE_FAST, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_name));
                if (parent.lock()->get_type() == s_id) {
                    if (check && gen.get_var(node->data._string, sq_local) != nullptr)
                        gen.error(this, "id conflict");
                    gen.add_var(node->data._string, shared_from_this());
                }
            }
            else if (clazz == global) {
                gen.emit(this, STORE_GLOBAL, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_global));
            }
            else if (clazz == closure) {
                gen.emit(this, STORE_DEREF, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_deref));
            }
            else {
                gen.error(this, "unsupported class type");
            }
            break;
        default:
            gen.error(this, "unsupported var type");
            break;
        }
        return can_be_lvalue;
    }

    int js_sym_var_t::gen_rvalue(ijsgen &gen) {
        switch (node->flag) {
            case a_literal:
                if (clazz == local) {
                    gen.emit(this, LOAD_NAME, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_name));
                } else if (clazz == fast) {
                    gen.emit(this, LOAD_FAST, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_name));
                } else if (clazz == global) {
                    gen.emit(this, LOAD_GLOBAL, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_global));
                } else if (clazz == closure) {
                    gen.emit(this, LOAD_DEREF, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_deref));
                } else {
                    gen.error(this, "unsupported class type");
                }
                break;
            case a_string:
                gen.emit(this, LOAD_CONST, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_string));
                break;
            case a_number:
                if (node->data._number != 0) {
                    gen.emit(this, LOAD_CONST, gen.load_number(node->data._number));
                } else {
                    if (std::signbit(node->data._number) == 0)
                        gen.emit(this, LOAD_ZERO, 0);
                    else
                        gen.emit(this, LOAD_ZERO, 1);
                }
                break;
            case a_regex:
                gen.emit(this, LOAD_CONST, gen.load_string(node->data._string, cjs_consts::get_string_t::gs_regex));
                break;
            case a_keyword:
                if (node->data._keyword == K_TRUE)
                    gen.emit(this, LOAD_TRUE);
                else if (node->data._keyword == K_FALSE)
                    gen.emit(this, LOAD_FALSE);
                else if (node->data._keyword == K_NULL)
                    gen.emit(this, LOAD_NULL);
                else if (node->data._keyword == K_UNDEFINED)
                    gen.emit(this, LOAD_UNDEFINED);
                else if (node->data._keyword == K_THIS)
                    gen.emit(this, LOAD_THIS);
                else
                    gen.error(this, "unsupported var type");
                break;
            default:
                gen.error(this, "unsupported var type");
                break;
        }
        return js_sym_t::gen_rvalue(gen);
    }

    // ----

    js_sym_var_id_t::js_sym_var_id_t(js_ast_node *node, const js_sym_t::ref &symbol) : js_sym_var_t(node), id(symbol) {

    }

    js_symbol_t js_sym_var_id_t::get_type() const {
        return s_var_id;
    }

    std::string js_sym_var_id_t::to_string() const {
        return js_sym_var_t::to_string();
    }

    int js_sym_var_id_t::gen_lvalue(ijsgen &gen) {
        init_id(gen);
        return js_sym_var_t::gen_lvalue(gen);
    }

    int js_sym_var_id_t::gen_rvalue(ijsgen &gen) {
        init_id(gen);
        return js_sym_var_t::gen_rvalue(gen);
    }

    int js_sym_var_id_t::gen_invoke(ijsgen &gen, js_sym_t::ref &list) {
        return js_sym_t::gen_invoke(gen, list);
    }

    void js_sym_var_id_t::init_id(ijsgen &gen) {
        auto str = std::string(node->data._identifier);
        if (str == "arguments") {
            clazz = gen.get_func_level() == 1 ? global : fast;
            return;
        }
        auto i = gen.get_var(str, sq_local_func);
        if (i) {
            id = i;
            clazz = gen.get_func_level() == 1 ? local : fast;
        } else {
            i = gen.get_var(str, sq_all);
            if (i) {
                id = i;
                clazz = closure;
                gen.add_closure(std::dynamic_pointer_cast<js_sym_var_id_t>(shared_from_this()));
            } else {
                clazz = global;
            }
        }
    }

    // ----

    js_sym_unop_t::js_sym_unop_t(js_sym_exp_t::ref exp, js_ast_node *op) : exp(std::move(exp)), op(op) {

    }

    js_symbol_t js_sym_unop_t::get_type() const {
        return s_unop;
    }

    std::string js_sym_unop_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_unop_t::gen_lvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        switch (op->data._op) {
            case T_INC:
                gen.emit(this, BINARY_INC);
                gen.emit(this, DUP_TOP);
                if (exp->gen_lvalue(gen) == no_lvalue)
                    gen.error(this, "unsupported unop");
                gen.emit(this, POP_TOP);
                break;
            case T_DEC:
                gen.emit(this, BINARY_DEC);
                gen.emit(this, DUP_TOP);
                if (exp->gen_lvalue(gen) == no_lvalue)
                    gen.error(this, "unsupported unop");
                gen.emit(this, POP_TOP);
                break;
            default:
                gen.error(this, "unsupported unop");
                break;
        }
        return js_sym_t::gen_lvalue(gen);
    }

    int js_sym_unop_t::gen_rvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        switch (op->data._op) {
            case T_INC:
                gen.emit(this, BINARY_INC);
                if (exp->gen_lvalue(gen) == no_lvalue)
                    gen.error(this, "unsupported unop");
                break;
            case T_DEC:
                gen.emit(this, BINARY_DEC);
                if (exp->gen_lvalue(gen) == no_lvalue)
                    gen.error(this, "unsupported unop");
                break;
            case T_LOG_NOT:
                gen.emit(this, UNARY_NOT);
                break;
            case T_BIT_NOT:
                gen.emit(this, UNARY_INVERT);
                break;
            case T_ADD:
                gen.emit(this, UNARY_POSITIVE);
                break;
            case T_SUB:
                gen.emit(this, UNARY_NEGATIVE);
                break;
            case K_DELETE: {
                auto ins = gen.get_ins(gen.code_length() - 1);
                gen.edit(gen.code_length() - 1, 0, UNARY_DELETE);
                if (ins == LOAD_ATTR) {
                    auto op1 = gen.get_ins(gen.code_length() - 1, 1);
                    gen.edit(gen.code_length() - 1, 1, op1);
                    gen.edit(gen.code_length() - 1, 3, 1);
                } else if (ins == BINARY_SUBSCR) {
                    gen.edit(gen.code_length() - 1, 1, -1);
                    gen.edit(gen.code_length() - 1, 3, 1);
                } else if (ins == LOAD_GLOBAL) {
                    auto op1 = gen.get_ins(gen.code_length() - 1, 1);
                    gen.edit(gen.code_length() - 1, 1, -2);
                    gen.edit(gen.code_length() - 1, 2, op1);
                    gen.edit(gen.code_length() - 1, 3, 2);
                } else {
                    gen.edit(gen.code_length() - 1, 1, -8);
                }
            }
                break;
            case K_TYPEOF:
                gen.emit(this, UNARY_TYPEOF);
                break;
            default:
                gen.error(this, "unsupported unop");
                break;
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_unop_t::set_parent(js_sym_t::ref node) {
        exp->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    // ----

    js_sym_sinop_t::js_sym_sinop_t(js_sym_exp_t::ref exp, js_ast_node *op) : exp(std::move(exp)), op(op) {

    }

    js_symbol_t js_sym_sinop_t::get_type() const {
        return s_sinop;
    }

    std::string js_sym_sinop_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_sinop_t::gen_lvalue(ijsgen &gen) {
        return exp->gen_lvalue(gen);
    }

    int js_sym_sinop_t::gen_rvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        switch (op->data._op) {
            case T_INC:
                gen.emit(this, DUP_TOP);
                gen.emit(this, BINARY_INC);
                if (exp->gen_lvalue(gen) == no_lvalue)
                    gen.error(this, "unsupported sinop");
                gen.emit(this, POP_TOP);
                break;
            case T_DEC:
                gen.emit(this, DUP_TOP);
                gen.emit(this, BINARY_DEC);
                if (exp->gen_lvalue(gen) == no_lvalue)
                    gen.error(this, "unsupported sinop");
                gen.emit(this, POP_TOP);
                break;
            default:
                gen.error(this, "unsupported sinop");
                break;
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_sinop_t::set_parent(js_sym_t::ref node) {
        exp->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    // ----

    js_sym_binop_t::js_sym_binop_t(js_sym_exp_t::ref exp1, js_sym_exp_t::ref exp2, js_ast_node *op)
            : exp1(std::move(exp1)), exp2(std::move(exp2)), op(op) {

    }

    js_symbol_t js_sym_binop_t::get_type() const {
        return s_binop;
    }

    std::string js_sym_binop_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_binop_t::gen_lvalue(ijsgen &gen) {
        return js_sym_t::gen_lvalue(gen);
    }

    int js_sym_binop_t::gen_rvalue(ijsgen &gen) {
        if (op->flag == a_keyword) {
            exp1->gen_rvalue(gen);
            exp2->gen_rvalue(gen);
            if (op->data._keyword == K_INSTANCEOF) {
                gen.emit(this, INSTANCE_OF);
            }
            else if (op->data._keyword == K_IN) {
                gen.emit(this, OBJECT_IN);
            }
            else {
                gen.error(this, "unsupported binop");
            }
            return 0;
        }
        assert(op->flag == a_operator);
        switch (op->data._op) {
            case T_ASSIGN: {
                exp2->gen_rvalue(gen);
                if (exp1->gen_lvalue(gen) == 0) {
                    gen.error(this, "invalid assignment");
                }
            }
                return 0;
            case T_ASSIGN_ADD:
            case T_ASSIGN_SUB:
            case T_ASSIGN_MUL:
            case T_ASSIGN_DIV:
            case T_ASSIGN_MOD:
            case T_ASSIGN_LSHIFT:
            case T_ASSIGN_RSHIFT:
            case T_ASSIGN_URSHIFT:
            case T_ASSIGN_AND:
            case T_ASSIGN_OR:
            case T_ASSIGN_XOR:
            case T_ASSIGN_POWER: {
                exp1->gen_rvalue(gen);
                exp2->gen_rvalue(gen);
                switch (op->data._op) {
                    case T_ASSIGN_ADD:
                        gen.emit(this, BINARY_ADD);
                        break;
                    case T_ASSIGN_SUB:
                        gen.emit(this, BINARY_SUBTRACT);
                        break;
                    case T_ASSIGN_MUL:
                        gen.emit(this, BINARY_MULTIPLY);
                        break;
                    case T_ASSIGN_DIV:
                        gen.emit(this, BINARY_TRUE_DIVIDE);
                        break;
                    case T_ASSIGN_MOD:
                        gen.emit(this, BINARY_MODULO);
                        break;
                    case T_ASSIGN_LSHIFT:
                        gen.emit(this, BINARY_LSHIFT);
                        break;
                    case T_ASSIGN_RSHIFT:
                        gen.emit(this, BINARY_RSHIFT);
                        break;
                    case T_ASSIGN_URSHIFT:
                        gen.emit(this, BINARY_URSHIFT);
                        break;
                    case T_ASSIGN_AND:
                        gen.emit(this, BINARY_AND);
                        break;
                    case T_ASSIGN_OR:
                        gen.emit(this, BINARY_OR);
                        break;
                    case T_ASSIGN_XOR:
                        gen.emit(this, BINARY_XOR);
                        break;
                    case T_ASSIGN_POWER:
                        gen.emit(this, BINARY_POWER);
                        break;
                    default:
                        break;
                }
                if (exp1->gen_lvalue(gen) == 0) {
                    gen.error(this, "invalid assignment");
                }
            }
                return 0;
            default:
                break;
        }
        exp1->gen_rvalue(gen);
        switch (op->data._op) {
            case T_LOG_AND: {
                auto idx = gen.code_length();
                gen.emit(this, JUMP_IF_FALSE_OR_POP, 0);
                exp2->gen_rvalue(gen);
                gen.edit(idx, 1, gen.code_length());
            }
                return 0;
            case T_LOG_OR: {
                auto idx = gen.code_length();
                gen.emit(this, JUMP_IF_TRUE_OR_POP, 0);
                exp2->gen_rvalue(gen);
                gen.edit(idx, 1, gen.code_length());
            }
                return 0;
            default:
                break;
        }
        exp2->gen_rvalue(gen);
        switch (op->data._op) {
            case T_ADD:
                gen.emit(this, BINARY_ADD);
                break;
            case T_SUB:
                gen.emit(this, BINARY_SUBTRACT);
                break;
            case T_MUL:
                gen.emit(this, BINARY_MULTIPLY);
                break;
            case T_DIV:
                gen.emit(this, BINARY_TRUE_DIVIDE);
                break;
            case T_MOD:
                gen.emit(this, BINARY_MODULO);
                break;
            case T_POWER:
                gen.emit(this, BINARY_POWER);
                break;
            case T_LESS:
                gen.emit(this, COMPARE_LESS);
                break;
            case T_LESS_EQUAL:
                gen.emit(this, COMPARE_LESS_EQUAL);
                break;
            case T_EQUAL:
                gen.emit(this, COMPARE_EQUAL);
                break;
            case T_NOT_EQUAL:
                gen.emit(this, COMPARE_NOT_EQUAL);
                break;
            case T_GREATER:
                gen.emit(this, COMPARE_GREATER);
                break;
            case T_GREATER_EQUAL:
                gen.emit(this, COMPARE_GREATER_EQUAL);
                break;
            case T_FEQUAL:
                gen.emit(this, COMPARE_FEQUAL);
                break;
            case T_FNOT_EQUAL:
                gen.emit(this, COMPARE_FNOT_EQUAL);
                break;
            case T_BIT_AND:
                gen.emit(this, BINARY_AND);
                break;
            case T_BIT_OR:
                gen.emit(this, BINARY_OR);
                break;
            case T_BIT_XOR:
                gen.emit(this, BINARY_XOR);
                break;
            case T_LSHIFT:
                gen.emit(this, BINARY_LSHIFT);
                break;
            case T_RSHIFT:
                gen.emit(this, BINARY_RSHIFT);
                break;
            case T_URSHIFT:
                gen.emit(this, BINARY_URSHIFT);
                break;
            default:
                gen.error(this, "unsupported binop");
                break;
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_binop_t::set_parent(js_sym_t::ref node) {
        exp1->set_parent(shared_from_this());
        exp2->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    // ----

    js_sym_triop_t::js_sym_triop_t(js_sym_exp_t::ref exp1, js_sym_exp_t::ref exp2,
                             js_sym_exp_t::ref exp3, js_ast_node *op1, js_ast_node *op2)
            : exp1(std::move(exp1)), exp2(std::move(exp2)), exp3(std::move(exp3)), op1(op1), op2(op2) {

    }

    js_symbol_t js_sym_triop_t::get_type() const {
        return s_triop;
    }

    std::string js_sym_triop_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_triop_t::gen_lvalue(ijsgen &gen) {
        return js_sym_t::gen_lvalue(gen);
    }

    int js_sym_triop_t::gen_rvalue(ijsgen &gen) {
        if (op1->data._op == T_QUERY && op2->data._op == T_COLON) {
            exp1->gen_rvalue(gen);
            auto idx = gen.code_length();
            gen.emit(op1, POP_JUMP_IF_FALSE, 0);
            exp2->gen_rvalue(gen);
            auto idx2 = gen.code_length();
            auto cur = gen.code_length();
            gen.emit(op2, JUMP_FORWARD, 0);
            gen.edit(idx, 1, gen.code_length());
            exp3->gen_rvalue(gen);
            gen.edit(idx2, 1, gen.code_length() - cur);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_triop_t::set_parent(js_sym_t::ref node) {
        exp1->set_parent(shared_from_this());
        exp2->set_parent(shared_from_this());
        exp3->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    // ----

    js_sym_member_dot_t::js_sym_member_dot_t(js_sym_exp_t::ref exp) : exp(std::move(exp)) {

    }

    js_symbol_t js_sym_member_dot_t::get_type() const {
        return s_member_dot;
    }

    std::string js_sym_member_dot_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_member_dot_t::gen_lvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        size_t i = 0;
        for (const auto &s : dots) {
            if (i + 1 < dots.size())
                gen.emit(s, LOAD_ATTR, gen.load_string(s->data._string, cjs_consts::get_string_t::gs_name));
            else
                gen.emit(s, STORE_ATTR, gen.load_string(s->data._string, cjs_consts::get_string_t::gs_name));
            i++;
        }
        return can_be_lvalue;
    }

    int js_sym_member_dot_t::gen_rvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        for (const auto &s : dots) {
            gen.emit(s, LOAD_ATTR, gen.load_string(s->data._string, cjs_consts::get_string_t::gs_name));
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_member_dot_t::set_parent(js_sym_t::ref node) {
        exp->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    // ----

    js_sym_member_index_t::js_sym_member_index_t(js_sym_exp_t::ref exp) : exp(std::move(exp)) {

    }

    js_symbol_t js_sym_member_index_t::get_type() const {
        return s_member_index;
    }

    std::string js_sym_member_index_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_member_index_t::gen_lvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        size_t i = 0;
        for (const auto &s : indexes) {
            if (i + 1 < indexes.size()) {
                s->gen_rvalue(gen);
                gen.emit(s.get(), BINARY_SUBSCR);
            } else {
                s->gen_rvalue(gen);
                gen.emit(s.get(), STORE_SUBSCR);
            }
            i++;
        }
        return can_be_lvalue;
    }

    int js_sym_member_index_t::gen_rvalue(ijsgen &gen) {
        exp->gen_rvalue(gen);
        for (const auto &s : indexes) {
            s->gen_rvalue(gen);
            gen.emit(s.get(), BINARY_SUBSCR);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_member_index_t::set_parent(js_sym_t::ref node) {
        exp->set_parent(shared_from_this());
        for (const auto &s : indexes) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_exp_seq_t::get_type() const {
        return s_expression_seq;
    }

    std::string js_sym_exp_seq_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_exp_seq_t::gen_rvalue(ijsgen &gen) {
        size_t i = 0;
        for (const auto &s : exps) {
            s->gen_rvalue(gen);
            if (i + 1 < exps.size())
                gen.emit(s.get(), POP_TOP);
            i++;
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_exp_seq_t::set_parent(js_sym_t::ref node) {
        for (const auto &s : exps) {
            s->set_parent(node);
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_array_t::get_type() const {
        return s_array;
    }

    std::string js_sym_array_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_array_t::gen_rvalue(ijsgen &gen) {
        if (rests.empty()) {
            for (const auto &s : exps) {
                if (s)
                    s->gen_rvalue(gen);
                else
                    gen.emit(nullptr, LOAD_EMPTY);
            }
            gen.emit(this, BUILD_LIST, exps.size());
        } else {
            size_t i = 0;
            auto j = 0;
            gen.emit(this, REST_ARGUMENT);
            for (const auto &s : exps) {
                if (s)
                    s->gen_rvalue(gen);
                else
                    gen.emit(nullptr, LOAD_EMPTY);
                if (i < rests.size() && rests[i] == j) {
                    gen.emit(s.get(), UNPACK_SEQUENCE);
                    i++;
                }
                j++;
            }
            gen.emit(this, BUILD_LIST, -1);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_array_t::set_parent(js_sym_t::ref node) {
        for (const auto &s : exps) {
            if (s)
                s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_object_pair_t::get_type() const {
        return s_object_pair;
    }

    js_symbol_t js_sym_object_pair_t::get_base_type() const {
        return s_object_pair;
    }

    std::string js_sym_object_pair_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_object_pair_t::gen_rvalue(ijsgen &gen) {
        key->gen_rvalue(gen);
        value->gen_rvalue(gen);
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_object_pair_t::set_parent(js_sym_t::ref node) {
        key->set_parent(shared_from_this());
        value->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_new_t::get_type() const {
        return s_new;
    }

    std::string js_sym_new_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_new_t::gen_rvalue(ijsgen &gen) {
        obj->gen_rvalue(gen);
        if (rests.empty()) {
            for (const auto &s : args) {
                s->gen_rvalue(gen);
            }
            gen.emit(this, CALL_FUNCTION_EX, args.size());
        } else {
            size_t i = 0;
            auto j = 0;
            gen.emit(this, REST_ARGUMENT);
            for (const auto &s : args) {
                s->gen_rvalue(gen);
                if (i < rests.size() && rests[i] == j) {
                    gen.emit(s.get(), UNPACK_SEQUENCE);
                    i++;
                }
                j++;
            }
            gen.emit(this, CALL_FUNCTION_EX, -1);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_new_t::set_parent(js_sym_t::ref node) {
        obj->set_parent(shared_from_this());
        for (const auto &s : args) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_object_t::get_type() const {
        return s_object;
    }

    std::string js_sym_object_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_object_t::gen_rvalue(ijsgen &gen) {
        if (rests.empty()) {
            for (auto k = pairs.rbegin(); k != pairs.rend(); k++) {
                (*k)->gen_rvalue(gen);
            }
            gen.emit(this, BUILD_MAP, pairs.size());
        } else {
            gen.emit(this, REST_ARGUMENT);
            auto i = ((int)pairs.size()) - 1, j = ((int)rests.size()) - 1;
            for (auto k = is_pair.rbegin(); k != is_pair.rend(); k++) {
                if (*k)
                    pairs[i--]->gen_rvalue(gen);
                else {
                    rests[j--]->gen_rvalue(gen);
                    gen.emit(this, UNPACK_EX);
                }
            }
            gen.emit(this, BUILD_MAP, -1);
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_object_t::set_parent(js_sym_t::ref node) {
        for (const auto &s : pairs) {
            s->set_parent(shared_from_this());
        }
        for (const auto &s : rests) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_call_method_t::get_type() const {
        return s_call_method;
    }

    std::string js_sym_call_method_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_call_method_t::gen_rvalue(ijsgen &gen) {
        obj->gen_rvalue(gen);
        auto desc = gen.get_code_text(this);
        gen.emit(method, LOAD_METHOD, gen.load_string(method->data._string, cjs_consts::get_string_t::gs_name));
        if (rests.empty()) {
            for (const auto &s : args) {
                s->gen_rvalue(gen);
            }
            gen.emit(this, CALL_METHOD, args.size(), gen.load_string(desc, cjs_consts::gs_debug));
        } else {
            size_t i = 0;
            auto j = 0;
            gen.emit(this, REST_ARGUMENT);
            for (const auto &s : args) {
                s->gen_rvalue(gen);
                if (i < rests.size() && rests[i] == j) {
                    gen.emit(s.get(), UNPACK_SEQUENCE);
                    i++;
                }
                j++;
            }
            gen.emit(this, CALL_METHOD, -1, gen.load_string(desc, cjs_consts::gs_debug));
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_call_method_t::set_parent(js_sym_t::ref node) {
        obj->set_parent(shared_from_this());
        for (const auto &s : args) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_call_function_t::get_type() const {
        return s_call_function;
    }

    std::string js_sym_call_function_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_call_function_t::gen_rvalue(ijsgen &gen) {
        obj->gen_rvalue(gen);
        auto desc = gen.get_code_text(this);
        if (rests.empty()) {
            for (const auto &s : args) {
                s->gen_rvalue(gen);
            }
            gen.emit(this, CALL_FUNCTION, args.size(), gen.load_string(desc, cjs_consts::gs_debug));
        } else {
            size_t i = 0;
            auto j = 0;
            gen.emit(this, REST_ARGUMENT);
            for (const auto &s : args) {
                s->gen_rvalue(gen);
                if (i < rests.size() && rests[i] == j) {
                    gen.emit(s.get(), UNPACK_SEQUENCE);
                    i++;
                }
                j++;
            }
            gen.emit(this, CALL_FUNCTION, -1, gen.load_string(desc, cjs_consts::gs_debug));
        }
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_call_function_t::set_parent(js_sym_t::ref node) {
        obj->set_parent(shared_from_this());
        for (const auto &s : args) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_t::get_type() const {
        return s_statement;
    }

    js_symbol_t js_sym_stmt_t::get_base_type() const {
        return s_statement;
    }

    std::string js_sym_stmt_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_stmt_t::gen_rvalue(ijsgen &gen) {
        return js_sym_t::gen_rvalue(gen);
    }

    // ----

    js_symbol_t js_sym_stmt_var_t::get_type() const {
        return s_statement_var;
    }

    std::string js_sym_stmt_var_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_var_t::gen_rvalue(ijsgen &gen) {
        for (const auto &s : vars) {
            s->gen_rvalue(gen);
            gen.emit(s.get(), POP_TOP);
        }
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_var_t::gen_rvalue_decl(ijsgen& gen)
    {
        for (const auto& s : vars) {
            s->gen_rvalue_decl(gen);
            gen.emit(s.get(), POP_TOP);
        }
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_var_t::set_parent(js_sym_t::ref node) {
        for (const auto &s : vars) {
            s->set_parent(shared_from_this());
        }
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_exp_t::get_type() const {
        return s_statement_exp;
    }

    std::string js_sym_stmt_exp_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_exp_t::gen_rvalue(ijsgen &gen) {
        seq->gen_rvalue(gen);
        gen.emit(this, POP_TOP);
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_exp_t::set_parent(js_sym_t::ref node) {
        seq->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_return_t::get_type() const {
        return s_statement_return;
    }

    std::string js_sym_stmt_return_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_return_t::gen_rvalue(ijsgen &gen) {
        gen.gen_try(K_RETURN);
        if (seq)
            seq->gen_rvalue(gen);
        else
            gen.emit(nullptr, LOAD_UNDEFINED);
        gen.emit(this, RETURN_VALUE);
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_return_t::set_parent(js_sym_t::ref node) {
        if (seq)
            seq->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_throw_t::get_type() const {
        return s_statement_throw;
    }

    std::string js_sym_stmt_throw_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_throw_t::gen_rvalue(ijsgen &gen) {
        if (seq)
            seq->gen_rvalue(gen);
        else
            gen.emit(nullptr, LOAD_UNDEFINED);
        gen.emit(this, THROW);
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_throw_t::set_parent(js_sym_t::ref node) {
        if (seq)
            seq->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_control_t::get_type() const {
        return s_statement_control;
    }

    std::string js_sym_stmt_control_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_control_t::gen_rvalue(ijsgen &gen) {
        gen.gen_try(keyword);
        gen.push_rewrites(gen.code_length(), keyword);
        gen.emit(this, JUMP_ABSOLUTE, 0);
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_control_t::set_parent(js_sym_t::ref node) {
        if (label)
            label->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_if_t::get_type() const {
        return s_statement_if;
    }

    std::string js_sym_stmt_if_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_if_t::gen_rvalue(ijsgen &gen) {
        seq->gen_rvalue(gen);
        auto idx = gen.code_length();
        gen.emit(this, POP_JUMP_IF_FALSE, 0);
        true_stmt->gen_rvalue(gen);
        if (false_stmt) {
            auto idx2 = gen.code_length();
            gen.emit(nullptr, JUMP_FORWARD, 0);
            gen.edit(idx, 1, gen.code_length());
            false_stmt->gen_rvalue(gen);
            gen.edit(idx2, 1, gen.code_length() - idx2);
        } else {
            gen.edit(idx, 1, gen.code_length());
        }
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_if_t::set_parent(js_sym_t::ref node) {
        seq->set_parent(shared_from_this());
        true_stmt->set_parent(shared_from_this());
        if (false_stmt)
            false_stmt->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_while_t::get_type() const {
        return s_statement_while;
    }

    std::string js_sym_stmt_while_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_while_t::gen_rvalue(ijsgen &gen) {
        if (do_while) {
            auto idx = gen.code_length();
            stmt->gen_rvalue(gen);
            seq->gen_rvalue(gen);
            gen.emit(this, POP_JUMP_IF_TRUE, idx);
        } else {
            auto idx = gen.code_length();
            seq->gen_rvalue(gen);
            auto idx2 = gen.code_length();
            gen.emit(this, POP_JUMP_IF_FALSE, 0);
            stmt->gen_rvalue(gen);
            gen.emit(nullptr, JUMP_ABSOLUTE, idx);
            gen.edit(idx2, 1, gen.code_length());
        }
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_while_t::set_parent(js_sym_t::ref node) {
        seq->set_parent(shared_from_this());
        stmt->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_for_t::get_type() const {
        return s_statement_for;
    }

    std::string js_sym_stmt_for_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_for_t::gen_rvalue(ijsgen &gen) {
        gen.enter(sp_for);
        if (exp) {
            exp->gen_rvalue(gen);
            gen.emit(exp.get(), POP_TOP);
        } else if (vars)
            vars->gen_rvalue(gen);
        auto L1 = 0;
        auto L2 = gen.code_length(); // cond
        if (cond) {
            cond->gen_rvalue(gen);
            L1 = gen.code_length();
            gen.emit(cond.get(), POP_JUMP_IF_FALSE, 0); // exit
        }
        body->gen_rvalue(gen);
        auto L3 = gen.code_length(); // iter
        if (iter) {
            iter->gen_rvalue(gen);
            gen.emit(iter.get(), POP_TOP);
        }
        gen.emit(nullptr, JUMP_ABSOLUTE, L2);
        if (cond)
            gen.edit(L1, 1, gen.code_length());
        auto j_break = gen.code_length();
        auto j_continue = L3;
        const auto &re = gen.get_rewrites();
        for (const auto &s : re) {
            if (s.second == K_BREAK)
                gen.edit(s.first, 1, j_break);
            else if (s.second == K_CONTINUE)
                gen.edit(s.first, 1, j_continue);
            else
                gen.error(this, "invalid rewrites");
        }
        gen.leave();
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_for_t::set_parent(js_sym_t::ref node) {
        if (exp)
            exp->set_parent(shared_from_this());
        else if (vars)
            vars->set_parent(shared_from_this());
        if (cond)
            cond->set_parent(shared_from_this());
        if (iter)
            iter->set_parent(shared_from_this());
        body->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_for_in_t::get_type() const {
        return s_statement_for_in;
    }

    std::string js_sym_stmt_for_in_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_for_in_t::gen_rvalue(ijsgen &gen) {
        gen.enter(sp_for_each);
        iter->gen_rvalue(gen);
        gen.emit(iter.get(), GET_ITER);
        auto idx_exit = gen.code_length();
        gen.emit(iter.get(), FOR_ITER, 0);
        if (exp)
            exp->gen_lvalue(gen);
        else
            vars->gen_lvalue(gen);
        gen.emit(nullptr, POP_TOP);
        body->gen_rvalue(gen);
        gen.emit(nullptr, JUMP_ABSOLUTE, idx_exit);
        gen.edit(idx_exit, 1, gen.code_length() - idx_exit);
        auto j_break = gen.code_length();
        auto j_continue = idx_exit;
        const auto &re = gen.get_rewrites();
        for (const auto &s : re) {
            if (s.second == K_BREAK)
                gen.edit(s.first, 1, j_break);
            else if (s.second == K_CONTINUE)
                gen.edit(s.first, 1, j_continue);
            else
                gen.error(this, "invalid rewrites");
        }
        gen.leave();
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_for_in_t::set_parent(js_sym_t::ref node) {
        if (exp)
            exp->set_parent(shared_from_this());
        else
            vars->set_parent(shared_from_this());
        iter->set_parent(shared_from_this());
        body->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_case_t::get_type() const {
        return s_case;
    }

    std::string js_sym_case_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_case_t::gen_rvalue(ijsgen &gen) {
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_case_t::set_parent(js_sym_t::ref node) {
        if (exp)
            exp->set_parent(shared_from_this());
        for (const auto &s : stmts) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_switch_t::get_type() const {
        return s_statement_switch;
    }

    std::string js_sym_stmt_switch_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_switch_t::gen_rvalue(ijsgen &gen) {
        gen.enter(sp_switch);
        exp->gen_rvalue(gen);
        size_t i = 0;
        auto L = 0;
        auto def = false;
        for (const auto &s : cases) {
            if (s->exp) {
                gen.emit(nullptr, DUP_TOP);
                s->exp->gen_rvalue(gen);
                gen.emit(this, COMPARE_FEQUAL);
                auto idx = gen.code_length();
                gen.emit(this, POP_JUMP_IF_FALSE, 0);
                if (L > 0)
                    gen.edit(L, 1, gen.code_length() - L);
                for (const auto& s : s->stmts) {
                    s->gen_rvalue(gen);
                }
                if (i + 1 < cases.size()) {
                    L = gen.code_length();
                    gen.emit(nullptr, JUMP_FORWARD, 0);
                }
                gen.edit(idx, 1, gen.code_length());
            }
            else {
                def = true;
                gen.emit(nullptr, POP_TOP);
                if (L > 0)
                    gen.edit(L, 1, gen.code_length());
                for (const auto& s : s->stmts) {
                    s->gen_rvalue(gen);
                }
                if (i + 1 < cases.size()) {
                    L = gen.code_length();
                    gen.emit(nullptr, JUMP_FORWARD, 0);
                }
            }
        }
        if (!def)
            gen.emit(nullptr, POP_TOP);
        auto j_break = gen.code_length();
        const auto &re = gen.get_rewrites();
        for (const auto &s : re) {
            if (s.second == K_BREAK)
                gen.edit(s.first, 1, j_break);
            else if (s.second == K_CONTINUE)
                gen.error(this, "invalid continue in switch");
            else
                gen.error(this, "invalid rewrites");
        }
        gen.leave();
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_switch_t::set_parent(js_sym_t::ref node) {
        for (const auto &s : cases) {
            s->set_parent(shared_from_this());
        }
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_stmt_try_t::get_type() const {
        return s_statement_try;
    }

    std::string js_sym_stmt_try_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_stmt_try_t::gen_rvalue(ijsgen &gen) {
        auto L1 = gen.code_length();
        gen.emit(finally_body.get(), SETUP_FINALLY, 0, 0); // finally
        // TRY
        gen.enter(sp_try, finally_body);
        try_body->gen_rvalue(gen);
        gen.leave();
        gen.emit(nullptr, EXIT_FINALLY);
        if (catch_body) {
            auto L2 = gen.code_length();
            gen.emit(nullptr, JUMP_FORWARD, 0);
            gen.edit(L1, 1, gen.code_length() - L1);
            gen.enter(sp_catch, finally_body);
            if (var) {
                gen.add_var(var->node->data._identifier, var);
                var->gen_lvalue(gen);
            }
            gen.emit(nullptr, POP_TOP);
            // CATCH
            catch_body->gen_rvalue(gen);
            gen.leave();
            gen.edit(L2, 1, gen.code_length() - L2);
            if (finally_body)
                gen.emit(nullptr, EXIT_FINALLY);
        }
        // FINALLY
        if (finally_body) {
            gen.edit(L1, 2, gen.code_length() - L1);
            finally_body->gen_rvalue(gen);
        }
        return js_sym_stmt_t::gen_rvalue(gen);
    }

    int js_sym_stmt_try_t::set_parent(js_sym_t::ref node) {
        try_body->set_parent(shared_from_this());
        if (var)
            var->set_parent(shared_from_this());
        if (catch_body)
            catch_body->set_parent(shared_from_this());
        if (finally_body)
            finally_body->set_parent(shared_from_this());
        return js_sym_stmt_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_block_t::get_type() const {
        return s_block;
    }

    std::string js_sym_block_t::to_string() const {
        return js_sym_stmt_t::to_string();
    }

    int js_sym_block_t::gen_rvalue(ijsgen &gen) {
        gen.enter(sp_block);
        decltype(stmts) func_stmts;
        decltype(stmts) var_decls;
        decltype(stmts) others;
        for (const auto &s : stmts) {
            if (s->get_type() == s_statement_exp) {
                auto exp = std::dynamic_pointer_cast<js_sym_stmt_exp_t>(s);
                const auto c = exp->seq->exps;
                if (c.size() == 1) {
                    if (c.front()->get_type() == s_code) {
                        func_stmts.push_back(s);
                        continue;
                    }
                }
            }
            else if (s->get_type() == s_statement_var) {
                var_decls.push_back(s);
            }
            others.push_back(s);
        }
        for (const auto& s : var_decls) {
            std::dynamic_pointer_cast<js_sym_stmt_var_t>(s)->gen_rvalue_decl(gen);
        }
        for (const auto& s : func_stmts) {
            s->gen_rvalue(gen);
        }
        for (const auto &s : others) {
            s->gen_rvalue(gen);
        }
        gen.leave();
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_block_t::set_parent(js_sym_t::ref node) {
        for (const auto &s : stmts) {
            s->set_parent(shared_from_this());
        }
        return js_sym_t::set_parent(node);
    }

    // ----

    js_symbol_t js_sym_code_t::get_type() const {
        return s_code;
    }

    std::string js_sym_code_t::to_string() const {
        return js_sym_t::to_string();
    }

    int js_sym_code_t::gen_rvalue(ijsgen &gen) {
        fullName = gen.get_fullname(name ? name->data._identifier : LAMBDA_ID);
        simpleName = name ? name->data._identifier : LAMBDA_ID;
        auto p = parent.lock();
        if (p) {
            if (p->get_type() == s_id) {
                auto _id = std::dynamic_pointer_cast<js_sym_id_t>(p);
                if (!_id->ids.empty()) {
                    std::stringstream ss2;
                    auto idx = _id->ids.back().get();
                    ss2 << "(" << gen.get_filename() << ":" << idx->line << ":" << idx->column
                        << ") " << gen.get_code_text(idx);
                    debugName = ss2.str();
                }
            } else if (p->get_type() == s_binop) {
                do {
                    auto _binop = std::dynamic_pointer_cast<js_sym_binop_t>(p);
                    if (_binop->op->flag == a_operator && _binop->op->data._op == T_ASSIGN) {
                        if (_binop->exp1->get_type() == s_member_dot) {
                            auto _dot = std::dynamic_pointer_cast<js_sym_member_dot_t>(_binop->exp1);
                            if (gen.get_code_text(_dot->exp.get()) == "this" && _dot->dots.size() == 1) {
                                std::stringstream ss2;
                                auto idx = _dot->exp.get();
                                ss2 << "(" << gen.get_filename() << ":" << idx->line << ":" << idx->column
                                    << ") " << gen.get_func_name() << ".prototype." << gen.get_code_text(_dot->dots.front());
                                debugName = ss2.str();
                                break;
                            }
                        }
                        std::stringstream ss3;
                        auto idx = _binop->exp1.get();
                        ss3 << "(" << gen.get_filename() << ":" << idx->line << ":" << idx->column
                            << ") " << fullName << " " << gen.get_code_text(idx);
                        debugName = ss3.str();
                    }
                } while (false);
            }
        }
        if (debugName.empty()) {
            std::stringstream ss;
            ss << "(" << gen.get_filename() << ":" << this->line << ":" << this->column
               << ") " << fullName;
            debugName = ss.str();
        }
        if (name)
            gen.add_var(simpleName, shared_from_this());
        if (!args.empty())
            gen.enter(sp_param);
        auto id = gen.push_function(std::dynamic_pointer_cast<js_sym_code_t>(shared_from_this()));
        if (body) {
            if (!arrow && name) {
                gen.enter(sp_block);
                gen.add_var(simpleName, shared_from_this());
            }
            body->gen_rvalue(gen);
            if (body->get_base_type() == s_expression)
                gen.emit(nullptr, RETURN_VALUE);
            auto l = gen.code_length();
            if (l > 0 && gen.get_ins(l - 1) != RETURN_VALUE) {
                gen.emit(nullptr, RETURN_VALUE);
            }
            if (!arrow && name) {
                gen.leave();
            }
        }
        gen.pop_function();
        uint32_t flag = 0;
        if (!closure.empty()) {
            for (const auto &s : closure) {
                gen.emit(s->node, LOAD_CLOSURE, gen.load_string(
                        s->node->data._identifier, cjs_consts::get_string_t::gs_name));
            }
            flag |= 8U;
            gen.emit(nullptr, BUILD_MAP, closure.size());
        }
        if (name) {
            gen.emit(name, LOAD_CONST, id);
            gen.emit(name, LOAD_CONST, gen.load_string(debugName, cjs_consts::get_string_t::gs_string));
            gen.emit(this, MAKE_FUNCTION, (int) flag);
            if (parent.lock()->get_type() == s_statement_exp) {
                if (gen.get_func_level() == 1)
                    gen.emit(name, STORE_GLOBAL, gen.load_string(name->data._identifier, cjs_consts::get_string_t::gs_global));
                else
                    gen.emit(name, STORE_NAME, gen.load_string(name->data._identifier, cjs_consts::get_string_t::gs_name));
            }
        } else {
            gen.emit(nullptr, LOAD_CONST, id);
            gen.emit(nullptr, LOAD_CONST, gen.load_string(debugName, cjs_consts::get_string_t::gs_string));
            gen.emit(this, MAKE_FUNCTION, (int) flag);
        }
        if (!args.empty())
            gen.leave();
        consts.save();
        return js_sym_t::gen_rvalue(gen);
    }

    int js_sym_code_t::set_parent(js_sym_t::ref node) {
        if (body)
            body->set_parent(shared_from_this());
        return js_sym_t::set_parent(node);
    }

    void js_sym_code_t::to_json(nlohmann::json& j) const
    {
        j = nlohmann::json{
            {"arrow", arrow},
            {"rest", rest},
            {"fullName", fullName},
            {"simpleName", simpleName},
            {"debugName", debugName},
            {"text", text},
            {"args", args_str},
            {"consts", consts},
            {"codes", codes},
            {"closure", closure_str},
        };
    }

    void js_sym_code_t::from_json(const nlohmann::json& j)
    {
        j.at("arrow").get_to(arrow);
        j.at("rest").get_to(rest);
        j.at("fullName").get_to(fullName);
        j.at("simpleName").get_to(simpleName);
        j.at("debugName").get_to(debugName);
        j.at("text").get_to(text);
        j.at("args").get_to(args_str);
        j.at("consts").get_to(consts);
        j.at("codes").get_to(codes);
        j.at("closure").get_to(closure_str);
    }
}