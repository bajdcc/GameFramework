//
// Project: clibparser
// Created by CC
//

#ifndef CLIBPARSER_CUNIT_H
#define CLIBPARSER_CUNIT_H

#include <string>
#include <unordered_set>
#include <vector>
#include <map>
#include <bitset>
#include "memory.h"

#define UNIT_NODE_MEM (32 * 1024)

namespace clib {

    enum unit_t {
        u_none,
        u_token,
        u_token_ref,
        u_rule,
        u_rule_ref,
        u_sequence,
        u_branch,
        u_optional,
    };

    class unit_builder;

    struct unit {
        unit_t t{ u_none };
        unit* next{ nullptr };
        unit* prev{ nullptr };
        unit_builder* builder{ nullptr };

        unit& operator=(const unit& u);
        unit& operator+(const unit& u);
        unit& operator|(const unit& u);
        unit& operator*();
        unit& operator~();
        unit& init(unit_builder* builder);
        unit& set_t(unit_t type);
    };

    struct unit_token : public unit {
        lexer_t type{ l_none };
        union {
            operator_t op;
            keyword_t keyword;
        } value;

        unit_token& set_type(lexer_t type);
        unit_token& set_op(operator_t op);
        unit_token& set_keyword(keyword_t keyword);
    };

    struct unit_collection : public unit {
        bool skip{ false };
        bool marked{ false };
        unit* child{ nullptr };

        unit_collection& set_skip(bool skip);
        unit_collection& set_marked(bool skip);
        unit_collection& set_child(unit* node);
    };

    struct unit_rule : public unit_collection {
        const char* s{ nullptr };
        uint32 attr{ 0 };

        unit_rule& set_s(const char* str);
        unit_rule& set_attr(uint32 attr);
    };

    struct nga_edge;
    struct nga_edge_list;

    struct nga_status {
        const char* label{ nullptr };
        bool final{ false };
        nga_edge_list* in{ nullptr }, * out{ nullptr };
    };

    struct pda_status : public nga_status {
        int rule;
    };

    struct nga_edge {
        nga_status* begin{ nullptr }, * end{ nullptr };
        bool skip{ false };
        bool marked{ false };
        unit* data{ nullptr };
    };

    enum pda_edge_t {
        e_shift,
        e_pass,
        e_move,
        e_left_recursion,
        e_left_recursion_not_greed,
        e_reduce,
        e_reduce_exp,
        e_finish,
    };

    struct pda_edge : public nga_edge {
        pda_edge_t type{ e_finish };
    };

    struct nga_edge_list {
        nga_edge_list* prev{ nullptr }, * next{ nullptr };
        nga_edge* edge{ nullptr };
    };

    unit_rule* to_rule(unit* u);
    unit_token* to_token(unit* u);
    unit_collection* to_collection(unit* u);
    unit_collection* to_ref(unit* u);
    const string_t& pda_edge_str(pda_edge_t type);
    const int& pda_edge_priority(pda_edge_t type);

    class unit_builder {
    public:
        virtual unit_collection& append(unit* collection, unit* child) = 0;
        virtual unit_collection& merge(unit* a, unit* b) = 0;
        virtual unit_collection& collection(unit* a, unit* b, unit_t type) = 0;
        virtual unit_collection& optional(unit* a) = 0;
        virtual unit* copy(unit* u) = 0;

        virtual nga_edge* enga(unit* node, bool init) = 0;
        virtual nga_edge* enga(unit* node, unit* u) = 0;
        virtual nga_edge* connect(nga_status* a, nga_status* b, bool is_pda = false) = 0;
    };

    struct nga_rule {
        int id;
        unit_rule* u;
        nga_status* status;
        int recursive;
        std::unordered_set<unit_token*> tokensList;
        std::unordered_set<unit_token*> tokensFirstset;
        std::unordered_set<unit_rule*> rulesFirstset;
    };

    struct pda_trans {
        int jump;
        pda_edge_t type;
        int status;
        string_t label;
        std::vector<unit*> LA;
        bool marked;
    };

    struct pda_rule {
        int id;
        int rule;
        bool final;
        coll_t coll;
        string_t label;
        std::vector<pda_trans> trans;
    };

    enum pda_rule_attr {
        r_normal = 0,
        r_not_greed = 1,
        r_exp = 2,
    };

    // 文法表达式
    class cunit : public unit_builder {
    public:
        cunit() = default;
        ~cunit() = default;

        cunit(const cunit&) = delete;
        cunit& operator=(const cunit&) = delete;

        unit& token(const lexer_t& type);
        unit& token(const operator_t& op);
        unit& token(const keyword_t& keyword);
        unit& rule(const string_t& s, coll_t t, uint32 attr = 0);

        unit_collection& append(unit* collection, unit* child) override;
        unit_collection& merge(unit* a, unit* b) override;
        unit_collection& collection(unit* a, unit* b, unit_t type) override;
        unit_collection& optional(unit* a) override;
        unit* copy(unit* u) override;

        nga_edge* enga(unit* node, bool init) override;
        nga_edge* enga(unit* node, unit* u) override;
        nga_edge* connect(nga_status* a, nga_status* b, bool is_pda = false) override;

        const std::vector<pda_rule>& get_pda() const;

    private:
        nga_status* status();
        pda_status* status(const char* label, int rule, bool final);
        void add_edge(nga_edge_list*& list, nga_edge* edge);
        void remove_edge(nga_edge_list*& list, nga_edge_list* edge);
        const char* label(unit* focused, bool front);
        void label(unit* node, unit* parent, unit* focused, bool front, std::ostream& os);
        void disconnect(nga_status* status);

    public:
        void gen(unit* root);
        void dump(std::ostream& os);

    private:
        void gen_nga();
        void check_nga();
        void gen_pda(unit* root);

        static nga_edge* conv_nga(unit* u);
        nga_status* delete_epsilon(nga_edge* edge);

        void error(const string_t&);
        string_t print_unit(unit*);

    private:
        const char* str(const string_t& s);

    private:
        memory_pool<UNIT_NODE_MEM> nodes;
        std::unordered_set<std::string> strings;
        std::vector<std::string> labels;
        std::map<std::string, nga_rule> rules;
        std::unordered_map<const char*, coll_t> rulesMap;
        std::vector<pda_rule> pdas;
        unit_rule* current_rule{ nullptr };
    };
};


#endif //CLIBPARSER_CUNIT_H
