//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSGEN_H
#define CLIBJS_CJSGEN_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include "cjsast.h"
#include <base\nlohmann_json\json.h>

#define LAMBDA_ID "<lambda>"

namespace clib {

    enum js_symbol_t {
        s_sym,
        s_id,
        s_function,
        s_var,
        s_var_id,
        s_expression,
        s_unop,
        s_sinop,
        s_binop,
        s_triop,
        s_member_dot,
        s_member_index,
        s_expression_seq,
        s_array,
        s_object,
        s_object_pair,
        s_new,
        s_call_method,
        s_call_function,
        s_ctrl,
        s_case,
        s_statement,
        s_statement_var,
        s_statement_exp,
        s_statement_return,
        s_statement_throw,
        s_statement_control,
        s_statement_if,
        s_statement_while,
        s_statement_for,
        s_statement_for_in,
        s_statement_switch,
        s_statement_try,
        s_block,
        s_code,
    };

    class js_sym_t;

    class js_sym_code_t;

    class js_sym_var_id_t;

    class ijsgen {
    public:
        virtual void emit(js_ast_node_index *, js_ins_t) = 0;
        virtual void emit(js_ast_node_index *, js_ins_t, int) = 0;
        virtual void emit(js_ast_node_index *, js_ins_t, int, int) = 0;
        virtual int get_ins(int, int = 0) const = 0;
        virtual int code_length() const = 0;
        virtual void edit(int, int, int) = 0;
        virtual int load_number(double d) = 0;
        virtual int load_string(const std::string &, int) = 0;
        virtual int push_function(std::shared_ptr<js_sym_code_t>) = 0;
        virtual void pop_function() = 0;
        virtual void enter(int, std::shared_ptr<js_sym_t> = nullptr) = 0;
        virtual void leave() = 0;
        virtual void push_rewrites(int index, int type) = 0;
        virtual const std::unordered_map<int, int> &get_rewrites() = 0;
        virtual std::shared_ptr<js_sym_t> get_var(const std::string &, int) = 0;
        virtual void add_var(const std::string &, std::shared_ptr<js_sym_t>) = 0;
        virtual void add_closure(std::shared_ptr<js_sym_var_id_t>) = 0;
        virtual int get_func_level() const = 0;
        virtual std::string get_func_name() const = 0;
        virtual std::string get_fullname(const std::string &name) const = 0;
        virtual bool is_arrow_func() const = 0;
        virtual std::string get_code_text(js_ast_node_index *) const = 0;
        virtual std::string get_filename() const = 0;
        virtual void gen_try(int) = 0;
        virtual void error(js_ast_node_index *, const std::string &) const = 0;
    };

    class js_sym_t : public js_ast_node_index, public std::enable_shared_from_this<js_sym_t> {
    public:
        using ref = std::shared_ptr<js_sym_t>;
        using weak_ref = std::weak_ptr<js_sym_t>;
        virtual js_symbol_t get_type() const;
        virtual js_symbol_t get_base_type() const;
        virtual std::string to_string() const;
        virtual int gen_lvalue(ijsgen &gen);
        virtual int gen_rvalue(ijsgen &gen);
        virtual int gen_invoke(ijsgen &gen, ref &list);
        virtual int set_parent(ref node);
        weak_ref parent;
    };

    enum js_sym_lvalue_t {
        no_lvalue,
        can_be_lvalue,
    };

    class js_sym_exp_t : public js_sym_t {
    public:
        using ref = std::shared_ptr<js_sym_exp_t>;
        js_symbol_t get_type() const override;
        js_symbol_t get_base_type() const override;
    };

    class js_sym_var_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_var_t>;
        explicit js_sym_var_t(js_ast_node *node);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        enum class_t {
            local,
            fast,
            closure,
            global,
        } clazz{local};
        js_ast_node *node{nullptr};
    };

    class js_sym_var_id_t : public js_sym_var_t {
    public:
        using ref = std::shared_ptr<js_sym_var_id_t>;
        explicit js_sym_var_id_t(js_ast_node *node, const js_sym_t::ref &symbol);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int gen_invoke(ijsgen &gen, js_sym_t::ref &list) override;
        js_sym_t::weak_ref id;
    private:
        void init_id(ijsgen &gen);
    };

    class js_sym_id_t : public js_sym_t {
    public:
        using ref = std::shared_ptr<js_sym_id_t>;
        js_symbol_t get_type() const override;
        js_symbol_t get_base_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int gen_rvalue_decl(ijsgen &gen);
        int set_parent(js_sym_t::ref node) override;
        void parse();
        std::vector<js_sym_var_t::ref> ids;
        js_sym_exp_t::ref init;
    };

    class js_sym_unop_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_unop_t>;
        explicit js_sym_unop_t(js_sym_exp_t::ref exp, js_ast_node *op);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        js_ast_node *op{nullptr};
    };

    class js_sym_sinop_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_sinop_t>;
        explicit js_sym_sinop_t(js_sym_exp_t::ref exp, js_ast_node *op);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        js_ast_node *op{nullptr};
    };

    class js_sym_binop_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_binop_t>;
        explicit js_sym_binop_t(js_sym_exp_t::ref exp1, js_sym_exp_t::ref exp2, js_ast_node *op);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp1, exp2;
        js_ast_node *op{nullptr};
    };

    class js_sym_triop_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_triop_t>;
        explicit js_sym_triop_t(js_sym_exp_t::ref exp1, js_sym_exp_t::ref exp2,
                             js_sym_exp_t::ref exp3, js_ast_node *op1, js_ast_node *op2);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp1, exp2, exp3;
        js_ast_node *op1{nullptr}, *op2{nullptr};
    };

    class js_sym_member_dot_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_member_dot_t>;
        explicit js_sym_member_dot_t(js_sym_exp_t::ref exp);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        std::vector<js_ast_node *> dots;
    };

    class js_sym_member_index_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_member_index_t>;
        explicit js_sym_member_index_t(js_sym_exp_t::ref exp);
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_lvalue(ijsgen &gen) override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        std::vector<js_sym_exp_t::ref> indexes;
    };

    class js_sym_exp_seq_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_exp_seq_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        std::vector<js_sym_exp_t::ref> exps;
    };

    class js_sym_array_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_array_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        std::vector<js_sym_exp_t::ref> exps;
        std::vector<int> rests;
    };

    class js_sym_object_pair_t : public js_sym_t {
    public:
        using ref = std::shared_ptr<js_sym_object_pair_t>;
        js_symbol_t get_type() const override;
        js_symbol_t get_base_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref key, value;
    };

    class js_sym_new_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_new_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref obj;
        std::vector<js_sym_exp_t::ref> args;
        std::vector<int> rests;
    };

    class js_sym_object_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_object_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        std::vector<bool> is_pair;
        std::vector<js_sym_object_pair_t::ref> pairs;
        std::vector<js_sym_exp_t::ref> rests;
    };

    class js_sym_call_method_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_call_method_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref obj;
        js_ast_node *method{nullptr};
        std::vector<js_sym_exp_t::ref> args;
        std::vector<int> rests;
    };

    class js_sym_call_function_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_call_function_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref obj;
        std::vector<js_sym_exp_t::ref> args;
        std::vector<int> rests;
    };

    class js_sym_stmt_t : public js_sym_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_t>;
        js_symbol_t get_type() const override;
        js_symbol_t get_base_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
    };

    class js_sym_stmt_var_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_var_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        std::vector<js_sym_id_t::ref> vars;
    };

    class js_sym_stmt_exp_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_exp_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_seq_t::ref seq;
    };

    class js_sym_stmt_return_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_return_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_seq_t::ref seq;
    };

    class js_sym_stmt_throw_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_throw_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_seq_t::ref seq;
    };

    class js_sym_stmt_control_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_control_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        int keyword{0};
        js_sym_var_t::ref label;
    };

    class js_sym_stmt_if_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_if_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_seq_t::ref seq;
        js_sym_stmt_t::ref true_stmt;
        js_sym_stmt_t::ref false_stmt;
    };

    class js_sym_stmt_while_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_while_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_seq_t::ref seq;
        js_sym_stmt_t::ref stmt;
        bool do_while{false};
    };

    class js_sym_stmt_for_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_for_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        js_sym_stmt_var_t::ref vars;
        js_sym_exp_t::ref cond;
        js_sym_exp_t::ref iter;
        js_sym_stmt_t::ref body;
    };

    class js_sym_stmt_for_in_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_for_in_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        js_sym_id_t::ref vars;
        js_sym_exp_t::ref iter;
        js_sym_stmt_t::ref body;
    };

    class js_sym_case_t : public js_sym_t {
    public:
        using ref = std::shared_ptr<js_sym_case_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        std::vector<js_sym_stmt_t::ref> stmts;
    };

    class js_sym_stmt_switch_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_switch_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_exp_t::ref exp;
        std::vector<js_sym_case_t::ref> cases;
    };

    class js_sym_stmt_try_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_stmt_try_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        js_sym_stmt_t::ref try_body;
        js_sym_var_t::ref var;
        js_sym_stmt_t::ref catch_body;
        js_sym_stmt_t::ref finally_body;
    };

    class js_sym_block_t : public js_sym_stmt_t {
    public:
        using ref = std::shared_ptr<js_sym_block_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        std::vector<js_sym_stmt_t::ref> stmts;
        js_sym_id_t::ref var;
    };

    class js_sym_code_t;

    class cjs_consts {
    public:
        enum get_string_t {
            gs_string,
            gs_name,
            gs_global,
            gs_deref,
            gs_regex,
        };
        int get_number(double n);
        int get_string(const std::string &str, get_string_t type);
        int get_function(std::shared_ptr<js_sym_code_t> code);
        std::string get_desc(int n) const;
        js_runtime_t get_type(int n) const;
        char *get_data(int n) const;
        const char *get_name(int n) const;
        const char *get_global(int n) const;
        void dump(const std::string *text) const;
        void save();
        const std::vector<char *> &get_consts_data() const;
        const std::vector<const char *> &get_names_data() const;
        const std::vector<const char *> &get_globals_data() const;
        const std::vector<const char *> &get_derefs_data() const;
        void conv_funcs(const std::unordered_map<std::shared_ptr<js_sym_code_t>, int>& funcs);
        void restore_funcs(const std::vector<std::shared_ptr<js_sym_code_t>>& funcs);
        void to_json(nlohmann::json&) const;
        void from_json(const nlohmann::json&);
    private:
        std::unordered_map<double, int> numbers;
        std::unordered_map<std::string, int> strings;
        std::unordered_map<std::string, int> regexes;
        std::unordered_map<std::string, int> globals;
        std::unordered_map<std::string, int> derefs;
        std::unordered_map<std::string, int> names;
        std::unordered_map<int, std::weak_ptr<js_sym_code_t>> functions;
        std::unordered_map<int, int> function_ids;
        std::vector<js_runtime_t> consts;
        std::vector<char *> consts_data;
        std::vector<const char *> names_data;
        std::vector<const char *> globals_data;
        std::vector<const char *> derefs_data;
        int index{0};
    };

    void copy_info(js_sym_t::ref dst, js_sym_t::ref src);
    void copy_info(js_sym_t::ref dst, js_ast_node *src);

    enum cjs_scope_t {
        sp_none,
        sp_block,
        sp_param,
        sp_for,
        sp_for_each,
        sp_while,
        sp_do_while,
        sp_switch,
        sp_try,
        sp_catch,
        sp_finally,
    };

    enum cjs_scope_query_t {
        sq_local,
        sq_local_func,
        sq_all,
    };

    struct cjs_scope {
        cjs_scope_t type;
        js_sym_t::ref sym;
        std::unordered_map<std::string, js_sym_t::weak_ref> vars;
        std::unordered_map<int, int> rewrites;
    };

    struct cjs_code {
        int line, column, start, end;
        int code, opnum, op1, op2;
        std::string desc;
    };

    class js_sym_code_t : public js_sym_exp_t {
    public:
        using ref = std::shared_ptr<js_sym_code_t>;
        using weak_ref = std::weak_ptr<js_sym_code_t>;
        js_symbol_t get_type() const override;
        std::string to_string() const override;
        int gen_rvalue(ijsgen &gen) override;
        int set_parent(js_sym_t::ref node) override;
        void to_json(nlohmann::json&) const;
        void from_json(const nlohmann::json&);
        js_ast_node *name{nullptr};
        bool arrow{false};
        bool rest{false};
        std::string fullName;
        std::string simpleName;
        std::string debugName;
        std::string text;
        std::vector<js_sym_var_t::ref> args;
        std::vector<std::string> args_str;
        js_sym_t::ref body;
        cjs_consts consts;
        std::vector<cjs_scope> scopes;
        std::vector<cjs_code> codes;
        std::vector<js_sym_var_id_t::ref> closure;
        std::unordered_set<std::string> closure_str;
    };

    struct cjs_code_result {
        using ref = std::shared_ptr<cjs_code_result>;
        js_sym_code_t::ref code;
        std::vector<js_sym_code_t::ref> funcs;
    };

    class cjsgen : public ijsgen {
    public:
        cjsgen();
        ~cjsgen() = default;

        cjsgen(const cjsgen &) = delete;
        cjsgen &operator=(const cjsgen &) = delete;

        bool gen_code(js_ast_node *node, const std::string *str, const std::string &name);
        cjs_code_result::ref get_code() const;

        static void print(const js_sym_t::ref &node, int level, std::ostream &os);

        void emit(js_ast_node_index *, js_ins_t) override;
        void emit(js_ast_node_index *, js_ins_t, int) override;
        void emit(js_ast_node_index *, js_ins_t, int, int) override;
        int get_ins(int, int = 0) const override;
        int code_length() const override;
        void edit(int, int, int) override;
        int load_number(double d) override;
        int load_string(const std::string &, int) override;
        int push_function(std::shared_ptr<js_sym_code_t>) override;
        void pop_function() override;
        void enter(int, js_sym_t::ref = nullptr) override;
        void leave() override;
        void push_rewrites(int index, int type) override;
        const std::unordered_map<int, int> &get_rewrites() override;
        std::shared_ptr<js_sym_t> get_var(const std::string &, int) override;
        void add_var(const std::string &, std::shared_ptr<js_sym_t>) override;
        void add_closure(std::shared_ptr<js_sym_var_id_t>) override;
        int get_func_level() const override;
        std::string get_fullname(const std::string &name) const override;
        std::string get_func_name() const override;
        bool is_arrow_func() const override;
        std::string get_code_text(js_ast_node_index *) const override;
        std::string get_filename() const override;
        void gen_try(int) override;
        void error(js_ast_node_index *, const std::string &) const override;

    private:
        void gen_rec(js_ast_node *node, int level);
        void gen_coll(const std::vector<js_ast_node *> &nodes, int level, js_ast_node *node);
        bool gen_before(const std::vector<js_ast_node *> &nodes, int level, js_ast_node *node);
        void gen_after(const std::vector<js_ast_node *> &nodes, int level, js_ast_node *node);

        void error(js_ast_node *, const std::string &, bool info = false) const;
        void error(const js_sym_t::ref& s, const std::string &) const;

        js_sym_exp_t::ref to_exp(const js_sym_t::ref& s);
        js_sym_stmt_t::ref to_stmt(const js_sym_t::ref& s);

        js_sym_t::ref find_symbol(js_ast_node *node);
        js_sym_var_t::ref primary_node(js_ast_node *node);

        void dump() const;
        void dump(js_sym_code_t::ref, bool print) const;

        js_ast_node* conv2str(js_ast_node*);

    private:
        const std::string *text{nullptr};
        std::string filename;
        std::vector<std::string> err;
        std::vector<std::vector<js_ast_node *>> ast;
        std::vector<std::vector<js_sym_t::ref>> tmp;
        std::vector<js_sym_code_t::ref> codes;
        std::vector<js_sym_code_t::ref> funcs;
        std::vector<std::string> reserveWords;
    };
}

#endif //CLIBJS_CJSGEN_H
