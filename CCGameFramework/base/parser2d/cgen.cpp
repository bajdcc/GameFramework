//
// Project: clibparser
// Created by bajdcc
//

#include "stdafx.h"
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <iomanip>
#include "cgen.h"
#include "cast.h"
#include "cvm.h"
#include "cexception.h"

#define REPORT_GEN 0
#define REPORT_GEN_FILE "gen.log"

#define LOG_TYPE 0

#define AST_IS_KEYWORD(node) ((node)->flag == ast_keyword)
#define AST_IS_KEYWORD_K(node, k) ((node)->data._keyword == (k))
#define AST_IS_KEYWORD_N(node, k) (AST_IS_KEYWORD(node) && AST_IS_KEYWORD_K(node, k))
#define AST_IS_OP(node) ((node)->flag == ast_operator)
#define AST_IS_OP_K(node, k) ((node)->data._op == (k))
#define AST_IS_OP_N(node, k) (AST_IS_OP(node) && AST_IS_OP_K(node, k))
#define AST_IS_ID(node) ((node)->flag == ast_literal)
#define AST_IS_COLL(node) ((node)->flag == ast_collection)
#define AST_IS_COLL_K(node, k) ((node)->data._coll == (k))
#define AST_IS_COLL_N(node, k) (AST_IS_COLL(node) && AST_IS_COLL_K(node, k))

namespace clib {

    std::stringstream cgen::log_out;

    static string_t sym_to_string(const sym_t::ref& sym) {
        if (sym)
            return sym->to_string();
        return "none";
    }

    template<class T>
    std::string string_join(const std::vector<T>& v, const string_t& sep) {
        if (v.empty())
            return "";
        std::stringstream ss;
        ss << v[0]->to_string();
        for (size_t i = 1; i < v.size(); ++i) {
            ss << sep;
            ss << v[i]->to_string();
        }
        return std::move(ss.str());
    }

    sym_list_t::ref cgen::exp_list(const std::vector<sym_t::ref>& exps) {
        auto list = std::make_shared<sym_list_t>();
        for (auto& exp : exps) {
            if (exp->get_base_type() != s_expression)
                error(exp, "need expression: " + exp->to_string());
            list->exps.push_back(std::dynamic_pointer_cast<type_exp_t>(exp));
        }
        return list;
    }

    static int align4(int n) {
        if (n % 4 == 0)
            return n;
        return (n | 3) + 1;
    }

    static ast_node * ast_get_parent(ast_node * node, int level) {
        for (int i = 0; i < level; ++i) {
            if (AST_IS_COLL_N(node, c_program)) {
                break;
            }
            node = node->parent;
        }
        return node;
    }

    static int cast_size(cast_t type) {
        switch (type) {
        case t_char:
        case t_uchar:
        case t_short:
        case t_ushort:
        case t_int:
        case t_uint:
        case t_float:
        case t_ptr:
            return 4;
        case t_long:
        case t_ulong:
        case t_double:
            return 8;
        }
        assert(!"invalid size");
        return -1;
    }

    static const char* cast_str(cast_t type) {
        switch (type) {
#define CAST_STR(n) case t_##n: return #n;
            CAST_STR(char)
                CAST_STR(uchar)
                CAST_STR(short)
                CAST_STR(ushort)
                CAST_STR(int)
                CAST_STR(uint)
                CAST_STR(float)
                CAST_STR(ptr)
                CAST_STR(long)
                CAST_STR(ulong)
                CAST_STR(double)
                CAST_STR(struct)
                CAST_STR(error)
#undef CAST_STR
        }
        return "error";
    }

    int sym_t::size(sym_size_t t, int level) const {
        assert(!"invalid size");
        return 0;
    }

    symbol_t sym_t::get_type() const {
        return s_sym;
    }

    symbol_t sym_t::get_base_type() const {
        return s_sym;
    }

    string_t sym_t::get_name() const {
        return "[Unknown]";
    }

    string_t sym_t::to_string() const {
        return "[Symbol]";
    }

    gen_t sym_t::gen_lvalue(igen & gen) {
        gen.error(line, column, "unsupport lvalue: " + to_string());
        return g_ok;
    }

    gen_t sym_t::gen_rvalue(igen & gen) {
        gen.error(line, column, "unsupport rvalue: " + to_string());
        return g_ok;
    }

    gen_t sym_t::gen_invoke(igen & gen, ref & list) {
        gen.error(line, column, "unsupport invoke");
        return g_error;
    }

    cast_t sym_t::get_cast() const {
        return t_error;
    }

    type_t::type_t(int ptr) : ptr(ptr), matrix(0) {}

    symbol_t type_t::get_type() const {
        return s_type;
    }

    symbol_t type_t::get_base_type() const {
        return s_type;
    }

    type_t::ref type_t::clone() const {
        assert(!"invalid clone");
        return nullptr;
    }

    type_base_t::type_base_t(lexer_t type, int ptr) : type_t(ptr), type(type) {}

    int type_base_t::size(sym_size_t t, int level) const {
        if (t == x_inc) {
            if (ptr == 0)
                return 1;
            if (ptr == 1)
                return LEX_SIZE(type);
            return sizeof(void*);
        }
        if (t == x_matrix) {
            // TODO: Fix bug, ID Assignment L-Value
            return matrix.size();
        }
        if (t == x_load) {
            if (ptr > 0)
                return sizeof(void*);
            return LEX_SIZE(type);
        }
        if (!matrix.empty()) {
            auto s = 1;
            for (auto& m : matrix) {
                s *= m;
            }
            if (ptr - matrix.size() > 0)
                return sizeof(void*) * s;
            return LEX_SIZE(type) * s;
        }
        else {
            if (ptr > 0)
                return sizeof(void*);
            return LEX_SIZE(type);
        }
    }

    symbol_t type_base_t::get_type() const {
        return s_type_base;
    }

    string_t type_base_t::get_name() const {
        return LEX_STRING(type);
    }

    string_t type_base_t::to_string() const {
        std::stringstream ss;
        ss << LEX_STRING(type);
        if (!matrix.empty()) {
            for (int i = matrix.size(); i < ptr; ++i) {
                ss << '*';
            }
            for (auto& m : matrix) {
                ss << "[" << m << "]";
            }
        }
        else {
            for (int i = 0; i < ptr; ++i) {
                ss << '*';
            }
        }
        return ss.str();
    }

    static cast_t lexer2cast(lexer_t type) {
        if (type == l_string)
            return t_ptr;
        int t = type - 2;
        assert(t >= t_char && t < t_error);
        return (cast_t)t;
    }

    cast_t type_base_t::get_cast() const {
        return ptr > 0 ? t_ptr : lexer2cast(type);
    }

    type_t::ref type_base_t::clone() const {
        auto obj = std::make_shared<type_base_t>(type, ptr);
        obj->_static = _static;
        if (!matrix.empty())
            obj->matrix = matrix;
        return obj;
    }

    type_typedef_t::type_typedef_t(const sym_t::ref & refer, int ptr) : type_t(ptr), refer(refer) {}

    symbol_t type_typedef_t::get_type() const {
        return s_type_typedef;
    }

    int type_typedef_t::size(sym_size_t t, int level) const {
        if (t == x_inc) {
            if (ptr > level) {
                return sizeof(void*);
            }
            return refer.lock()->size(t, level);
        }
        if (ptr > 0)
            return sizeof(void*);
        return refer.lock()->size(t, level);
    }

    string_t type_typedef_t::to_string() const {
        std::stringstream ss;
        ss << refer.lock()->get_name();
        for (int i = 0; i < ptr; ++i) {
            ss << '*';
        }
        return ss.str();
    }

    cast_t type_typedef_t::get_cast() const {
        return ptr > 0 ? t_ptr : refer.lock()->get_cast();
    }

    type_t::ref type_typedef_t::clone() const {
        auto obj = std::make_shared<type_typedef_t>(refer.lock(), ptr);
        obj->_static = _static;
        if (!matrix.empty())
            obj->matrix = matrix;
        return obj;
    }

    sym_id_t::sym_id_t(const type_t::ref & base, const string_t & id)
        : base(base), id(id) {
        line = base->line;
        column = base->column;
    }

    symbol_t sym_id_t::get_type() const {
        return s_id;
    }

    int sym_id_t::size(sym_size_t t, int level) const {
        return base->size(t, level);
    }

    symbol_t sym_id_t::get_base_type() const {
        return s_id;
    }

    string_t sym_id_t::get_name() const {
        return id;
    }

    string_t sym_id_t::to_string() const {
        std::stringstream ss;
        ss << base->to_string() << " " << id << ", ";
        ss << "Class: " << sym_class_string(clazz) << ", ";
        if (init)
            ss << "Init: " << init->to_string() << ", ";
        ss << "Addr: " << addr;
        return ss.str();
    }

    gen_t sym_id_t::gen_lvalue(igen & gen) {
        if (clazz == z_global_var) {
            // gen.error(line, column, "global id cannot be modified");
            gen.emit(IMM, DATA_BASE | addr);
            return g_no_load;
        }
        else if (clazz == z_local_var) {
            gen.emit(LEA, addr);
        }
        else if (clazz == z_param_var) {
            gen.emit(LEA, addr);
        }
        else if (clazz == z_struct_var) {
            gen.emit(IMM, addr);
        }
        else if (clazz == z_function) {
            gen.error(line, column, "function id cannot be modified");
        }
        return g_ok;
    }

    gen_t sym_id_t::gen_rvalue(igen & gen) {
        if (clazz == z_global_var) {
            gen.emit(IMM, DATA_BASE | addr);
            if (base->ptr == 0)
                gen.emit(LOAD, base->size(x_load));
        }
        else if (clazz == z_local_var) {
            gen.emit(LEA, addr);
            gen.emit(LOAD, base->size(x_load));
        }
        else if (clazz == z_param_var) {
            gen.emit(LEA, addr);
            gen.emit(LOAD, base->size(x_load));
        }
        else if (clazz == z_struct_var) {
            gen.error(line, column, "not implemented");
        }
        else if (clazz == z_function) {
            gen.emit(IMM, USER_BASE | addr);
        }
        return g_ok;
    }

    cast_t sym_id_t::get_cast() const {
        return base->get_cast();
    }

    sym_struct_t::sym_struct_t(bool _struct, const string_t & id) : _struct(_struct), id(id) {}

    symbol_t sym_struct_t::get_type() const {
        return s_struct;
    }

    symbol_t sym_struct_t::get_base_type() const {
        return s_struct;
    }

    int sym_struct_t::size(sym_size_t t, int level) const {
        if (_size == 0) {
            if (_struct) {
                for (auto& decl : decls) {
                    decl->addr = _size;
                    auto size = decl->size(t, level);
                    if ((size & 3) != 0)
                        size += 4 - (size & 3);
                    *const_cast<int*>(&_size) += size;
                    decl->addr_end = _size;
                }
            }
            else {
                for (auto& decl : decls) {
                    decl->addr = 0;
                    auto s = decl->size(t, level);
                    *const_cast<int*>(&_size) = max(_size, s);
                    decl->addr_end = s;
                }
            }
        }
        return _size;
    }

    string_t sym_struct_t::get_name() const {
        return id;
    }

    string_t sym_struct_t::to_string() const {
        std::stringstream ss;
        ss << "struct " << id << ", ";
        ss << "decls: [" << string_join(decls, "; ") << "], ";
        return ss.str();
    }

    cast_t sym_struct_t::get_cast() const {
        return t_struct;
    }

    sym_func_t::sym_func_t(const type_t::ref & base, const string_t & id) : sym_id_t(base, id) {
        line = base->line;
        column = base->column;
    }

    symbol_t sym_func_t::get_type() const {
        return s_function;
    }

    symbol_t sym_func_t::get_base_type() const {
        return s_function;
    }

    int sym_func_t::size(sym_size_t t, int level) const {
        if (t == x_inc)
            return 0;
        return sizeof(void*);
    }

    string_t sym_func_t::to_string() const {
        std::stringstream ss;
        ss << base->to_string() << " " << id << ", ";
        ss << "Param: [" << string_join(params, "; ") << "], ";
        ss << "Class: " << sym_class_string(clazz) << ", ";
        ss << "Addr: " << addr;
        return ss.str();
    }

    int cast_find(cast_t src, cast_t dst) {
        if (src == t_error)
            return -1;
        if (dst == t_error)
            return -1;
        /*
         * 按照编号分别为：
         * char     0
         * uchar    1
         * short    2
         * ushort   3
         * int      4
         * uint     5
         * long     6
         * ulong    7
         * float    8
         * double   9
         * ptr      10
         * struct   11
         * 操作：
         * [-1] 出错
         * [00] 不转换
         * [01] 4B-4B 有符号转无符号
         * [02] 4B-4B 无符号转有符号
         * [03] 4B-8B 有符号转无符号
         * [04] 4B-8B 无符号转有符号
         * [05] 8B-4B 有符号转无符号
         * [06] 8B-4B 无符号转有符号
         * [07] 8B-8B 有符号转无符号
         * [08] 8B-8B 无符号转有符号
         * [09] 4B-8B 有符号转有符号
         * [10] 4B-8B 无符号转无符号
         * [11] 8B-4B 有符号转有符号
         * [12] 8B-4B 无符号转无符号
         * [13] 1B-8B 有符号扩展
         * [14] 2B-8B 有符号扩展
         * [20] 4B-4B 无符号转float
         * [21] 4B-4B 有符号转float
         * [22] 8B-4B 无符号转float
         * [23] 8B-4B 有符号转float
         * [24] 4B-4B float转无符号
         * [25] 4B-4B float转有符号
         * [26] 4B-8B float转无符号
         * [27] 4B-8B float转有符号
         * [28] 4B-8B float转double
         * [30] 4B-8B 无符号转double
         * [31] 4B-8B 有符号转double
         * [32] 8B-8B 无符号转double
         * [33] 8B-8B 有符号转double
         * [34] 8B-4B double转无符号
         * [35] 8B-4B double转有符号
         * [36] 8B-8B double转无符号
         * [37] 8B-8B double转有符号
         * [38] 8B-4B double转float
         */
        static int _cast[][12] = { // 转换矩阵
            /* [SRC] [DST]  C   UC  S   US  I   UI  L   UL  F   D   P   T */
            /* char    */ { 0,  1,  13, 1,  13, 1,  13, 3,  21, 31, 1,  -1},
            /* uchar   */ { 2,  0,  2,  0,  2,  0,  4,  10, 20, 30, 0,  -1},
            /* short   */ { 0,  1,  0,  1,  14, 1,  14, 3,  21, 31, 1,  -1},
            /* ushort  */ { 2,  0,  2,  0,  2,  0,  4,  10, 20, 30, 0,  -1},
            /* int     */ { 0,  1,  0,  1,  0,  1,  9,  3,  21, 31, 1,  -1},
            /* uint    */ { 2,  0,  2,  0,  2,  0,  4,  10, 20, 30, 0,  -1},
            /* long    */ { 11, 5,  11, 5,  11, 5,  0,  7,  23, 33, 5,  -1},
            /* ulong   */ { 6,  12, 6,  12, 6,  12, 8,  0,  22, 32, 12, -1},
            /* float   */ { 25, 24, 25, 24, 25, 24, 27, 26, 0,  28, -1, -1},
            /* double  */ { 35, 34, 35, 34, 35, 34, 37, 36, 38, 0,  -1, -1},
            /* ptr     */ { 2,  0,  2,  0,  2,  0,  4,  10, -1, -1, 0,  -1},
            /* struct  */ { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0},
        };
        return _cast[src][dst];
    }

    gen_t sym_func_t::gen_invoke(igen & gen, sym_t::ref & list) {
        assert(list->get_type() == s_list);
        auto args = std::dynamic_pointer_cast<sym_list_t>(list);
        auto & exps = args->exps;
        if (ellipsis) {
            if (exps.size() < params.size()) {
                gen.error(line, column, "invoke: argument size not equal, required: " + to_string() +
                    ", but got: " + args->to_string());
            }
        }
        else if (exps.size() != params.size()) {
            gen.error(line, column, "invoke: argument size not equal, required: " + to_string() +
                ", but got: " + args->to_string());
        }
        auto total_size = 0;
        for (auto i = ((int)exps.size()) - 1; i >= 0; --i) {
            exps[i]->gen_rvalue(gen);
            auto exp_type = exps[i]->base->get_cast();
            int c = 0;
            if (i >= (int)params.size()) {
                c = exp_type == t_struct ? exps[i]->size(x_size) : cast_size(exp_type);
            }
            else {
                auto param_type = params[i]->base->get_cast();
                auto s = cast_find(exp_type, param_type);
                if (s == -1)
                    gen.error(line, column, "invoke: argument unsupported cast, required: " + params[i]->to_string() +
                        ", but got: " + exps[i]->to_string() + ", func: " + to_string());
                if (s != 0) {
                    gen.emit(CAST, s);
                }
                c = param_type == t_struct ? params[i]->size(x_size) : cast_size(param_type);
            }
            gen.emit(PUSH, c);
            total_size += c;
        }
        if (implemented) {
            gen.emit(IMM, addr);
        }
        else {
            gen.emit(IMM, 0);
            write_backs.push_back(gen.current() - 1);
        }
        gen.emit(CALL);
        if (!exps.empty()) {
            gen.emit(ADJ, total_size / 4);
        }
        return g_ok;
    }

    void sym_func_t::gen_labels(igen& gen)
    {
        for (auto& r : labels_writeback) {
            auto name = std::get<1>(r);
            auto f = labels.find(name);
            if (f != labels.end()) {
                gen.edit(std::get<0>(r), f->second);
            }
            else {
                gen.error(std::get<2>(r), std::get<3>(r), "missing label: " + name);
            }
        }
        labels_writeback.clear();
    }

    cast_t sym_func_t::get_cast() const {
        return t_ptr;
    }

    type_exp_t::type_exp_t(const type_t::ref & base) : base(base) {}

    symbol_t type_exp_t::get_type() const {
        return s_expression;
    }

    symbol_t type_exp_t::get_base_type() const {
        return s_expression;
    }

    cast_t type_exp_t::get_cast() const {
        assert(base);
        return base->get_cast();
    }

    int type_exp_t::size(sym_size_t t, int level) const {
        if (!base) return 0;
        return base->size(t, level);
    }

    gen_t type_exp_t::gen_invoke(igen& gen, sym_t::ref& list)
    {
        assert(list->get_type() == s_list);
        auto args = std::dynamic_pointer_cast<sym_list_t>(list);
        auto & exps = args->exps;
        auto total_size = 0;
        for (size_t i = 0; i < exps.size(); ++i) {
            exps[i]->gen_rvalue(gen);
            auto exp_type = exps[i]->base->get_cast();
            auto c = exp_type == t_struct ? exps[i]->size(x_size) : cast_size(exp_type);
            gen.emit(PUSH, c);
            total_size += c;
        }
        gen_rvalue(gen);
        gen.emit(CALL);
        if (!exps.empty()) {
            gen.emit(ADJ, total_size / 4);
        }
        base = std::make_shared<type_base_t>(l_int);
        return g_ok;
    }

    sym_var_t::sym_var_t(const type_t::ref & base, ast_node * node) : type_exp_t(base), node(node) {
        line = node->line;
        column = node->column;
    }

    symbol_t sym_var_t::get_type() const {
        return s_var;
    }

    int sym_var_t::size(sym_size_t t, int level) const {
        return base->size(t, level);
    }

    string_t sym_var_t::get_name() const {
        if (node->flag == ast_literal)
            return node->data._string;
        return to_string();
    }

    string_t sym_var_t::to_string() const {
        std::stringstream ss;
        ss << "(type: " << base->to_string() << ", " << cast::to_string(node) << ')';
        return ss.str();
    }

    gen_t sym_var_t::gen_lvalue(igen & gen) {
        if (node->flag == ast_string) {
            gen.emit(IMM, DATA_BASE | (uint32)gen.load_string(node->data._string));
            base = std::make_shared<type_base_t>(l_char, 1);
            return g_no_load;
        }
        gen.error(line, column, "invalid lvalue: " + to_string());
        return g_error;
    }

    gen_t sym_var_t::gen_rvalue(igen & gen) {
        switch ((ast_t)node->flag) {
        case ast_char:
        case ast_uchar:
        case ast_short:
        case ast_ushort:
        case ast_int:
        case ast_uint:
        case ast_float:
            gen.emit(IMM, node->data._ins._1); // 载入4字节
            break;
        case ast_long:
        case ast_ulong:
        case ast_double:
            gen.emit(IMX, node->data._ins._1, node->data._ins._2); // 载入8字节
            break;
        case ast_string:
            gen.emit(IMM, DATA_BASE | (uint32)gen.load_string(node->data._string));
            break;
        case ast_keyword: {
            if (AST_IS_KEYWORD_K(node, k_true))
                gen.emit(IMM, 1);
            else if (AST_IS_KEYWORD_K(node, k_false))
                gen.emit(IMM, 0);
            else
                gen.error(line, column, "sym_var_t::gen_rvalue unsupported keyword type");
        }
                          break;
        default:
            gen.error(line, column, "sym_var_t::gen_rvalue unsupported type");
            break;
        }
        return g_ok;
    }

    cast_t sym_var_t::get_cast() const {
        return lexer2cast(cast::ast_lexer((ast_t)node->flag));
    }

    sym_var_id_t::sym_var_id_t(const type_t::ref & base, ast_node * node, const sym_t::ref & symbol)
        : sym_var_t(base, node), id(symbol) {}

    symbol_t sym_var_id_t::get_type() const {
        return s_var_id;
    }

    int sym_var_id_t::size(sym_size_t t, int level) const {
        return id.lock()->size(t, level);
    }

    string_t sym_var_id_t::get_name() const {
        return id.lock()->get_name();
    }

    string_t sym_var_id_t::to_string() const {
        return id.lock()->to_string();
    }

    gen_t sym_var_id_t::gen_lvalue(igen & gen) {
        return id.lock()->gen_lvalue(gen);
    }

    gen_t sym_var_id_t::gen_rvalue(igen & gen) {
        return id.lock()->gen_rvalue(gen);
    }

    gen_t sym_var_id_t::gen_invoke(igen & gen, sym_t::ref & list) {
        if (id.lock()->get_type() == s_function)
            return id.lock()->gen_invoke(gen, list);
        assert(list->get_type() == s_list);
        auto args = std::dynamic_pointer_cast<sym_list_t>(list);
        auto & exps = args->exps;
        auto total_size = 0;
        for (auto& exp : exps) {
            exp->gen_rvalue(gen);
            auto exp_type = exp->base->get_cast();
            auto c = exp_type == t_struct ? exp->size(x_size) : cast_size(exp_type);
            gen.emit(PUSH, c);
            total_size += c;
        }
        id.lock()->gen_rvalue(gen);
        gen.emit(CALL);
        if (!exps.empty()) {
            gen.emit(ADJ, total_size / 4);
        }
        return g_ok;
    }

    cast_t sym_var_id_t::get_cast() const {
        return id.lock()->get_cast();
    }

    sym_cast_t::sym_cast_t(const type_exp_t::ref & exp, const type_t::ref & base) : type_exp_t(base), exp(exp) {
        line = base->line;
        column = base->column;
    }

    symbol_t sym_cast_t::get_type() const {
        return s_cast;
    }

    int sym_cast_t::size(sym_size_t t, int level) const {
        return base->size(t, level);
    }

    string_t sym_cast_t::get_name() const {
        return exp->get_name();
    }

    string_t sym_cast_t::to_string() const {
        std::stringstream ss;
        ss << "(cast, " << base->to_string() << ",  " << exp->to_string() << ')';
        return ss.str();
    }

    gen_t sym_cast_t::gen_lvalue(igen & gen) {
        gen.error(line, column, "cast: unsupported lvalue");
        return g_error;
    }

    gen_t sym_cast_t::gen_rvalue(igen & gen) {
        auto r = exp->gen_rvalue(gen);
        auto src = exp->base->get_cast();
        auto dst = base->get_cast();
        if (src == t_error)
            gen.error(line, column, "cast: src error");
        if (dst == t_error)
            gen.error(line, column, "cast: dst error");
        auto s = cast_find(src, dst);
        if (s == -1)
            gen.error(line, column, "cast: unsupported cast");
        if (s != 0)
            gen.emit(CAST, s);
        return r;
    }

    sym_unop_t::sym_unop_t(const type_exp_t::ref & exp, ast_node * op)
        : type_exp_t(nullptr), exp(exp), op(op) {
        line = exp->line;
        column = exp->column;
    }

    symbol_t sym_unop_t::get_type() const {
        return s_unop;
    }

    int sym_unop_t::size(sym_size_t t, int level) const {
        if (t == x_inc)
            return 0;
        if (t == x_load) {
            if (AST_IS_OP_N(op, op_times))
                return exp->size(x_inc, level + 1);
        }
        return exp->size(t, level);
    }

    string_t sym_unop_t::get_name() const {
        return to_string();
    }

    string_t sym_unop_t::to_string() const {
        std::stringstream ss;
        ss << "(unop, " << cast::to_string(op) << ",  " << exp->to_string() << ')';
        return ss.str();
    }

    gen_t sym_unop_t::gen_lvalue(igen & gen) {
        switch (op->data._op) {
        case op_plus:
        case op_minus:
        case op_logical_not:
        case op_bit_not:
        case op_bit_and:
            gen.error(line, column, "[unop] invalid lvalue: " + to_string());
            break;
        case op_plus_plus:
        case op_minus_minus: {
            exp->gen_lvalue(gen);
            gen.emit(PUSH, cast_size(t_ptr));
            gen.emit(PUSH, cast_size(t_ptr));
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            auto size = base->get_cast();
            auto c = cast_size(size);
            gen.emit(PUSH, c);
            auto inc = exp->size(x_inc);
            gen.emit(IMM, max(inc, 1));
            auto t = cast_find(t_int, size);
            if (t > 0)
                gen.emit(CAST, t);
            gen.emit(OP_INS(op->data._op), size);
            gen.emit(SAVE, exp->size(x_load));
            gen.emit(POP, c);
            return g_ok;
        }
        case op_times: // 解引用
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            if (base->get_cast() != t_ptr)
                gen.error(line, column, "[unop] invalid deref: " + to_string());
            base->ptr--;
            break;
        default:
            gen.error(line, column, "[unop] not supported lvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    gen_t sym_unop_t::gen_rvalue(igen & gen) {
        if (AST_IS_KEYWORD_N(op, k_sizeof)) {
            gen.emit(IMM, exp->size(x_size));
            base = std::make_shared<type_base_t>(l_int, 0);
            return g_ok;
        }
        switch (op->data._op) {
        case op_plus:
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            break;
        case op_minus:
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            gen.emit(NEG, base->get_cast());
            break;
        case op_plus_plus:
        case op_minus_minus: {
            exp->gen_lvalue(gen);
            gen.emit(PUSH, cast_size(t_ptr));
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            auto size = base->get_cast();
            auto c = cast_size(size);
            gen.emit(PUSH, c);
            auto inc = exp->size(x_inc);
            gen.emit(IMM, max(inc, 1));
            auto t = cast_find(t_int, size);
            if (t > 0)
                gen.emit(CAST, t);
            gen.emit(OP_INS(op->data._op), size);
            gen.emit(SAVE, exp->size(x_load));
            return g_ok;
        }
        case op_logical_not: {
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            if (base->ptr > 0) {
                base.reset();
                base = std::make_shared<type_base_t>(l_int);
            }
            auto size = base->get_cast();
            gen.emit(LNT, size);
        }
                             break;
        case op_bit_not: {
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            gen.emit(NOT, base->get_cast());
        }
                         break;
        case op_bit_and: // 取地址
            exp->gen_lvalue(gen);
            base = exp->base->clone();
            base->ptr++;
            break;
        case op_times: // 解引用
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            if (base->get_cast() != t_ptr)
                gen.error(line, column, "invalid deref: " + to_string());
            base->ptr--;
            gen.emit(LOAD, max(exp->size(x_inc), 1));
            break;
        default:
            gen.error(line, column, "[unop] not supported rvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    sym_sinop_t::sym_sinop_t(const type_exp_t::ref & exp, ast_node * op)
        : type_exp_t(nullptr), exp(exp), op(op) {
        line = exp->line;
        column = exp->column;
    }

    symbol_t sym_sinop_t::get_type() const {
        return s_sinop;
    }

    int sym_sinop_t::size(sym_size_t t, int level) const {
        return exp->size(t, level);
    }

    string_t sym_sinop_t::get_name() const {
        return to_string();
    }

    string_t sym_sinop_t::to_string() const {
        std::stringstream ss;
        ss << "(sinop, " << cast::to_string(op) << ",  " << exp->to_string() << ')';
        return ss.str();
    }

    gen_t sym_sinop_t::gen_lvalue(igen & gen) {
        switch (op->data._op) {
        case op_plus_plus:
        case op_minus_minus: {
            exp->gen_lvalue(gen);
            gen.emit(PUSH, cast_size(t_ptr));
            gen.emit(PUSH, cast_size(t_ptr));
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            auto size = base->get_cast();
            auto c = cast_size(size);
            gen.emit(PUSH, c);
            auto inc = exp->size(x_inc);
            gen.emit(IMM, max(inc, 1));
            auto t = cast_find(t_int, size);
            if (t > 0)
                gen.emit(CAST, t);
            gen.emit(OP_INS(op->data._op), base->get_cast());
            gen.emit(SAVE, exp->size(x_load));
            gen.emit(POP, c);
        }
                             break;
        default:
            gen.error(line, column, "[sinop] not supported lvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    gen_t sym_sinop_t::gen_rvalue(igen & gen) {
        switch (op->data._op) {
        case op_plus_plus:
        case op_minus_minus: {
            exp->gen_rvalue(gen);
            base = exp->base->clone();
            auto size = base->get_cast();
            auto c = cast_size(size);
            gen.emit(PUSH, c);
            exp->gen_lvalue(gen);
            gen.emit(PUSH, cast_size(t_ptr));
            exp->gen_rvalue(gen);
            gen.emit(PUSH, c);
            auto inc = exp->size(x_inc);
            gen.emit(IMM, max(inc, 1));
            auto t = cast_find(t_int, size);
            if (t > 0)
                gen.emit(CAST, t);
            gen.emit(OP_INS(op->data._op), size);
            gen.emit(SAVE, exp->size(x_load));
            gen.emit(POP, c);
        }
                             break;
        default:
            gen.error(line, column, "[sinop] not supported rvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    sym_binop_t::sym_binop_t(const type_exp_t::ref & exp1, const type_exp_t::ref & exp2, ast_node * op)
        : type_exp_t(nullptr), exp1(exp1), exp2(exp2), op(op) {
        line = exp1->line;
        column = exp1->column;
    }

    symbol_t sym_binop_t::get_type() const {
        return s_binop;
    }

    int sym_binop_t::size(sym_size_t t, int level) const {
        if (!base) return 0;
        return base->size(t, level);
    }

    string_t sym_binop_t::get_name() const {
        return to_string();
    }

    string_t sym_binop_t::to_string() const {
        std::stringstream ss;
        ss << "(binop, " << cast::to_string(op)
            << ", exp1: " << exp1->to_string()
            << ", exp2: " << exp2->to_string() << ')';
        return ss.str();
    }

    gen_t sym_binop_t::gen_lvalue(igen & gen) {
        switch (op->data._op) {
        case op_lsquare: {
            auto r = exp1->gen_lvalue(gen);
            if (r != g_no_load && exp1->size(x_matrix) == 0)
                gen.emit(LOAD, exp1->size(x_size));
            base = exp1->base->clone();
            auto size = base->get_cast();
            auto c = cast_size(size);
            if (size != t_ptr)
                gen.error(line, column, "invalid address by []: " + exp1->base->to_string());
            base->ptr--;
            if (!base->matrix.empty()) {
                base->matrix.pop_back();
            }
            gen.emit(PUSH, c); // 压入数组地址
            exp2->gen_rvalue(gen); // index
            size = exp2->base->get_cast();
            c = cast_size(size);
            auto n = exp1->size(x_inc);
            if (n > 1) {
                gen.emit(PUSH, c);
                gen.emit(IMM, n);
                gen.emit(MUL, size);
                gen.emit(ADD, size);
            }
            else {
                gen.emit(ADD, size);
            }
        }
                         break;
        case op_dot: {
            exp1->gen_lvalue(gen);
            base = exp1->base;
            if (base->get_type() != s_type_typedef)
                gen.error(line, column, "[binop] need struct type: " + exp1->to_string());
            auto type_def = std::dynamic_pointer_cast<type_typedef_t>(base);
            auto _type = type_def->refer.lock();
            if (_type->get_type() != s_struct)
                gen.error(line, column, "[binop] need struct type: " + exp1->to_string());
            if (exp1->get_cast() == t_ptr)
                gen.error(line, column, "[binop] need non-pointer type: " + exp1->to_string());
            auto _struct = std::dynamic_pointer_cast<sym_struct_t>(_type);
            auto dec = exp2->get_name();
            sym_id_t::ref field;
            for (auto& d : _struct->decls) {
                if (d->get_name() == dec) {
                    field = d;
                    break;
                }
            }
            if (!field)
                gen.error(line, column, "[binop] invalid struct field: " + to_string());
            gen.emit(PUSH, cast_size(t_ptr));
            field->gen_lvalue(gen);
            base = field->base;
            gen.emit(ADD, t_ptr);
        }
                     break;
        case op_pointer: {
            exp1->gen_rvalue(gen);
            base = exp1->base;
            if (base->get_cast() != t_ptr) {
                gen.error(line, column, "[binop] need struct pointer");
            }
            if (base->get_type() != s_type_typedef)
                gen.error(line, column, "[binop] need struct type");
            auto type_def = std::dynamic_pointer_cast<type_typedef_t>(base);
            auto _type = type_def->refer.lock();
            if (_type->get_type() != s_struct)
                gen.error(line, column, "[binop] need struct type");
            auto _struct = std::dynamic_pointer_cast<sym_struct_t>(_type);
            auto dec = exp2->get_name();
            sym_id_t::ref field;
            for (auto& d : _struct->decls) {
                if (d->get_name() == dec) {
                    field = d;
                    break;
                }
            }
            if (!field)
                gen.error(line, column, "[binop] invalid struct field: " + to_string());
            gen.emit(PUSH, cast_size(t_ptr));
            field->gen_lvalue(gen);
            base = field->base;
            gen.emit(ADD, t_ptr);
        }
                         break;
        default:
            gen.error(line, column, "[binop] not supported lvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    gen_t sym_binop_t::gen_rvalue(igen & gen) {
        switch (op->data._op) {
        case op_equal:
        case op_plus:
        case op_minus:
        case op_times:
        case op_divide:
        case op_bit_and:
        case op_bit_or:
        case op_bit_xor:
        case op_mod:
        case op_less_than:
        case op_less_than_or_equal:
        case op_greater_than:
        case op_greater_than_or_equal:
        case op_not_equal:
        case op_left_shift:
        case op_right_shift: {
            exp1->gen_rvalue(gen); // exp1
            auto idx = gen.current();
            gen.emit(NOP); // insert CAST inst
            gen.emit(NOP);
            gen.emit(PUSH, cast_size(exp1->base->get_cast()));
            exp2->gen_rvalue(gen); // exp2
#if LOG_TYPE
            cgen::log_out << "[DEBUG] Binop type: op1= " << cast_str(exp1->base->get_cast())
                << ", op2=  " << cast_str(exp2->base->get_cast()) << std::endl;
#endif
            if ((op->data._op == op_plus || op->data._op == op_minus) &&
                exp1->base->get_cast() == t_ptr &&
                exp2->base->get_cast() == t_int) { // 指针+常量
                base = exp1->base->clone();
                auto inc = exp1->size(x_inc);
                if (inc > 1) {
                    auto size = cast_size(exp2->base->get_cast());
                    gen.emit(PUSH, size);
                    gen.emit(IMM, inc);
                    gen.emit(MUL, exp2->base->get_cast());
                }
                gen.emit(OP_INS(op->data._op), base->get_cast());
            }
            else {
                auto t1 = exp1->base->get_cast();
                auto t2 = exp2->base->get_cast();
                if (t1 != t2) {
                    if (t1 == t_error)
                        gen.error(line, column, "arithmetic binop: src error");
                    if (t2 == t_error)
                        gen.error(line, column, "arithmetic binop: dst error");
                    auto use_first = t1 > t2;
                    auto max_type = use_first ? t1 : t2;
                    auto min_type = use_first ? t2 : t1;
                    if (max_type == t_ptr || max_type == t_struct) {
                        gen.error(line, column, "arithmetic binop: unsupported cast, exp1= " + exp1->to_string() +
                            ", exp2= " + exp2->to_string());
                    }
                    auto s = cast_find(min_type, max_type);
                    if (s == -1)
                        gen.error(line, column, "arithmetic binop: invalid cast, exp1= " + exp1->to_string() +
                            ", exp2= " + exp2->to_string());
                    if (s != 0) {
                        if (use_first) {
                            gen.emit(CAST, s);
                        }
                        else {
                            gen.edit(idx, CAST);
                            gen.edit(idx + 1, s); // CAST s
                            gen.edit(idx + 3, cast_size(max_type));
                        }
                    }
                    base = use_first ? exp1->base->clone() : exp2->base->clone();
                    gen.emit(OP_INS(op->data._op), max_type);
                }
                else {
                    base = exp1->base->clone();
                    gen.emit(OP_INS(op->data._op), base->get_cast());
                }
                switch (op->data._op) {
                case op_equal:
                case op_less_than:
                case op_less_than_or_equal:
                case op_greater_than:
                case op_greater_than_or_equal:
                case op_not_equal: {
                    base.reset();
                    base = std::make_shared<type_base_t>(l_int);
                }
                                   break;
                default:
                    break;
                }
            }
        }
                             break;
        case op_assign: {
            exp1->gen_lvalue(gen);
            gen.emit(PUSH, cast_size(t_ptr));
            exp2->gen_rvalue(gen);
            auto t1 = exp1->base->get_cast();
            auto t2 = exp2->base->get_cast();
            if (t1 != t2) {
                if (t1 == t_error)
                    gen.error(line, column, "assign assign: src error");
                if (t2 == t_error)
                    gen.error(line, column, "assign assign: dst error");
                auto s = cast_find(t2, t1);
                if (s == -1)
                    gen.error(line, column, "assign assign: invalid cast, exp1= " + exp1->to_string() +
                        ", exp2= " + exp2->to_string());
                if (s != 0)
                    gen.emit(CAST, s);
            }
            base = exp1->base->clone();
            auto size = exp1->size(x_load);
            if (size == 0) {
                size = exp1->size(x_load);
                gen.error(line, column, "size == zero: " + to_string());
            }
            gen.emit(SAVE, size);
        }
                        break;
        case op_plus_assign:
        case op_minus_assign:
        case op_times_assign:
        case op_div_assign:
        case op_and_assign:
        case op_or_assign:
        case op_xor_assign:
        case op_mod_assign:
        case op_left_shift_assign:
        case op_right_shift_assign: {
            exp1->gen_lvalue(gen);
            gen.emit(PUSH, cast_size(t_ptr));
            exp1->gen_rvalue(gen);
            gen.emit(PUSH, cast_size(exp1->base->get_cast()));
            exp2->gen_rvalue(gen);
            auto t1 = exp1->base->get_cast();
            auto t2 = exp2->base->get_cast();
            if (t1 != t2) {
                if (t1 == t_error)
                    gen.error(line, column, "assign binop: src error");
                if (t2 == t_error)
                    gen.error(line, column, "assign binop: dst error");
                auto s = cast_find(t2, t1);
                if (s == -1)
                    gen.error(line, column, "assign binop: invalid cast, exp1= " + exp1->to_string() +
                        ", exp2= " + exp2->to_string());
                if (s != 0)
                    gen.emit(CAST, s);
            }
            base = exp1->base->clone();
            gen.emit(OP_INS(op->data._op), base->get_cast());
            gen.emit(SAVE, exp1->size(x_load));
        }
                                    break;
        case op_logical_and:
        case op_logical_or: {
            exp1->gen_rvalue(gen);
            gen.emit(OP_INS(op->data._op), -1); // 短路优化
            auto L1 = gen.current() - 1;
            exp2->gen_rvalue(gen);
            auto t1 = exp1->base->get_cast();
            auto t2 = exp2->base->get_cast();
            if (!(t1 == t_ptr && t2 == t_ptr) && max(t1, t2) >= t_long) {
                gen.error(line, column, "logical binop: unsupported cast, op= " + OP_STRING(op->data._op) +
                    ", exp1= " + exp1->to_string() +
                    ", exp2= " + exp2->to_string());
            }
            base = t1 > t2 ? exp1->base->clone() : exp2->base->clone();
            gen.edit(L1, gen.current()); // a = exit
        }
                            break;
        case op_lsquare: {
            gen_lvalue(gen);
            gen.emit(LOAD, base->size(x_load));
        }
                         break;
        case op_lparan: {
            auto exp = std::dynamic_pointer_cast<sym_t>(exp2);
            exp1->gen_invoke(gen, exp);
            base = exp1->base->clone();
        }
                        break;
        case op_dot: {
            gen_lvalue(gen);
            gen.emit(LOAD, base->size(x_load));
        }
                     break;
        case op_pointer: {
            gen_lvalue(gen);
            gen.emit(LOAD, base->size(x_load));
        }
                         break;
        default:
            gen.error(line, column, "[binop] not supported rvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    sym_triop_t::sym_triop_t(const type_exp_t::ref & exp1, const type_exp_t::ref & exp2,
        const type_exp_t::ref & exp3, ast_node * op1, ast_node * op2)
        : type_exp_t(nullptr), exp1(exp1), exp2(exp2), exp3(exp3), op1(op1), op2(op2) {
        line = exp1->line;
        column = exp1->column;
    }

    symbol_t sym_triop_t::get_type() const {
        return s_triop;
    }

    int sym_triop_t::size(sym_size_t t, int level) const {
        if (t == x_inc)
            return 0;
        return max(max(exp1->size(t, level), exp2->size(t, level)), exp3->size(t, level));
    }

    string_t sym_triop_t::get_name() const {
        return to_string();
    }

    string_t sym_triop_t::to_string() const {
        std::stringstream ss;
        ss << "(triop, " << cast::to_string(op1)
            << ", " << cast::to_string(op2)
            << ", exp1: " << exp1->to_string()
            << ", exp2: " << exp2->to_string()
            << ", exp3: " << exp3->to_string() << ')';
        return ss.str();
    }

    gen_t sym_triop_t::gen_lvalue(igen & gen) {
        gen.error(line, column, "[triop] not supported: " + to_string());
        return g_error;
    }

    gen_t sym_triop_t::gen_rvalue(igen & gen) {
        if (AST_IS_OP_N(op1, op_query) && AST_IS_OP_N(op2, op_colon)) {
            exp1->gen_rvalue(gen); // cond
            gen.emit(JZ, -1);
            auto L1 = gen.current() - 1;
            exp2->gen_rvalue(gen); // true
            base = exp2->base;
            gen.emit(JMP, -1);
            auto L2 = gen.current() - 1;
            gen.edit(L1, gen.current());
            exp3->gen_rvalue(gen); // false
            gen.edit(L2, gen.current());
            return g_ok;
        }
        else {
            gen.error(line, column, "[triop] not supported: " + to_string());
            return g_error;
        }
    }

    sym_list_t::sym_list_t() : type_exp_t(nullptr) {}

    symbol_t sym_list_t::get_type() const {
        return s_list;
    }

    int sym_list_t::size(sym_size_t t, int level) const {
        if (t == x_inc)
            return 0;
        return exps.size();
    }

    string_t sym_list_t::get_name() const {
        return to_string();
    }

    string_t sym_list_t::to_string() const {
        if (exps.empty())
            return "(list: empty)";
        std::stringstream ss;
        ss << "(list, " << string_join(exps, ", ") << ')';
        return ss.str();
    }

    gen_t sym_list_t::gen_lvalue(igen & gen) {
        gen.error(line, column, "not supported: " + to_string());
        return g_error;
    }

    gen_t sym_list_t::gen_rvalue(igen & gen) {
        for (auto& exp : exps) {
            exp->gen_rvalue(gen);
        }
        base = exps.back()->base->clone();
        return g_ok;
    }

    sym_ctrl_t::sym_ctrl_t(ast_node * op) : op(op) {
        line = op->line;
        column = op->column;
    }

    symbol_t sym_ctrl_t::get_type() const {
        return s_ctrl;
    }

    int sym_ctrl_t::size(sym_size_t t, int level) const {
        return 0;
    }

    string_t sym_ctrl_t::get_name() const {
        return KEYWORD_STRING(op->data._keyword);
    }

    string_t sym_ctrl_t::to_string() const {
        std::stringstream ss;
        ss << "(" << get_name();
        if (exp)
            ss << ", " << exp->to_string();
        ss << ")";
        return ss.str();
    }

    gen_t sym_ctrl_t::gen_lvalue(igen & gen) {
        gen.error(line, column, "[ctrl] not supported lvalue: " + to_string());
        return g_error;
    }

    gen_t sym_ctrl_t::gen_rvalue(igen & gen) {
        switch (op->data._keyword) {
        case k_return: {
#if LOG_TYPE
            cgen::log_out << "[DEBUG] Return: exp= " << sym_to_string(exp) << std::endl;
#endif
            if (exp)
                exp->gen_rvalue(gen);
            gen.emit(LEV);
        }
                       break;
        case k_break:
        case k_continue: {
            gen.emit(line, column, op->data._keyword);
        }
                         break;
        case k_interrupt: {
            auto number = std::dynamic_pointer_cast<sym_var_t>(exp);
#if LOG_TYPE
            cgen::log_out << "[DEBUG] Interrupt: number= " << sym_to_string(exp) << std::endl;
#endif
            gen.emit(INTR, number->node->data._int);
        }
                          break;
        case k_goto: {
            auto label = std::dynamic_pointer_cast<sym_var_t>(exp);
#if LOG_TYPE
            cgen::log_out << "[DEBUG] Goto: label= " << sym_to_string(exp) << std::endl;
#endif
            gen.emit(JMP, -1);
            gen.add_label(label->line, label->column, gen.current() - 1, label->node->data._string);
        }
                          break;
        default:
            gen.error(line, column, "[ctrl] not supported rvalue: " + to_string());
            return g_error;
        }
        return g_ok;
    }

    // --------------------------------------------------------------

    cgen::cgen() {
        init_macro();
        reset();
    }

    void cgen::init_macro()
    {
        macros.insert(std::make_pair("__LINE__", m_line));
        macros.insert(std::make_pair("__COLUMN__", m_column));
        macros.insert(std::make_pair("__FUNC__", m_func));
        macros.insert(std::make_pair("__FILE__", m_file));
    }

    void cgen::gen(const string_t& page, ast_node * node) {
        this->page = page;
        gen_rec(node, 0);
        if (!func_write_backs.empty()) {
            std::stringstream ss;
            ss << "unimplemented func: ";
            std::copy(func_write_backs.begin(), func_write_backs.end(),
                std::ostream_iterator<std::string>(ss, ", "));
            auto s = ss.str();
            s.pop_back();
            s.pop_back();
            error(node, s);
        }
    }

    void cgen::reset() {
        page.clear();
        symbols.clear();
        symbols.emplace_back();
        tmp.clear();
        tmp.emplace_back();
        ast.clear();
        ast.emplace_back();
        data.clear();
        text.clear();
        cases.clear();
        ctx.reset();
        cycle.clear();
        incs.clear();
        pdbs.clear();
        pdbs.push_back(std::make_tuple(0, "error"));
        func_write_backs.clear();
        log_out.str("");
        labeled_id = -1;
        macro_data.clear();
    }

    template <class T>
    static void bit_write(std::vector<byte>& v, const T& n) {
        std::copy((byte*)& n, ((byte*)& n) + sizeof(n), std::back_inserter(v));
    }

    std::vector<byte> cgen::file() const {
        std::vector<byte> file;
        auto entry = symbols[0].find("main");
        if (entry == symbols[0].end()) {
            error(-1, -1, "main() not defined");
        }
#if LOG_TYPE
        {
            std::ofstream log(REPORT_GEN_FILE, std::ios::app | std::ios::out);
            log << std::endl << log_out.str() << std::endl;
        }
#endif
        // MAGIC
        auto magic = string_t(PE_MAGIC);
        std::copy((byte*)magic.data(), (byte*)magic.data() + magic.size(), std::back_inserter(file));
        // ENTRY ADDR
        bit_write(file, std::dynamic_pointer_cast<sym_func_t>(entry->second)->addr);
        // DATA SIZE
        bit_write(file, data.size() * sizeof(data[0]));
        // TEXT SIZE
        bit_write(file, text.size() * sizeof(text[0]));
        // PDB SIZE
        std::vector<byte> pdb_data;
        {
            bit_write(pdb_data, pdbs.size());
            auto tmp_idx = pdb_data.size();
            for (auto& p : pdbs) {
                // INDEX
                bit_write(pdb_data, std::get<0>(p));
                // ADDR
                bit_write(pdb_data, 0);
            }
            // CODE
            auto code_idx = pdb_data.size();
            int i = 0;
            for (auto& p : pdbs) {
                // ADDR
                auto addr = (PDB_ADDR*)(pdb_data.data() + tmp_idx) + i++;
                addr->addr = pdb_data.size() - code_idx;
                // CODE
                for (auto& s : std::get<1>(p)) {
                    pdb_data.push_back((byte)s);
                }
                pdb_data.push_back(0);
            }
        }
        bit_write(file, pdb_data.size());
        // PDB2 SIZE
        std::vector<byte> pdb_data2;
        {
            std::vector<sym_func_t::ref> funcs;
            for (auto& sy : symbols[0]) {
                if (sy.second->get_type() == s_function)
                    funcs.push_back(std::dynamic_pointer_cast<sym_func_t>(sy.second));
            }
            bit_write(pdb_data2, funcs.size());
            auto tmp_idx = pdb_data2.size();
            for (auto& p : funcs) {
                // INDEX
                bit_write(pdb_data2, p->entry - 1);
                // ADDR
                bit_write(pdb_data2, 0);
            }
            // CODE
            auto code_idx = pdb_data2.size();
            int i = 0;
            for (auto& p : funcs) {
                // ADDR
                auto addr = (PDB_ADDR*)(pdb_data2.data() + tmp_idx) + i++;
                addr->addr = pdb_data2.size() - code_idx;
                std::stringstream ss;
                string_t page; int L; CStringA ts;
                if (get_line(p->line, page, L)) {
                    ts.Format("[%s:%d:%d:%d]", page.c_str(), L, p->column, p->entry - 1);
                }
                else {
                    ts.Format("[???:%d:%d:%d]", p->line, p->column, p->entry - 1);
                }
                ss << ts.GetBuffer(0) << " ";
                ss << p->base->to_string() << " " << p->id << "(";
                for (size_t pi = 0; pi < p->params.size(); pi++) {
                    ss << p->params[pi]->base->to_string() << " " << p->params[pi]->id;
                    if (pi + 1 < p->params.size())
                        ss << ", ";
                }
                ss << ")";
                auto info = ss.str();
                // CODE
                for (auto& s : info) {
                    pdb_data2.push_back((byte)s);
                }
                pdb_data2.push_back(0);
            }
        }
        bit_write(file, pdb_data2.size());
        // DATA
        std::copy(data.begin(), data.end(), std::back_inserter(file));
        // TEXT
        std::copy((byte*)text.data(), ((byte*)text.data()) + text.size() * sizeof(text[0]), std::back_inserter(file));
        // PDB
        std::copy(pdb_data.begin(), pdb_data.end(), std::back_inserter(file));
        // PDB2
        std::copy(pdb_data2.begin(), pdb_data2.end(), std::back_inserter(file));
        return file;
    }

    void cgen::emit(ins_t i) {
#if LOG_TYPE
        log_out << "[DEBUG] *GEN* ==> [" << setiosflags(std::ios::right)
            << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(8)
            << std::setfill('0') << text.size() << "] " << setiosflags(std::ios::left)
            << std::setw(4) << std::setfill(' ') << INS_STRING(i) << std::endl;
#endif
        text.push_back(i);
    }

    void cgen::emit(ins_t i, int d) {
#if LOG_TYPE
        log_out << "[DEBUG] *GEN* ==> [" << setiosflags(std::ios::right)
            << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(8)
            << std::setfill('0') << text.size() << "] " << setiosflags(std::ios::left)
            << std::setw(4) << std::setfill(' ') << INS_STRING(i) << " " << setiosflags(std::ios::right)
            << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(8)
            << std::setfill('0') << d << "(" << std::dec << d << ")" << std::endl;
#endif
        text.push_back(i);
        text.push_back(d);
    }

    void cgen::emit(ins_t i, int d, int e) {
#if LOG_TYPE
        log_out << "[DEBUG] *GEN* ==> [" << setiosflags(std::ios::right)
            << std::setiosflags(std::ios::uppercase) << std::hex << std::setw(8)
            << std::setfill('0') << text.size() << "] " << setiosflags(std::ios::left)
            << std::setw(4) << std::setfill(' ') << INS_STRING(i) << " " << setiosflags(std::ios::right)
            << std::setiosflags(std::ios::uppercase) << std::hex
            << std::setw(8) << std::setfill('0') << d << " "
            << std::setw(8) << std::setfill('0') << e << std::endl;
#endif
        text.push_back(i);
        text.push_back(d);
        text.push_back(e);
    }

    void cgen::emit(int line, int column, keyword_t k) {
        switch (k) {
        case k_break: {
            if (cycle.empty()) {
                error(line, column, "invalid break");
            }
            for (auto c = cycle.rbegin(); c != cycle.rend(); ++c) {
                if (c->_break >= 0) {
                    emit(JMP, c->_break);
                    return;
                }
            }
            error(line, column, "invalid break");
        }
                      break;
        case k_continue: {
            if (cycle.empty()) {
                error(line, column, "invalid continue");
            }
            for (auto c = cycle.rbegin(); c != cycle.rend(); ++c) {
                if (c->_continue >= 0) {
                    emit(JMP, c->_continue);
                    return;
                }
            }
            error(line, column, "invalid continue");
        }
                         break;
        default:
            error(line, column, "invalid keyword: " + KEYWORD_STRING(k));
            break;
        }
    }

    int cgen::current() const {
        return text.size();
    }

    void cgen::edit(int addr, int value) {
        if (addr >= 0 && addr < (int)text.size()) {
            text[addr] = value;
        }
    }

    int cgen::load_string(const string_t & s) {
        auto addr = data.size();
        std::copy(s.begin(), s.end(), std::back_inserter(data));
        data.push_back(0);
        while (data.size() % 4 != 0) {
            data.push_back(0);
        }
#if LOG_TYPE
        log_out << "[DEBUG] *GEN* <== STRING: \"" << cast::display_str(s.c_str())
            << "\", addr: " << addr << ", size: " << s.length() << std::endl;
#endif
        return addr;
    }

    void cgen::add_label(int line, int column, int index, const string_t& label)
    {
        auto c = ctx.lock();
        if (!c || c->get_type() != s_function) {
            error(line, column, "invalid label: must be in function");
        }
        auto func = std::dynamic_pointer_cast<sym_func_t>(c);
        auto f = func->labels.find(label);
        if (f != func->labels.end()) {
            edit(index, f->second);
        }
        else {
            func->labels_writeback.emplace_back(index, label, line, column);
        }
    }

    template<class T>
    static void gen_recursion(ast_node * node, int level, T f) {
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
    static std::vector<T*> gen_get_children(T * node) {
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

    void cgen::gen_rec(ast_node * node, int level) {
        if (node == nullptr)
            return;
        auto rec = [this](auto n, auto l) { this->gen_rec(n, l); };
        auto type = (ast_t)node->flag;
        if (type == ast_collection) {
            if ((node->attr & a_exp) && node->child == node->child->next) {
                gen_recursion(node->child, level, rec);
                return;
            }
        }
        tmp.emplace_back();
        ast.emplace_back();
        switch (type) {
        case ast_root: {// 根结点，全局声明
            gen_recursion(node->child, level, rec);
        }
                       break;
        case ast_collection: {
            auto children = gen_get_children(node->child);
            gen_coll(children, level + 1, node);
        }
                             break;
        case ast_keyword:
        case ast_operator:
        case ast_literal:
        case ast_string:
        case ast_char:
        case ast_uchar:
        case ast_short:
        case ast_ushort:
        case ast_int:
        case ast_uint:
        case ast_long:
        case ast_ulong:
        case ast_float:
        case ast_double:
            ast.back().push_back(node);
            break;
        }
        if (!tmp.back().empty()) {
            //assert(tmp.back().size() == 1);
            auto& top = tmp[tmp.size() - 2];
            for (auto& t : tmp.back()) {
                top.push_back(t);
            }
        }
        tmp.pop_back();
        if (!ast.back().empty()) {
            auto& top = ast[ast.size() - 2];
            std::copy(ast.back().begin(), ast.back().end(), std::back_inserter(top));
        }
        ast.pop_back();
    }

    void cgen::gen_coll(const std::vector<ast_node*> & nodes, int level, ast_node * node) {
        switch (node->data._coll) {
        case c_program:
            break;
        case c_primaryExpression:
            break;
        case c_constant:
            break;
        case c_postfixExpression:
            break;
        case c_argumentExpressionList:
            break;
        case c_unaryExpression:
            break;
        case c_unaryOperator:
            break;
        case c_castExpression:
            break;
        case c_multiplicativeExpression:
            break;
        case c_additiveExpression:
            break;
        case c_shiftExpression:
            break;
        case c_relationalExpression:
            break;
        case c_equalityExpression:
            break;
        case c_andExpression:
            break;
        case c_exclusiveOrExpression:
            break;
        case c_inclusiveOrExpression:
            break;
        case c_logicalAndExpression:
            break;
        case c_logicalOrExpression:
            break;
        case c_conditionalExpression:
            break;
        case c_assignmentExpression:
            break;
        case c_assignmentOperator:
            break;
        case c_expression:
            break;
        case c_constantExpression:
            break;
        case c_declaration:
            break;
        case c_declarationSpecifiers:
            break;
        case c_declarationSpecifiers2:
            break;
        case c_declarationSpecifier:
            break;
        case c_initDeclaratorList:
            break;
        case c_initDeclarator:
            break;
        case c_storageClassSpecifier:
            break;
        case c_typeSpecifier:
            break;
        case c_structOrUnion:
            break;
        case c_structDeclarationList: {
            auto name = (ast.rbegin() + 1)->back();
            auto & sym = symbols[0];
            auto f = sym.find(name->data._string);
            if (f != sym.end()) {
                if (ctx.lock()) {
                    ctx_stack.push_back(ctx.lock());
                }
                ctx = f->second;
            }
            else {
                error(name, "invalid struct/union name");
            }
        }
                                      break;
        case c_structDeclaration:
            break;
        case c_specifierQualifierList:
            break;
        case c_structDeclaratorList:
            break;
        case c_structDeclarator:
            break;
        case c_enumSpecifier:
            break;
        case c_enumeratorList:
            break;
        case c_enumerator:
            break;
        case c_enumerationConstant:
            break;
        case c_typeQualifier:
            break;
        case c_declarator:
            break;
        case c_directDeclarator:
            break;
        case c_pointer:
            break;
        case c_typeQualifierList:
            break;
        case c_parameterTypeList:
            break;
        case c_parameterList:
            break;
        case c_parameterDeclaration:
            break;
        case c_identifierList:
            break;
        case c_typeName:
            break;
        case c_abstractDeclarator:
            break;
        case c_directAbstractDeclarator:
            break;
        case c_typedefName:
            break;
        case c_initializer:
            break;
        case c_initializerList:
            break;
        case c_designation:
            break;
        case c_designatorList:
            break;
        case c_designator:
            break;
        case c_statement:
            break;
        case c_labeledStatement: {
            labeled_id = (int)text.size();
            if (nodes[0]->flag == ast_literal) {
                break;
            }
            switch_t s;
            s.addr = labeled_id;
            cases.back().insert(std::make_pair(nodes[0], s));
#if LOG_TYPE
            log_out << "[DEBUG] Case: addr= " << s.addr << std::endl;
#endif
        }
                                 break;
        case c_compoundStatement:
            symbols.emplace_back();
            break;
        case c_blockItemList:
            break;
        case c_blockItem:
            break;
        case c_expressionStatement:
            break;
        case c_selectionStatement:
        case c_iterationStatement:
            return gen_stmt(nodes, level, node);
        case c_forCondition:
            break;
        case c_forDeclaration:
            break;
        case c_forExpression:
            break;
        case c_jumpStatement:
            break;
        case c_pragmaStatement:
            break;
        case c_compilationUnit:
            break;
        case c_translationUnit:
            break;
        case c_externalDeclaration:
            break;
        case c_functionDefinition:
        case c_structOrUnionSpecifier:
            symbols.emplace_back();
            break;
        case c_declarationList:
            break;
        }
        for (auto& n : nodes) {
            gen_rec(n, level);
        }
        auto& asts = ast.back();
        switch (node->data._coll) {
        case c_program:
            break;
        case c_primaryExpression: {
            if (tmp.back().empty()) {
                auto pri = primary_node(asts[0]);
                tmp.back().push_back(pri);
                asts.clear();
            }
        }
                                  break;
        case c_constant:
            break;
        case c_postfixExpression: {
            if (AST_IS_COLL_N(nodes[0], c_primaryExpression)) {
                auto tmp_i = 0;
                auto exp = to_exp(tmp.back()[tmp_i++]);
                for (size_t i = 1; i < nodes.size(); ++i) {
                    auto& a = nodes[i];
                    if (AST_IS_OP(a)) {
                        if (AST_IS_OP_K(a, op_plus_plus) || AST_IS_OP_K(a, op_minus_minus)) {
                            exp = std::make_shared<sym_sinop_t>(exp, a);
                        }
                        else if (AST_IS_OP_K(a, op_dot) || AST_IS_OP_K(a, op_pointer)) {
                            ++i;
                            auto exp2 = primary_node(nodes[i]);
                            exp = std::make_shared<sym_binop_t>(exp, exp2, a);
                        }
                        else if (AST_IS_OP_K(a, op_lsquare)) {
                            ++i;
                            auto exp2 = to_exp(tmp.back()[tmp_i++]);
                            exp = std::make_shared<sym_binop_t>(exp, exp2, a);
                        }
                        else if (AST_IS_OP_K(a, op_lparan)) {
                            ++i;
                            if (!AST_IS_OP_K(nodes[i], op_rparan)) {
                                exp = std::make_shared<sym_binop_t>(exp,
                                    to_exp(tmp.back()[tmp_i++]), a);
                                ++i;
                            }
                            else {
                                auto exp2 = std::make_shared<sym_list_t>();
                                exp = std::make_shared<sym_binop_t>(exp, exp2, a);
                            }
                        }
                        else {
                            error(a, "invalid postfix  op");
                        }
                    }
                    else {
                        error(a, "invalid postfix  coll");
                    }
                }
                tmp.back().clear();
                tmp.back().push_back(exp);
                asts.clear();
            }
        }
                                  break;
        case c_argumentExpressionList: {
            auto list = std::make_shared<sym_list_t>();
            for (auto& _t : tmp.back()) {
                list->exps.push_back(to_exp(_t));
            }
            tmp.back().clear();
            tmp.back().push_back(list);
        }
                                       break;
        case c_unaryExpression: {
            auto& op = asts[0];
            if (AST_IS_OP(op)) {
                switch (op->data._op) {
                case op_plus:
                case op_plus_plus:
                case op_minus:
                case op_minus_minus:
                case op_logical_and:
                case op_logical_not:
                case op_bit_and:
                case op_bit_not:
                case op_times: {
                    auto& _exp = tmp.back().back();
                    auto exp = to_exp(_exp);
                    tmp.back().clear();
                    auto unop = std::make_shared<sym_unop_t>(exp, op);
                    tmp.back().push_back(unop);
                    asts.clear();
                }
                               break;
                default:
                    error(asts[0], "invalid unary  op");
                    break;
                }
            }
            else if (AST_IS_KEYWORD_N(op, k_sizeof)) {
                auto type = tmp.back().front();
                if (type->get_base_type() == s_type) {
                    auto exp = std::make_shared<type_exp_t>(std::dynamic_pointer_cast<type_t>(type));
                    auto s = std::make_shared<sym_unop_t>(exp, op);
                    tmp.back().clear();
                    tmp.back().push_back(s);
                }
                else {
                    auto exp = to_exp(type);
                    auto s = std::make_shared<sym_unop_t>(exp, op);
                    tmp.back().clear();
                    tmp.back().push_back(s);
                }
                asts.clear();
            }
            else {
                error(asts[0], "invalid unary  coll");
            }
        }
                                break;
        case c_unaryOperator:
            break;
        case c_castExpression: {
            assert(tmp.back().front()->get_base_type() == s_type);
            auto type = std::dynamic_pointer_cast<type_t>(tmp.back().front());
            auto exp = to_exp(tmp.back().back());
            auto cast = std::make_shared<sym_cast_t>(exp, type);
            tmp.back().clear();
            tmp.back().push_back(cast);
        }
                               break;
        case c_multiplicativeExpression:
        case c_additiveExpression:
        case c_shiftExpression:
        case c_relationalExpression:
        case c_equalityExpression:
        case c_andExpression:
        case c_exclusiveOrExpression:
        case c_inclusiveOrExpression:
        case c_logicalAndExpression:
        case c_logicalOrExpression:
        case c_conditionalExpression: {
            size_t tmp_i = 0;
            auto exp1 = to_exp(tmp.back()[tmp_i++]);
            auto exp2 = to_exp(tmp.back()[tmp_i++]);
            for (size_t i = 0; i < asts.size(); ++i) {
                auto& a = asts[i];
                if (AST_IS_OP(a)) {
                    if (node->data._coll == c_conditionalExpression &&
                        AST_IS_OP_K(a, op_query)) { // triop
                        auto exp3 = to_exp(tmp.back()[tmp_i++]);
                        exp1 = std::make_shared<sym_triop_t>(exp1, exp2, exp3, a, asts[i + 1]);
                        if (tmp_i < tmp.back().size())
                            exp2 = to_exp(tmp.back()[tmp_i++]);
                        i++;
                    }
                    else { // binop
                        exp1 = std::make_shared<sym_binop_t>(exp1, exp2, a);
                        if (tmp_i < tmp.back().size())
                            exp2 = to_exp(tmp.back()[tmp_i++]);
                    }
                }
                else {
                    error(a, "invalid binop: coll");
                }
            }
            tmp.back().clear();
            tmp.back().push_back(exp1);
            asts.clear();
        }
                                      break;
        case c_assignmentExpression: {
            auto tmp_i = 0;
            auto exp1 = to_exp(tmp.back()[tmp_i++]);
            auto exp2 = to_exp(tmp.back()[tmp_i++]);
            auto exp = std::make_shared<sym_binop_t>(exp1, exp2, asts.front());
            tmp.back().clear();
            tmp.back().push_back(exp);
            asts.clear();
        }
                                     break;
        case c_assignmentOperator:
            break;
        case c_expression:
            break;
        case c_constantExpression:
            break;
        case c_declaration:
            break;
        case c_declarationSpecifiers:
        case c_specifierQualifierList: {
            type_t::ref base_type;
            if (AST_IS_KEYWORD_N(asts[0], k_struct) || AST_IS_KEYWORD_K(asts[0], k_union)) {
                auto f = symbols[0].find(asts[1]->data._string);
                if (f == symbols[0].end()) {
                    error(asts[1], "missing struct type");
                }
                auto s = std::dynamic_pointer_cast<sym_struct_t>(f->second);
                if (s->get_type() != s_struct) {
                    error(asts[1], "need struct type");
                }
                tmp.back().push_back(s);
                asts.clear();
                break;
            }
            if (AST_IS_KEYWORD_N(asts[0], k_enum)) {
                auto ast_i = 1;
                auto tmp_i = 0;
                if (AST_IS_ID(asts[ast_i]))
                    ast_i++;
                if (AST_IS_OP_N(asts[ast_i], op_lbrace))
                    ast_i++;
                auto clazz = ctx.lock() ? z_local_var : z_global_var;
                auto& s = symbols.back();
                auto& _tmp = tmp.back();
                ast_node zero;
                zero.flag = ast_int;
                zero.data._int = 0;
                auto init_type = std::make_shared<type_base_t>(l_int, 0);
                type_exp_t::ref init = std::make_shared<sym_var_t>(init_type, &zero);
                auto delta = -1;
                for (size_t i = ast_i; i < asts.size(); ++i) {
                    auto& a = asts[i];
                    if (AST_IS_ID(a)) {
                        if (a->parent != a->parent->next) {
                            delta = 0;
                            init = to_exp(_tmp[tmp_i++]);
                        }
                        else {
                            delta++;
                        }
                        if (s.find(a->data._string) == s.end()) {
                            auto type = std::make_shared<type_base_t>(l_int, 0);
                            add_id(type, clazz, a, init, delta);
                        }
                        else {
                            error(a, "conflict enum id: ", true);
                        }
                    }
                }
                asts.clear();
                _tmp.clear();
                _tmp.emplace_back(std::make_shared<type_base_t>(l_int, 0));
                break;
            }
            bool _static = false;
            if (AST_IS_KEYWORD_N(asts[0], k_static)) { // static
                _static = true;
                asts.erase(asts.begin());
            }
            if (AST_IS_KEYWORD_N(asts[0], k_unsigned)) { // unsigned ...
                if (asts.size() == 1 || (asts.size() > 1 && !AST_IS_KEYWORD(asts[1]))) {
                    base_type = std::make_shared<type_base_t>(l_uint);
                    base_type->line = asts[0]->line;
                    base_type->column = asts[0]->column;
                    asts.erase(asts.begin());
                }
                else {
                    assert(asts.size() > 1 && AST_IS_KEYWORD(asts[1]));
                    lexer_t type;
                    switch (asts[1]->data._keyword) {
                    case k_char:
                        type = l_uchar;
                        break;
                    case k_short:
                        type = l_short;
                        break;
                    case k_int:
                        type = l_uint;
                        break;
                    case k_long:
                        type = l_ulong;
                        break;
                    default:
                        error(asts[1], "invalid unsigned * get_type");
                        break;
                    }
                    base_type = std::make_shared<type_base_t>(type);
                    base_type->line = asts[0]->line;
                    base_type->column = asts[0]->column;
                    asts.erase(asts.begin());
                    asts.erase(asts.begin());
                }
            }
            else {
                if (AST_IS_KEYWORD_N(asts[0], k_signed))
                    asts.erase(asts.begin());
                auto type = l_none;
                switch (asts[0]->data._keyword) {
                case k_char:
                    type = l_char;
                    break;
                case k_double:
                    type = l_double;
                    break;
                case k_float:
                    type = l_float;
                    break;
                case k_int:
                case k_void:
                    type = l_int;
                    break;
                case k_long:
                    type = l_long;
                    break;
                case k_short:
                    type = l_short;
                    break;
                default:
                    break;
                }
                if (type != l_none) {
                    base_type = std::make_shared<type_base_t>(type);
                    base_type->line = asts[0]->line;
                    base_type->column = asts[0]->column;
                    asts.erase(asts.begin());
                }
                else {
                    if (AST_IS_ID(asts[0])) {
                        auto& sym = symbols[0];
                        auto f = sym.find(asts[0]->data._string);
                        if (f != sym.end()) {
                            auto& typedef_name = f->second;
                            auto t = typedef_name->get_base_type();
                            if (t == s_type || t == s_struct || t == s_function) {
                                base_type = std::make_shared<type_typedef_t>(typedef_name);
                                base_type->line = asts[0]->line;
                                base_type->column = asts[0]->column;
                                asts.erase(asts.begin());
                            }
                            else {
                                error(asts[0], "invalid typedef name");
                            }
                        }
                        else {
                            error(asts[0], "unknown typedef name");
                        }
                    }
                    else {
                        error(asts[0], "invalid * get_type");
                    }
                }
            }
            if (!asts.empty()) {
                auto ptr = 0;
                for (auto& a : asts) {
                    if (AST_IS_OP_N(a, op_times)) {
                        ptr++;
                    }
                    else {
                        break;
                    }
                }
                base_type->ptr = ptr;
                asts.erase(asts.begin(), asts.begin() + ptr);
            }
#if LOG_TYPE
            log_out << "[DEBUG] Type: " << base_type->to_string() << std::endl;
#endif
            base_type->_static = _static;
            tmp.back().push_back(base_type);
        }
                                       break;
        case c_declarationSpecifiers2:
            break;
        case c_declarationSpecifier:
            break;
        case c_initDeclaratorList:
        case c_structDeclaratorList: {
            decltype(z_undefined) clazz;
            if (AST_IS_COLL_N(node, c_initDeclaratorList)) {
                if (ctx.lock()) {
                    clazz = z_local_var;
                }
                else {
                    clazz = z_global_var;
                }
            }
            else {
                clazz = z_struct_var;
            }
            type_t::ref type;
            {
                auto t = (tmp.rbegin() + 1)->back();
                if (t->get_base_type() == s_type) {
                    type = std::dynamic_pointer_cast<type_t>(t);
                    if (type->_static && clazz == z_local_var)
                        clazz = z_global_var;
                }
                else {
                    type = std::make_shared<type_typedef_t>(t);
                }
            }
            auto ptr = 0;
            auto tmp_i = 0;
            for (auto& ast : asts) {
                if (AST_IS_OP_N(ast, op_times)) {
                    ptr++;
                }
                else {
                    auto new_type = type->clone();
                    new_type->ptr = ptr;
                    auto _a = ast->next;
                    std::vector<int> matrix;
                    while (_a != ast) {
                        if (AST_IS_OP_N(_a, op_lsquare)) {
                            matrix.push_back(_a->next->data._int);
                            _a = _a->next;
                        }
                        else {
                            error(_a, "not support: ", true);
                        }
                        _a = _a->next;
                    }
                    if (!matrix.empty()) {
                        new_type->ptr += matrix.size();
                        new_type->matrix = matrix;
                    }
                    type_exp_t::ref init;
                    if (clazz != z_struct_var) {
                        auto t = tmp.back()[tmp_i++];
                        if (t) {
                            init = to_exp(t);
                        }
                    }
                    auto new_id = add_id(new_type, clazz, ast, init);
                    ptr = 0;
                }
            }
            tmp.back().clear();
        }
                                     break;
        case c_initDeclarator: {
            if (tmp.back().empty())
                tmp.back().push_back(nullptr);
        }
                               break;
        case c_storageClassSpecifier:
            break;
        case c_typeSpecifier:
            break;
        case c_structOrUnion:
            break;
        case c_structDeclarationList:
            break;
        case c_structDeclaration:
            break;
        case c_structDeclarator:
            break;
        case c_enumSpecifier:
            break;
        case c_enumeratorList:
            break;
        case c_enumerator:
            break;
        case c_enumerationConstant:
            break;
        case c_typeQualifier:
            break;
        case c_declarator:
            break;
        case c_directDeclarator: {
            if (AST_IS_OP_N(node->child->next, op_lparan)) {
                auto has_impl = AST_IS_COLL_N(node->parent->parent, c_functionDefinition);
                type_t::ref type;
                for (auto t = tmp.rbegin() + 1; t != tmp.rend(); t++) {
                    if (!t->empty()) {
                        auto tt = t->front();
                        if (tt && tt->get_base_type() == s_type) {
                            type = std::dynamic_pointer_cast<type_t>(t->front());
                            break;
                        }
                    }
                }
                assert(type);
                if (AST_IS_COLL_N(node->parent->child, c_pointer)) {
                    auto children = gen_get_children(node->parent->child->child);
                    for (auto& child : children) {
                        assert(AST_IS_OP_N(child, op_times));
                    }
                    type->ptr = children.size();
                }
                auto func = std::make_shared<sym_func_t>(type, nodes[0]->data._string);
                ctx = func;
                func->clazz = z_function;
                func->implemented = has_impl;
                func->addr = text.size();
                decltype(func) old_func;
                {
                    auto f = symbols[0].find(func->get_name());
                    if (f != symbols[0].end()) {
                        if (f->second->get_type() == s_function) {
                            old_func = std::dynamic_pointer_cast<sym_func_t>(f->second); // 假设为前置声明
                            if (!old_func->implemented)
                                symbols[0].erase(func->get_name());
                            else
                                error(asts[0], "conflict declaration in function: " + func->id +
                                    ", with: " + old_func->to_string());
                        }
                    }
                }
                symbols[0].insert(std::make_pair(nodes[0]->data._string, func));
                std::unordered_set<string_t> ids;
                auto ptr = 0;
                auto ellipsis = false;
                for (size_t i = 2, j = 0; i < asts.size() && j < tmp.size(); ++i) {
                    auto& pa = asts[i];
                    if (ellipsis) {
                        error(pa, "invalid param after ellipsis: ", true);
                    }
                    if (AST_IS_ID(pa)) {
                        const auto& pt = std::dynamic_pointer_cast<type_t>(tmp.back()[j]);
                        pt->ptr = ptr;
                        auto& name = pa->data._string;
                        auto id = std::make_shared<sym_id_t>(pt, name);
                        id->line = pa->line;
                        id->column = pa->column;
                        id->clazz = z_param_var;
                        allocate(id, nullptr);
                        func->params.push_back(id);
                        if (!ids.insert(name).second) {
                            error(id, "conflict id: " + id->to_string());
                        }
                        {
                            auto f = symbols[0].find(pa->data._string);
                            if (f != symbols[0].end()) {
                                if (f->second->get_type() == s_function) {
                                    error(id, "conflict argument in function: " + id->to_string());
                                }
                            }
                        }
                        ptr = 0;
                        j++;
                    }
                    else if (AST_IS_OP_N(pa, op_times)) {
                        ptr++;
                    }
                    else if (AST_IS_OP_N(pa, op_ellipsis)) {
                        ellipsis = true;
                    }
                    else {
                        error(pa, "invalid param: ", true);
                    }
                }
                func->ellipsis = ellipsis;
                func->ebp += sizeof(void*);
                func->ebp_local = func->ebp;
                for (auto& param : func->params) {
                    param->addr += 2 * sizeof(void*);
                    param->addr_end += 2 * sizeof(void*);
                }
#if LOG_TYPE
                log_out << "[DEBUG] Func: " << ctx.lock()->to_string() << std::endl;
#endif
                if (has_impl) {
                    if (old_func) {
                        {
                            if (old_func->params.size() != func->params.size()) {
                                error(asts[0], "conflict declaration in function: " + func->to_string() +
                                    ", with: " + old_func->to_string() + ", wrong params");
                            }
                            for (size_t i = 0; i < old_func->params.size(); i++) {
                                if (old_func->params[i]->base->to_string() != func->params[i]->base->to_string()) {
                                    error(asts[0], "conflict declaration in function: " + func->to_string() +
                                        ", with: " + old_func->to_string() + ", wrong param type, need: " +
                                        old_func->params[i]->to_string() + ", but got: " + func->params[i]->to_string());
                                }
                            }
                        }
                        for (auto& addr : old_func->write_backs) {
                            edit(addr, func->addr);
                        }
                        func_write_backs.erase(func->id);
                    }
                    tmp.back().push_back(func);
                    emit(ENT, 0);
                    func->entry = text.size() - 1;
                }
                else {
                    if (old_func) {
                        error(asts[0], "conflict declaration in function: " + func->to_string() +
                            ", with: " + old_func->to_string());
                    }
                    else {
                        func_write_backs.insert(func->id);
                    }
                    ctx.reset();
                }
                tmp.back().clear();
                asts.clear();
            }
            else if (AST_IS_OP_N(node->child->next, op_lsquare)) {
                if (asts.size() > 1)
                    asts.erase(asts.begin() + 1, asts.end());
            }
        }
                                 break;
        case c_pointer:
            break;
        case c_typeQualifierList:
            break;
        case c_parameterTypeList:
            break;
        case c_parameterList:
            break;
        case c_parameterDeclaration:
            break;
        case c_identifierList:
            break;
        case c_typeName: {
            if (!asts.empty()) {
                auto type = std::dynamic_pointer_cast<type_t>(tmp.back().front());
                for (auto& a : asts) {
                    if (AST_IS_OP_N(a, op_times)) {
                        type->ptr++;
                    }
                }
                asts.clear();
            }
        }
                         break;
        case c_abstractDeclarator:
            break;
        case c_directAbstractDeclarator:
            break;
        case c_typedefName:
            break;
        case c_initializer: {
            if (AST_IS_OP_N(nodes[0], op_lbrace)) {
                auto list = exp_list(tmp.back());
                tmp.back().clear();
                tmp.back().push_back(list);
                asts.clear();
            }
        }
                            break;
        case c_initializerList:
            break;
        case c_designation:
            break;
        case c_designatorList:
            break;
        case c_designator:
            break;
        case c_statement: {
            if (!tmp.back().empty()) {
                CStringA s;
                string_t page;
                int line;
                for (auto& _t : tmp.back()) {
                    auto j = _t->to_string();
                    if (get_line(_t->line, page, line)) {
                        s.Format("LOC: [%s:%d:%d], CODE: %s", page.c_str(), line, _t->column, _t->to_string().c_str());
                    }
                    else {
                        s.Format("LOC: [%d:%d], CODE: %s", _t->line, _t->column, _t->to_string().c_str());
                    }
                    pdbs.push_back(std::make_tuple(text.size(), s.GetBuffer(0)));
                    _t->gen_rvalue(*this);
                }
            }
        }
                          break;
        case c_labeledStatement: {
            if (AST_IS_KEYWORD_N(asts[0], k_case)) {
                cases.back().at(asts[0])._case = to_exp(tmp.back().front());
#if LOG_TYPE
                log_out << "[DEBUG] Case: " << cases.back().back()._case->to_string() << std::endl;
#endif
            }
            else if (AST_IS_KEYWORD_N(asts[0], k_default)) {
            }
            else if (AST_IS_ID(asts[0])) { // GOTO LABEL
                auto c = ctx.lock();
                if (!c || c->get_type() != s_function) {
                    error(asts[0], "invalid label: must be in function", true);
                }
                auto func = std::dynamic_pointer_cast<sym_func_t>(c);
                auto name = string_t(asts[0]->data._string);
                if (func->labels.find(name) != func->labels.end()) {
                    error(asts[0], "invalid label: conflict", true);
                }
                auto addr = labeled_id;
                func->labels.insert(std::make_pair(name, addr));
#if LOG_TYPE
                    log_out << "[DEBUG] Label: " << asts[0]->data._string << std::endl;
#endif
            }
            tmp.back().clear();
            asts.clear();
        }
                                 break;
        case c_compoundStatement: {
            symbols.pop_back();
            tmp.back().clear();
        }
                                  break;
        case c_blockItemList:
            break;
        case c_blockItem:
            break;
        case c_expressionStatement: {
#if LOG_TYPE
            log_out << "[DEBUG]  " << string_join(tmp.back(), ", ") << std::endl;
#endif
        }
                                    break;
        case c_selectionStatement:
            break;
        case c_iterationStatement:
            break;
        case c_forCondition:
            break;
        case c_forDeclaration:
            break;
        case c_forExpression:
            break;
        case c_jumpStatement: {
            auto& a = asts[0];
            if (AST_IS_KEYWORD(a)) {
                if (AST_IS_KEYWORD_K(a, k_return)) {
                    if (!tmp.back().empty()) {
                        auto exp = tmp.back().front();
                        if (exp->get_base_type() != s_expression) {
                            error(a, "return requires  ", true);
                        }
                        auto _exp = std::dynamic_pointer_cast<type_exp_t>(exp);
                        auto ctrl = std::make_shared<sym_ctrl_t>(a);
                        ctrl->exp = _exp;
                        tmp.back().clear();
                        tmp.back().push_back(ctrl);
                        asts.clear();
                    }
                    else {
                        auto ctrl = std::make_shared<sym_ctrl_t>(a);
                        tmp.back().clear();
                        tmp.back().push_back(ctrl);
                        asts.clear();
                    }
                }
                else if (AST_IS_KEYWORD_K(a, k_break) || AST_IS_KEYWORD_K(a, k_continue)) {
                    auto ctrl = std::make_shared<sym_ctrl_t>(a);
                    tmp.back().push_back(ctrl);
                    asts.clear();
                }
                else if (AST_IS_KEYWORD_K(a, k_interrupt)) {
                    auto ctrl = std::make_shared<sym_ctrl_t>(a);
                    ctrl->exp = primary_node(asts[1]);
                    tmp.back().push_back(ctrl);
                    asts.clear();
                }
                else if (AST_IS_KEYWORD_K(a, k_goto)) {
                    auto ctrl = std::make_shared<sym_ctrl_t>(a);
                    auto old = asts[1]->flag;
                    asts[1]->flag = ast_string;
                    ctrl->exp = primary_node(asts[1]);
                    asts[1]->flag = old;
                    tmp.back().push_back(ctrl);
                    asts.clear();
                }
                else {
                    error(a, "invalid jump: keyword", true);
                }
            }
            else {
                error(a, "invalid jump: op", true);
            }
        }
                              break;
        case c_pragmaStatement: {
            auto& m = asts[1];
            assert(m->flag == ast_string);
            auto s = string_t(m->data._string);
            if (s.substr(0, 5) == "note:") { // 代码页名
                incs.push_back(std::make_tuple(m->line, s.substr(5)));
            }
            else {
                error(m, "invalid pragma: " + s, true);
            }
            tmp.back().clear();
            asts.clear();
        }
            break;
        case c_compilationUnit:
            break;
        case c_translationUnit:
            break;
        case c_externalDeclaration: {
            ctx_stack.clear();
            ctx.reset();
        }
                                    break;
        case c_functionDefinition: {
            auto func = std::dynamic_pointer_cast<sym_func_t>(ctx.lock());
            func->gen_labels(*this);
            ctx_stack.clear();
            ctx.reset();
            symbols.pop_back();
            emit(LEV);
        }
                                   break;
        case c_structOrUnionSpecifier: {
            if (ctx.lock()->get_type() == s_struct)
                ctx.lock()->size(x_size);
            if (!ctx_stack.empty()) {
                ctx = ctx_stack.back();
                ctx_stack.pop_back();
            }
            symbols.pop_back();
        }
                                       break;
        case c_declarationList:
            break;
        }
    }

    void cgen::gen_stmt(const std::vector<ast_node*> & nodes, int level, ast_node * node) {
        auto& k = nodes[0];
        if (AST_IS_KEYWORD_K(k, k_if)) {
            gen_rec(nodes[1], level); // exp
            auto exp = exp_list(tmp.back());
            tmp.back().clear();
#if LOG_TYPE
            log_out << "[DEBUG] If: " << exp->to_string() << std::endl;
#endif
            exp->gen_rvalue(*this);
            emit(JZ, -1);
            auto L1 = (int)text.size() - 1;
            gen_rec(nodes[2], level); // true
            if (nodes.size() < 4) { // one branch
                text[L1] = (int)text.size();
            }
            else {
                emit(JMP, -1);
                auto L2 = (int)text.size() - 1;
                text[L1] = (int)text.size();
                gen_rec(nodes[4], level); // false
                text[L2] = (int)text.size();
            }
            tmp.back().clear();
        }
        else if (AST_IS_KEYWORD_K(k, k_while)) {
            auto& _exp = nodes[1];
            auto& _stmt = nodes[2];
            gen_rec(_exp, level); // exp
            auto exp = exp_list(tmp.back());
#if LOG_TYPE
            log_out << "[DEBUG] While: " << exp->to_string() << std::endl;
#endif
            tmp.back().clear();
            emit(JMP, (int)text.size() + 4);
            auto L1 = (int)text.size(); // break
            emit(JMP, -1);
            auto L2 = (int)text.size(); // continue
            exp->gen_rvalue(*this);
            emit(JZ, L1); // jump break
            cycle_t c{ L1, L2 };
            cycle.push_back(c);
            gen_rec(_stmt, level); // stmt
            emit(JMP, L2); // jump continue
            text[L2 - 1] = (int)text.size(); // jump exit
            tmp.back().clear();
            cycle.pop_back();
        }
        else if (AST_IS_KEYWORD_K(k, k_for)) {
            auto& _exp = nodes[1];
            auto& _stmt = nodes[2];
            auto _cond = gen_get_children(_exp->child);
            std::array<sym_t::ref, 3> _cond_exp;
            auto _cond_i = 0;
            for (auto& _c : _cond) {
                gen_rec(_c, level);
                if (!tmp.back().empty()) {
                    _cond_exp[_cond_i] = exp_list(tmp.back());
                    tmp.back().clear();
                }
                else {
                    _cond_i++;
                    ast.back().clear();
                }
            }
            if (_cond_exp[0]) { // init exp
#if LOG_TYPE
                log_out << "[DEBUG] For: init= " << sym_to_string(_cond_exp[0]) << std::endl;
#endif
                _cond_exp[0]->gen_rvalue(*this);
            }
            emit(JMP, (int)text.size() + 4);
            auto L1 = (int)text.size(); // break
            emit(JMP, -1);
            auto L2 = (int)text.size(); // continue
            if (_cond_exp[1]) { // cond exp
#if LOG_TYPE
                log_out << "[DEBUG] For: cond= " << sym_to_string(_cond_exp[1]) << std::endl;
#endif
                _cond_exp[1]->gen_rvalue(*this);
                emit(JZ, L1); // jump break
            }
            cycle_t c{ L1, L2 };
            cycle.push_back(c);
            gen_rec(_stmt, level); // stmt
            if (_cond_exp[2]) { // iter exp
#if LOG_TYPE
                log_out << "[DEBUG] For: iter= " << sym_to_string(_cond_exp[2]) << std::endl;
#endif
                _cond_exp[2]->gen_rvalue(*this);
            }
            emit(JMP, L2); // jump continue
            text[L2 - 1] = (int)text.size(); // jump exit
            tmp.back().clear();
            cycle.pop_back();
        }
        else if (AST_IS_KEYWORD_K(k, k_do)) {
            auto& _exp = nodes[3];
            auto& _stmt = nodes[1];
            gen_rec(_exp, level); // exp
            auto exp = exp_list(tmp.back());
#if LOG_TYPE
            log_out << "[DEBUG] Do-while: " << exp->to_string() << std::endl;
#endif
            tmp.back().clear();
            emit(JMP, -1);
            auto L1 = (int)text.size(); // break
            emit(JMP, -1);
            auto L2 = (int)text.size(); // continue
            exp->gen_rvalue(*this);
            emit(JZ, L1); // jump break
            cycle_t c{ L1, L2 };
            cycle.push_back(c);
            text[L1 - 1] = (int)text.size();
            gen_rec(_stmt, level); // stmt
            emit(JMP, L2); // jump continue
            text[L2 - 1] = (int)text.size(); // jump exit
            tmp.back().clear();
            cycle.pop_back();
        }
        else if (AST_IS_KEYWORD_K(k, k_switch)) {
            auto& _exp = nodes[1];
            auto& _stmt = nodes[2];
            gen_rec(_exp, level); // exp
            auto exp = exp_list(tmp.back());
#if LOG_TYPE
            log_out << "[DEBUG] Switch: " << exp->to_string() << std::endl;
#endif
            tmp.back().clear();
            emit(JMP, (int)text.size() + 4);
            auto L1 = (int)text.size(); // break
            emit(JMP, -1);
            exp->gen_rvalue(*this);
            auto c2 = cast_size(exp->base->get_cast());
            emit(PUSH, c2); // PUSH cond
            emit(JMP, -1); // jump cases
            auto L2 = (int)text.size(); // cases
            cycle_t c{ L1, -1 };
            cycle.push_back(c);
            cases.emplace_back();
            gen_rec(_stmt, level); // stmt
            emit(JMP, L1);
            text[L2 - 1] = text.size(); // jump cases
            auto _default = -1;
            auto & _cases = cases.back();
            std::vector<switch_t*> cc;
            for (auto& s : _cases)
                cc.push_back(&s.second);
            std::sort(cc.begin(), cc.end(), [](switch_t* a, switch_t* b) {
                return a->addr > b->addr;
                });
            for (size_t i = 0; i < cc.size(); ++i) {
                auto& _c = *(cc[i]);
                if (_c._case) {
                    _c._case->gen_rvalue(*this);
                    emit(CASE);
                    emit(JZ, _c.addr);
                }
                else if (_default == -1) {
                    _default = i;
                }
                else {
                    error(k, "conflict default: ", true);
                }
            }
            emit(POP, c2);
            if (_default != -1) {
                emit(JMP, cc[_default]->addr);
            }
            text[L1 + 1] = text.size(); // jump break
            tmp.back().clear();
            cases.pop_back();
            cycle.pop_back();
        }
        else {
            error(k, "invalid stmt keyword: ", true);
        }
    }

    void cgen::error(int x, int y, const string_t & str) const {
        std::stringstream ss;
        string_t page;
        int line;
        if (get_line(x, page, line)) {
            ss << "[" << page << ":" << line << ":" << y << "] ";
        }
        else {
            ss << "[" << x << ":" << y << "] ";
        }
        ss << str;
        throw cexception(ex_gen, ss.str());
    }

    bool cgen::get_line(int L, string_t& s, int& line) const
    {
        if (incs.empty())
            return false;
        for (size_t i = 0; i < incs.size(); i++) {
            auto& t = incs[i];
            if (std::get<0>(t) > L) {
                s = std::get<1>(incs[i - 1]);
                line = L - std::get<0>(incs[i - 1]);
                return true;
            }
        }
        s = std::get<1>(incs.back());
        line = L - std::get<0>(incs.back());
        return true;
    }

    void cgen::error(ast_node * node, const string_t & str, bool info) const {
        std::stringstream ss;
        string_t page;
        int line;
        if (get_line(node->line, page, line)) {
            ss << "[" << page << ":" << line << ":" << node->column << "] ";
        }
        else {
            ss << "[" << node->line << ":" << node->column << "] ";
        }
        ss << str;
        if (info) {
            cast::print(node, 0, ss);
        }
        throw cexception(ex_gen, ss.str());
    }

    void cgen::error(sym_t::ref s, const string_t & str) const {
        std::stringstream ss;
        string_t page;
        int line;
        if (get_line(s->line, page, line)) {
            ss << "[" << page << ":" << line << ":" << s->column << "] ";
        }
        else {
            ss << "[" << s->line << ":" << s->column << "] ";
        }
        ss << str;
        throw cexception(ex_gen, ss.str());
    }

    type_exp_t::ref cgen::to_exp(sym_t::ref s) {
        if (s->get_base_type() != s_expression)
            error(s, "need expression: " + s->to_string());
        return std::dynamic_pointer_cast<type_exp_t>(s);
    }

    void cgen::allocate(sym_id_t::ref id, const type_exp_t::ref & init, int delta) {
        if (id->base->get_base_type() != s_type)
            error(id, "need type: " + id->to_string());
        auto type = id->base;
        if (id->clazz == z_global_var) {
            id->addr = data.size();
            auto size = align4(type->size(x_size));
            if (init) {
                if (init->get_type() == s_var) {
                    auto var = std::dynamic_pointer_cast<sym_var_t>(init);
                    auto& node = var->node;
                    if (node->flag != ast_string) {
                        std::copy((char*)& node->data._ins,
                            ((char*)& node->data._ins) + size,
                            std::back_inserter(data));
                        if (delta > 0) {
                            *(((int*)(data.data() + data.size())) - 1) += delta;
                        }
                    }
                    else {
                        load_string(node->data._string);
                    }
                }
                else if (init->get_type() == s_var_id) {
                    auto var = std::dynamic_pointer_cast<sym_var_id_t>(init);
                    auto _id = var->id.lock();
                    if (_id->get_type() != s_id)
                        error(id, "allocate: invalid init value");
                    auto var2 = std::dynamic_pointer_cast<sym_id_t>(_id);
                    std::copy(data.data() + var2->addr,
                        data.data() + var2->addr_end,
                        std::back_inserter(data));
                    if (delta > 0) {
                        *(((int*)(data.data() + data.size())) - 1) += delta;
                    }
                }
                else if (init->get_type() == s_unop) {
                    auto v1 = std::dynamic_pointer_cast<sym_unop_t>(init);
                    if (AST_IS_OP_N(v1->op, op_minus) && v1->exp->get_type() == s_var) {
                        auto var = std::dynamic_pointer_cast<sym_var_t>(v1->exp);
                        auto& node = var->node;
                        if (node->flag != ast_string) {
                            std::copy((char*)& node->data._ins,
                                ((char*)& node->data._ins) + size,
                                std::back_inserter(data));
                            switch ((ast_t)node->flag) {
                            case ast_char:
                            case ast_short:
                            case ast_int:
                                *(((int*)(data.data() + data.size())) - 1) = -*(((int*)(data.data() + data.size())) - 1);
                                if (delta > 0) {
                                    *(((int*)(data.data() + data.size())) - 1) += delta;
                                }
                                break;
                            case ast_long:
                                *(((int64*)(data.data() + data.size())) - 1) = -*(((int64*)(data.data() + data.size())) - 1);
                                if (delta > 0) {
                                    *(((int64*)(data.data() + data.size())) - 1) += (int64)delta;
                                }
                                break;
                            case ast_float:
                                *(((float*)(data.data() + data.size())) - 1) = -*(((float*)(data.data() + data.size())) - 1);
                                if (delta > 0) {
                                    *(((float*)(data.data() + data.size())) - 1) += (float)delta;
                                }
                                break;
                            case ast_double:
                                *(((double*)(data.data() + data.size())) - 1) = -*(((double*)(data.data() + data.size())) - 1);
                                if (delta > 0) {
                                    *(((double*)(data.data() + data.size())) - 1) += (double)delta;
                                }
                                break;
                            default:
                                error(id, "allocate: unop not supported");
                                break;
                            }
                        }
                        else {
                            error(id, "allocate: unop not supported string");
                        }
                    }
                    else {
                        error(id, "allocate: unop not supported");
                    }
                }
                else if (init->get_type() == s_list) {
                    auto list = std::dynamic_pointer_cast<sym_list_t>(init);
                    if (id->base->matrix.empty())
                        error(id, "allocate: need array type");
                    if (id->base->matrix[0] == 0)
                        id->base->matrix[0] = (int)list->exps.size();
                    if (id->base->matrix[0] != (int)list->exps.size())
                        error(id, "allocate: array size not equal");
                    auto old_ptr = id->base->ptr;
                    if (id->base->ptr > 0)id->base->ptr--;
                    auto c = id->get_cast(); // GET TYPE
                    id->base->ptr = old_ptr;
                    std::vector<std::tuple<size_t, string_t>> string_writeback;
                    for (auto& exp : list->exps) {
                        if (exp->get_type() == s_unop) {
                            auto v1 = std::dynamic_pointer_cast<sym_unop_t>(exp);
                            if (AST_IS_OP_N(v1->op, op_minus) && v1->exp->get_type() == s_var) {
                                auto var = std::dynamic_pointer_cast<sym_var_t>(v1->exp);
                                auto& node = var->node;
                                if (node->flag != ast_string) {
                                    std::copy((char*)& node->data._ins,
                                        ((char*)& node->data._ins) + var->base->size(x_size),
                                        std::back_inserter(data));
                                    switch ((ast_t)node->flag) {
                                    case ast_char:
                                    case ast_short:
                                    case ast_int:
                                        *(((int*)(data.data() + data.size())) - 1) = -*(((int*)(data.data() + data.size())) - 1);
                                        break;
                                    case ast_long:
                                        *(((int64*)(data.data() + data.size())) - 1) = -*(((int64*)(data.data() + data.size())) - 1);
                                        break;
                                    case ast_float:
                                        *(((float*)(data.data() + data.size())) - 1) = -*(((float*)(data.data() + data.size())) - 1);
                                        break;
                                    case ast_double:
                                        *(((double*)(data.data() + data.size())) - 1) = -*(((double*)(data.data() + data.size())) - 1);
                                        break;
                                    default:
                                        error(id, "allocate: unop not supported");
                                        break;
                                    }
                                }
                                else {
                                    error(id, "allocate: array item not support string");
                                }
                            }
                            else {
                                error(id, "allocate: array item unop not supported");
                            }
                        }
                        else {
                            if (exp->get_type() != s_var)
                                error(exp, "allocate: array item exp not equal");
                            if (exp->get_cast() != c)
                                error(exp, "allocate: array item type not equal, required: " +
                                    id->base->to_string() + ", but got: " + exp->base->to_string());
                            auto var = std::dynamic_pointer_cast<sym_var_t>(exp);
                            auto & node = var->node;
                            if (node->flag != ast_string) {
                                std::copy((char*)& node->data._ins,
                                    ((char*)& node->data._ins) + align4(var->base->size(x_size)),
                                    std::back_inserter(data));
                            }
                            else {
                                string_writeback.push_back(std::make_tuple(data.size(), node->data._string));
                                std::copy((char*)& node->data._string, (char*)& node->data._string + sizeof(void*), std::back_inserter(data));
                            }
                        }
                    }
                    for (const auto& swr : string_writeback) {
                        uint32 addr = load_string(std::get<1>(swr));
                        *(uint32*)& data[std::get<0>(swr)] = DATA_BASE | addr;
                    }
                }
                else {
                    error(id, "allocate: not supported");
                }
            }
            else {
                for (auto i = 0; i < size; ++i) {
                    data.push_back(0);
                }
            }
            id->addr_end = data.size();
        }
        else if (id->clazz == z_local_var) {
            auto size = align4(type->size(x_size));
            auto func = std::dynamic_pointer_cast<sym_func_t>(ctx.lock());
            func->ebp_local += size;
            text[func->entry] += size;
            id->addr = func->ebp - func->ebp_local;
            id->addr_end = id->addr - size;
            if (init) {
                if (init->get_type() == s_list) {
                    auto list = std::dynamic_pointer_cast<sym_list_t>(init);
                    if (id->base->matrix.empty())
                        error(id, "allocate: need array type");
                    if (id->base->matrix[0] == 0)
                        id->base->matrix[0] = (int)list->exps.size();
                    if (id->base->matrix[0] != (int)list->exps.size())
                        error(id, "allocate: array size not equal");
                    auto old_ptr = id->base->ptr;
                    if (id->base->ptr > 0)id->base->ptr--;
                    auto c = id->get_cast(); // GET TYPE
                    id->base->ptr = old_ptr;
                    // L_VALUE
                    // LOOP:
                    // PUSH
                    // PUSH
                    // R_VALUE
                    // SAVE
                    // IMM SIZE
                    // ADD
                    id->gen_lvalue(*this);
                    for (auto& exp : list->exps) {
                        emit(PUSH, cast_size(t_ptr));
                        emit(PUSH, cast_size(t_ptr));
                        exp->gen_rvalue(*this);
                        if (exp->get_cast() != c)
                            error(init, "allocate: array item type not equal, required: " +
                                id->base->to_string() + ", but got: " + exp->base->to_string());
                        auto s = align4(exp->base->size(x_size));
                        emit(SAVE, s);
                        emit(IMM, s);
                        emit(ADD, t_ptr);
                    }
                }
                else {
                    // L_VALUE
                    // PUSH
                    // R_VALUE
                    // SAVE
                    id->gen_lvalue(*this);
                    emit(PUSH, cast_size(t_ptr));
                    init->gen_rvalue(*this);
                    if (delta > 0) {
                        emit(PUSH, cast_size(t_ptr));
                        emit(IMM, delta);
                        emit(ADD);
                    }
                    if (type->get_cast() != init->get_cast())
                        error(init, "allocate: not equal init type, need: " + type->to_string() +
                            ", but got: " + init->to_string());
                    emit(SAVE, size);
                }
            }
        }
        else if (id->clazz == z_param_var) {
            if (init)
                error(id, "allocate: invalid init value");
            auto size = align4(type->size(x_size));
            auto func = std::dynamic_pointer_cast<sym_func_t>(ctx.lock());
            id->addr = func->ebp;
            id->addr_end = func->ebp + size;
            func->ebp += size;
        }
        else if (id->clazz == z_struct_var) {
            if (init)
                error(id, "allocate: invalid init value");
            auto _struct = std::dynamic_pointer_cast<sym_struct_t>(ctx.lock());
            if (!_struct)
                error(id, "allocate: need struct");
            _struct->decls.push_back(id);
        }
        else {
            error(id, "allocate: not supported");
        }
    }

    sym_id_t::ref cgen::add_id(const type_base_t::ref & type, sym_class_t clazz,
        ast_node * node, const type_exp_t::ref & init, int delta) {
        assert(AST_IS_ID(node));
        auto new_id = std::make_shared<sym_id_t>(type, node->data._string);
        new_id->line = node->line;
        new_id->column = node->column;
        new_id->clazz = clazz;
        new_id->init = init;
        if (init && init->base && type->get_cast() != init->get_cast()) {
            error(node, "id: not equal init type, required: " + type->to_string() +
                ", but got: " + init->base->to_string() + ", ", true);
        }
        allocate(new_id, init, delta);
#if LOG_TYPE
        log_out << "[DEBUG] Id: " << new_id->to_string() << std::endl;
#endif
        if (!symbols.back().insert(std::make_pair(node->data._string, new_id)).second) {
            error(new_id, "conflict id: " + new_id->to_string());
        }
        if (symbols.size() > 1) {
            auto f = symbols[0].find(node->data._string);
            if (f != symbols[0].end()) {
                if (f->second->get_type() == s_function) {
                    error(new_id, "conflict id with function: " + new_id->to_string());
                }
            }
        }
        return new_id;
    }

    sym_t::ref cgen::find_symbol(ast_node* node) {
        auto name = string_t(node->data._string);
        for (auto i = (int)symbols.size() - 1; i > 0; i--) {
            auto f = symbols[i].find(name);
            if (f != symbols[i].end()) {
                return f->second;
            }
        }
        if (ctx.lock()) {
            auto _ctx = ctx.lock();
            if (_ctx->get_type() == s_function) {
                auto func = std::dynamic_pointer_cast<sym_func_t>(_ctx);
                for (auto& param : func->params) {
                    if (param->get_name() == name) {
                        return param;
                    }
                }
            }
        }
        if (!symbols.empty()) {
            auto f = symbols[0].find(name);
            if (f != symbols[0].end()) {
                return f->second;
            }
        }
        auto f2 = macros.find(name);
        if (f2 != macros.end()) {
            switch (f2->second) {
            case m_line:
            {
                auto t = std::make_shared<type_base_t>(l_int, 0);
                macro_data.emplace_back();
                auto& d = macro_data.back();
                d.insert(d.end(), sizeof(ast_node), '\0');
                auto n = (ast_node*)& d.front();
                CopyMemory(n, node, sizeof(ast_node));
                n->flag = ast_int;
                string_t page; int L;
                if (get_line(n->line, page, L)) {
                    n->data._int = L;
                }
                else {
                    n->data._int = n->line;
                }
                return std::make_shared<sym_var_t>(t, n);
            }
            case m_column:
            {
                auto t = std::make_shared<type_base_t>(l_int, 0);
                macro_data.emplace_back();
                auto& d = macro_data.back();
                d.insert(d.end(), sizeof(ast_node), '\0');
                auto n = (ast_node*)& d.front();
                CopyMemory(n, node, sizeof(ast_node));
                n->flag = ast_int;
                n->data._int = n->column;
                return std::make_shared<sym_var_t>(t, n);
            }
            case m_func:
            {
                if (ctx.lock()) {
                    auto _ctx = ctx.lock();
                    if (_ctx->get_type() == s_function) {
                        auto func = std::dynamic_pointer_cast<sym_func_t>(_ctx);
                        auto t = std::make_shared<type_base_t>(l_char, 1);
                        macro_data.emplace_back();
                        auto& d = macro_data.back();
                        d.insert(d.end(), sizeof(ast_node), '\0');
                        auto n = (ast_node*)& d.front();
                        CopyMemory(n, node, sizeof(ast_node));
                        n->flag = ast_string;
                        macro_data.emplace_back();
                        auto& d2 = macro_data.back();
                        std::copy(func->id.begin(), func->id.end(), std::back_inserter(d2));
                        d2.push_back('\0');
                        auto n2 = (char*)& d2[0];
                        n->data._string = n2;
                        return std::make_shared<sym_var_t>(t, n);
                    }
                }
            }
            case m_file:
            {
                auto t = std::make_shared<type_base_t>(l_char, 1);
                macro_data.emplace_back();
                auto& d = macro_data.back();
                d.insert(d.end(), sizeof(ast_node), '\0');
                auto n = (ast_node*)& d.front();
                CopyMemory(n, node, sizeof(ast_node));
                n->flag = ast_string;
                macro_data.emplace_back();
                auto& d2 = macro_data.back();
                std::copy(page.begin(), page.end(), std::back_inserter(d2));
                d2.push_back('\0');
                auto n2 = (char*)& d2[0];
                n->data._string = n2;
                return std::make_shared<sym_var_t>(t, n);
            }
            default:
                error(node, "invalid macro: " + name);
            }
        }
        return nullptr;
    }

    sym_var_t::ref cgen::primary_node(ast_node * node) {
        type_t::ref t;
        switch (node->flag) {
        case ast_literal: {
            if (AST_IS_COLL_N(node->parent, c_postfixExpression) &&
                (AST_IS_OP_N(node->prev, op_dot) || AST_IS_OP_N(node->prev, op_pointer))) {
                t = std::make_shared<type_base_t>(l_int, 0);
                return std::make_shared<sym_var_t>(t, node);
            }
            auto sym = find_symbol(node);
            if (!sym)
                error(node, "undefined id: " + string_t(node->data._string));
            if (sym->get_type() == s_id || sym->get_type() == s_function) {
                t = std::dynamic_pointer_cast<sym_id_t>(sym)->base->clone();
                return std::make_shared<sym_var_id_t>(t, node, sym);
            }
            if (sym->get_type() == s_struct) {
                t = std::dynamic_pointer_cast<type_typedef_t>(sym);
                return std::make_shared<sym_var_id_t>(t, node, sym);
            }
            if (sym->get_type() == s_var) {
                return std::dynamic_pointer_cast<sym_var_t>(sym);
            }
            error(node, "required id but got: " + sym->to_string());
        }
        case ast_string:
            t = std::make_shared<type_base_t>(l_char, 1);
            break;
#define DEF_LEX(name) \
            case ast_##name: \
                t = std::make_shared<type_base_t>(l_##name, 0); \
                break;
            DEF_LEX(char);
            DEF_LEX(uchar);
            DEF_LEX(short);
            DEF_LEX(ushort);
            DEF_LEX(int);
            DEF_LEX(uint);
            DEF_LEX(long);
            DEF_LEX(ulong);
            DEF_LEX(float);
            DEF_LEX(double);
#undef DEF_LEX
        case ast_keyword: {
            if (AST_IS_KEYWORD_K(node, k_true) || AST_IS_KEYWORD_K(node, k_false))
                t = std::make_shared<type_base_t>(l_int, 0);
            else
                error(node, "invalid var keyword type: ", true);
        }
                          break;
        default:
            error(node, "invalid var type: ", true);
            break;
        }
        return std::make_shared<sym_var_t>(t, node);
    }

    std::tuple<sym_class_t, string_t> sym_class_string_list[] = {
        std::make_tuple(z_undefined, "undefined"),
        std::make_tuple(z_global_var, "global id"),
        std::make_tuple(z_local_var, "local id"),
        std::make_tuple(z_param_var, "param id"),
        std::make_tuple(z_struct_var, "struct id"),
        std::make_tuple(z_function, "func id"),
    };

    const string_t& sym_class_string(sym_class_t t) {
        assert(t >= z_undefined && t < z_end);
        return std::get<1>(sym_class_string_list[t]);
    }

    string_t unnamed_gen(int id) {
        std::stringstream ss;
        ss << "_unnamed_" << id << "_";
        return ss.str();
    }

    backtrace_direction cgen::check(pda_edge_t edge, ast_node * node) {
        if (edge != e_shift) { // CONTAINS reduce, recursion, move
            if (AST_IS_COLL(node)) {
                switch (node->data._coll) {
                case c_structOrUnionSpecifier: { // MODIFY, CANNOT RECOVERY
                    if (AST_IS_ID(node->child->next)) {
                        auto _struct = node->child->child->data._keyword;
                        auto id = node->child->next->data._string;
                        auto& sym = symbols[0];
                        if (sym.find(id) != sym.end()) {
                            // CONFLICT STRUCT DECLARATION
                            return b_fail;
                        }
                        sym.insert(std::make_pair(id, std::make_shared<sym_struct_t>(_struct == k_struct, id)));
                    }
                }
                                               break;
                case c_typedefName: { // READONLY
                    if (AST_IS_ID(node->child)) {
                        auto id = node->child->next->data._string;
                        auto& sym = symbols[0];
                        auto f = sym.find(id);
                        if (f != sym.end()) {
                            auto t = f->second->get_base_type();
                            if (t == s_type || t == s_struct || t == s_function)
                                return b_next;
                        }
                        return b_error;
                    }
                }
                                    break;
                default:
                    break;
                }
            }
        }
        return b_next;
    }

    void cgen::error_handler(int state, const std::vector<pda_trans>& trans, int& jump)
    {
    }
}
