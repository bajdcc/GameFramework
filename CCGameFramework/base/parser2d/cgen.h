//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_CGEN_H
#define CLIBPARSER_CGEN_H

#include <vector>
#include <memory>
#include "cast.h"
#include "cparser.h"
#include "cvm.h"

namespace clib {

    enum symbol_t {
        s_sym,
        s_type,
        s_type_base,
        s_type_typedef,
        s_id,
        s_struct,
        s_function,
        s_var,
        s_var_id,
        s_expression,
        s_cast,
        s_unop,
        s_sinop,
        s_binop,
        s_triop,
        s_list,
        s_ctrl,
        s_statement,
    };

    enum cast_t {
        t_char,
        t_uchar,
        t_short,
        t_ushort,
        t_int,
        t_uint,
        t_long,
        t_ulong,
        t_float,
        t_double,
        t_ptr,
        t_struct,
        t_error,
    };

    enum gen_t {
        g_ok,
        g_error,
        g_no_load,
    };

    class igen {
    public:
        virtual void emit(ins_t) = 0;
        virtual void emit(ins_t, int) = 0;
        virtual void emit(ins_t, int, int) = 0;
        virtual void emit(int, int, keyword_t) = 0;
        virtual int current() const = 0;
        virtual void edit(int, int) = 0;
        virtual int load_string(const string_t&) = 0;
        virtual void add_label(int, int, int, const string_t&) = 0;
        virtual void error(int, int, const string_t&) const = 0;
    };

    enum sym_size_t {
        x_load,
        x_size,
        x_inc,
        x_matrix,
    };

    class sym_t {
    public:
        using ref = std::shared_ptr<sym_t>;
        using weak_ref = std::weak_ptr<sym_t>;
        virtual symbol_t get_type() const;
        virtual symbol_t get_base_type() const;
        virtual int size(sym_size_t t, int level = 0) const;
        virtual string_t get_name() const;
        virtual string_t to_string() const;
        virtual gen_t gen_lvalue(igen& gen);
        virtual gen_t gen_rvalue(igen& gen);
        virtual gen_t gen_invoke(igen& gen, ref& list);
        virtual cast_t get_cast() const;
        int line{ 0 }, column{ 0 };
    };

    class type_t : public sym_t {
    public:
        using ref = std::shared_ptr<type_t>;
        explicit type_t(int ptr = 0);
        symbol_t get_type() const override;
        symbol_t get_base_type() const override;
        virtual ref clone() const;
        int ptr;
        bool _static{ false };
        std::vector<int> matrix;
    };

    class type_base_t : public type_t {
    public:
        explicit type_base_t(lexer_t type, int ptr = 0);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        cast_t get_cast() const override;
        type_t::ref clone() const override;
        lexer_t type;
    };

    class type_typedef_t : public type_t {
    public:
        explicit type_typedef_t(const sym_t::ref& refer, int ptr = 0);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t to_string() const override;
        cast_t get_cast() const override;
        ref clone() const override;
        sym_t::weak_ref refer;
    };

    enum sym_class_t {
        z_undefined,
        z_global_var,
        z_local_var,
        z_param_var,
        z_struct_var,
        z_function,
        z_end,
    };

    const string_t& sym_class_string(sym_class_t);

    class type_exp_t : public sym_t {
    public:
        using ref = std::shared_ptr<type_exp_t>;
        explicit type_exp_t(const type_t::ref& base);
        symbol_t get_type() const override;
        symbol_t get_base_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        gen_t gen_invoke(igen& gen, sym_t::ref& list) override;
        cast_t get_cast() const override;
        type_t::ref base;
    };

    class sym_id_t : public sym_t {
    public:
        using ref = std::shared_ptr<sym_id_t>;
        explicit sym_id_t(const type_t::ref& base, const string_t& id);
        symbol_t get_type() const override;
        symbol_t get_base_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        cast_t get_cast() const override;
        type_t::ref base;
        type_exp_t::ref init;
        string_t id;
        sym_class_t clazz{ z_undefined };
        int addr{ 0 };
        int addr_end{ 0 };
    };

    class sym_struct_t : public sym_t {
    public:
        using ref = std::shared_ptr<sym_struct_t>;
        explicit sym_struct_t(bool _struct, const string_t& id);
        symbol_t get_type() const override;
        symbol_t get_base_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        cast_t get_cast() const override;
        bool _struct{ true };
        string_t id;
        int _size{ 0 };
        std::vector<sym_id_t::ref> decls;
    };

    class sym_func_t : public sym_id_t {
    public:
        using ref = std::shared_ptr<sym_func_t>;
        explicit sym_func_t(const type_t::ref& base, const string_t& id);
        symbol_t get_type() const override;
        symbol_t get_base_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t to_string() const override;
        gen_t gen_invoke(igen& gen, sym_t::ref& list) override;
        void gen_labels(igen& gen);
        cast_t get_cast() const override;
        std::vector<sym_id_t::ref> params;
        int ebp{ 0 }, ebp_local{ 0 };
        int entry{ 0 };
        bool implemented{ false };
        bool ellipsis{ false };
        std::vector<int> write_backs;
        std::unordered_map<string_t, int> labels;
        std::vector<std::tuple<int, string_t, int, int>> labels_writeback;
    };

    class sym_var_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_var_t>;
        explicit sym_var_t(const type_t::ref& base, ast_node* node);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        cast_t get_cast() const override;
        ast_node* node{ nullptr };
    };

    class sym_var_id_t : public sym_var_t {
    public:
        using ref = std::shared_ptr<sym_var_id_t>;
        explicit sym_var_id_t(const type_t::ref& base, ast_node* node, const sym_t::ref& symbol);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        gen_t gen_invoke(igen& gen, sym_t::ref& list) override;
        cast_t get_cast() const override;
        sym_t::weak_ref id;
    };

    class sym_cast_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_cast_t>;
        explicit sym_cast_t(const type_exp_t::ref& exp, const type_t::ref& base);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        type_exp_t::ref exp;
    };

    class sym_unop_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_unop_t>;
        explicit sym_unop_t(const type_exp_t::ref& exp, ast_node* op);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        type_exp_t::ref exp;
        ast_node* op{ nullptr };
    };

    class sym_sinop_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_sinop_t>;
        explicit sym_sinop_t(const type_exp_t::ref& exp, ast_node* op);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        type_exp_t::ref exp;
        ast_node* op{ nullptr };
    };

    class sym_binop_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_binop_t>;
        explicit sym_binop_t(const type_exp_t::ref& exp1, const type_exp_t::ref& exp2, ast_node* op);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        type_exp_t::ref exp1, exp2;
        ast_node* op{ nullptr };
    };

    class sym_triop_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_triop_t>;
        explicit sym_triop_t(const type_exp_t::ref& exp1, const type_exp_t::ref& exp2,
            const type_exp_t::ref& exp3, ast_node* op1, ast_node* op2);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        type_exp_t::ref exp1, exp2, exp3;
        ast_node* op1{ nullptr }, * op2{ nullptr };
    };

    class sym_list_t : public type_exp_t {
    public:
        using ref = std::shared_ptr<sym_list_t>;
        sym_list_t();
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        std::vector<type_exp_t::ref> exps;
    };

    class sym_ctrl_t : public sym_t {
    public:
        using ref = std::shared_ptr<sym_ctrl_t>;
        explicit sym_ctrl_t(ast_node* op);
        symbol_t get_type() const override;
        int size(sym_size_t t, int level = 0) const override;
        string_t get_name() const override;
        string_t to_string() const override;
        gen_t gen_lvalue(igen& gen) override;
        gen_t gen_rvalue(igen& gen) override;
        type_exp_t::ref exp;
        ast_node* op{ nullptr };
    };

    struct cycle_t {
        int _break;
        int _continue;
    };

    struct switch_t {
        type_exp_t::ref _case;
        int addr;
    };

    struct PDB_ADDR {
        uint idx;
        uint addr;
    };

    struct PDB {
        uint code_len;
        char* data;
    };

    struct PE {
        char magic[4];
        uint entry;
        uint data_len;
        uint text_len;
        uint pdb_len;
        uint pdb2_len;
        byte data;
        // byte *data;
        // byte *text;
        // PDB pdb;
        // PDB pdb2;
    };

    // 生成虚拟机指令
    class cgen : public csemantic, public igen {
    public:
        cgen();
        ~cgen() = default;

        cgen(const cgen&) = delete;
        cgen& operator=(const cgen&) = delete;

        backtrace_direction check(pda_edge_t, ast_node*) override;
        void error_handler(int, const std::vector<pda_trans>&, int&) override;

        void gen(const string_t& page, ast_node* node);
        void reset();
        std::vector<byte> file() const;

        void emit(ins_t) override;
        void emit(ins_t, int) override;
        void emit(ins_t, int, int) override;
        void emit(int, int, keyword_t) override;
        int current() const override;
        void edit(int, int) override;
        int load_string(const string_t&) override;
        void add_label(int, int, int, const string_t&) override;
        void error(int, int, const string_t&) const override;
    private:
        void gen_rec(ast_node* node, int level);
        void gen_coll(const std::vector<ast_node*>& nodes, int level, ast_node* node);
        void gen_stmt(const std::vector<ast_node*>& nodes, int level, ast_node* node);

        void allocate(sym_id_t::ref id, const type_exp_t::ref& init, int delta = 0);
        sym_id_t::ref add_id(const type_base_t::ref&, sym_class_t, ast_node*, const type_exp_t::ref&, int = 0);

        sym_t::ref find_symbol(ast_node* node);
        sym_var_t::ref primary_node(ast_node* node);

        bool get_line(int, string_t&, int&) const;
        void error(ast_node*, const string_t&, bool info = false) const;
        void error(sym_t::ref s, const string_t&) const;

        sym_list_t::ref exp_list(const std::vector<sym_t::ref>& exps);

        type_exp_t::ref to_exp(sym_t::ref s);

    private:
        enum macro_t {
            m_none,
            m_line,
            m_column,
            m_func,
            m_file,
        };
        std::unordered_map<string_t, macro_t> macros;
        std::list<std::vector<char>> macro_data;
        void init_macro();

        const string_t& coll_str(coll_t);

    public:
        static std::stringstream log_out;

    private:
        std::vector<LEX_T(int)> text; // 代码
        std::vector<LEX_T(char)> data; // 数据
        std::vector<std::unordered_map<LEX_T(string), std::shared_ptr<sym_t>>> symbols; // 符号表
        std::vector<std::vector<ast_node*>> ast;
        std::vector<std::vector<sym_t::ref>> tmp;
        std::vector<cycle_t> cycle;
        std::vector<std::unordered_map<ast_node*, switch_t>> cases;
        std::unordered_set<string_t> func_write_backs;
        sym_t::weak_ref ctx;
        std::vector<sym_t::ref> ctx_stack;
        int global_id{ 0 };
        std::vector<std::tuple<int, string_t>> incs;
        std::vector<std::tuple<int, string_t>> pdbs;
        int labeled_id{ -1 };
        string_t page;
    };
}

#endif //CLIBPARSER_CGEN_H
