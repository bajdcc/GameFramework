//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBcjsunit_H
#define CLIBcjsunit_H

#include <string>
#include <unordered_set>
#include <vector>
#include <map>
#include <array>
#include <bitset>
#include "cjsmem.h"
#include "cjstypes.h"

namespace clib {

    using namespace types;

    class js_unit_builder;

    struct js_unit {
        js_unit_t t{ u_none };
        js_unit* next{ nullptr };
        js_unit* prev{ nullptr };
        js_unit_builder* builder{ nullptr };

        js_unit& operator=(const js_unit& u);
        js_unit& operator+(const js_unit& u);
        js_unit& operator|(const js_unit& u);
        js_unit& operator*();
        js_unit& operator~();
        js_unit& operator()(void* cb);
        js_unit& init(js_unit_builder* builder);
        js_unit& set_t(js_unit_t type);
    };

    struct js_unit_token : public js_unit {
        js_lexer_t type{ NONE };
        bool LA{ false };

        js_unit_token& set_type(js_lexer_t type);
        js_unit_token& set_LA(bool b);
    };

    struct js_unit_collection : public js_unit {
        bool skip{ false };
        bool marked{ false };
        js_unit* child{ nullptr };
        void* callback{ nullptr };

        js_unit_collection& set_skip(bool skip);
        js_unit_collection& set_marked(bool skip);
        js_unit_collection& set_child(js_unit* node);
        js_unit_collection& set_cb(void* cb);
    };

    struct js_unit_rule : public js_unit_collection {
        const char* s{ nullptr };
        uint32_t attr{ 0 };

        js_unit_rule& set_s(const char* str);
        js_unit_rule& set_attr(uint32_t attr);
    };

    struct js_nga_edge;
    struct js_nga_edge_list;

    struct js_nga_status {
        const char* label{ nullptr };
        bool final{ false };
        js_nga_edge_list* in{ nullptr }, * out{ nullptr };
    };

    struct js_pda_status : public js_nga_status {
        int rule;
    };

    struct js_nga_edge {
        js_nga_status* begin{ nullptr }, * end{ nullptr };
        bool skip{ false };
        bool marked{ false };
        void* cb{ nullptr };
        js_unit* data{ nullptr };
    };

    enum js_pda_edge_t {
        e_shift,
        e_pass,
        e_rule,
        e_move,
        e_left_recursion,
        e_left_recursion_not_greed,
        e_reduce,
        e_reduce_exp,
        e_finish,
    };

    struct js_pda_edge : public js_nga_edge {
        js_pda_edge_t type{ e_finish };
    };

    struct js_nga_edge_list {
        js_nga_edge_list* prev{ nullptr }, * next{ nullptr };
        js_nga_edge* edge{ nullptr };
    };

    js_unit_rule* js_to_rule(js_unit* u);
    js_unit_token* js_to_token(js_unit* u);
    js_unit_collection* js_to_collection(js_unit* u);
    js_unit_collection* js_to_ref(js_unit* u);
    const std::string& js_pda_edge_str(js_pda_edge_t type);
    const int& js_pda_edge_priority(js_pda_edge_t type);

    class js_unit_builder {
    public:
        virtual js_unit_collection& append(js_unit* collection, js_unit* child) = 0;
        virtual js_unit_collection& merge(js_unit* a, js_unit* b) = 0;
        virtual js_unit_collection& collection(js_unit* a, js_unit* b, js_unit_t type) = 0;
        virtual js_unit_collection& optional(js_unit* a) = 0;
        virtual js_unit* copy(js_unit* u) = 0;

        virtual js_nga_edge* enga(js_unit* node, bool init) = 0;
        virtual js_nga_edge* enga(js_unit* node, js_unit* u) = 0;
        virtual js_nga_edge* connect(js_nga_status* a, js_nga_status* b, bool is_pda) = 0;
    };

    struct js_nga_rule {
        int id;
        js_unit_rule* u;
        js_nga_status* status;
        int recursive;
        std::unordered_set<js_unit_token*> tokensList;
        std::unordered_set<js_unit_token*> tokensFirstset;
        std::unordered_set<js_unit_rule*> rulesFirstset;
    };

    struct js_pda_trans {
        int jump;
        js_pda_edge_t type;
        int status;
        std::string label;
        std::vector<js_unit*> LA;
        std::shared_ptr<std::array<std::shared_ptr<std::vector<js_unit*>>, RULE_END - RULE_START - 1>> rule;
        bool marked;
        int cost;
        void* pred;
        void* cb;
        int reduced_rule;
    };

    struct js_pda_rule {
        int id;
        int rule;
        bool final;
        bool pred;
        bool cb;
        js_coll_t coll;
        std::string label;
        std::vector<js_pda_trans> trans;
    };

    enum js_pda_rule_attr {
        r_normal = 0,
        r_not_greed = 1,
        r_exp = 2,
    };

    enum js_pda_coll_pred {
        p_ALLOW,
        p_DELAY,
        p_REMOVE,
    };

    struct js_pda_coll_t {
        js_unit* r;
        js_unit* a;
        js_pda_edge_t ea;
        int cost;
        void* pred;
    };

    struct js_pda_coll2_t {
        js_unit* r;
        js_unit* a;
        js_pda_edge_t ea;
        int cost;
    };

    // 文法表达式
    class cjsunit : public js_unit_builder {
    public:
        cjsunit() = default;
        ~cjsunit() = default;

        cjsunit(const cjsunit&) = delete;
        cjsunit& operator=(const cjsunit&) = delete;

        js_unit& token(const js_lexer_t& type, bool LA = false);
        js_unit& rule(const std::string& s, js_coll_t t, uint32_t attr = 0);

        js_unit_collection& append(js_unit* collection, js_unit* child) override;
        js_unit_collection& merge(js_unit* a, js_unit* b) override;
        js_unit_collection& collection(js_unit* a, js_unit* b, js_unit_t type) override;
        js_unit_collection& optional(js_unit* a) override;
        js_unit* copy(js_unit* u) override;

        js_nga_edge* enga(js_unit* node, bool init) override;
        js_nga_edge* enga(js_unit* node, js_unit* u) override;
        js_nga_edge* connect(js_nga_status* a, js_nga_status* b, bool is_pda) override;

        std::vector<js_pda_rule>& get_pda();
        const std::vector<js_pda_rule>& get_pda() const;

        void adjust(js_unit* r, js_unit* a, js_pda_edge_t ea, int cost, void* pred = nullptr);

    private:
        js_nga_status* status();
        js_pda_status* status(const char* label, int rule, bool final);
        void add_edge(js_nga_edge_list*& list, js_nga_edge* edge);
        void remove_edge(js_nga_edge_list*& list, js_nga_edge* edge);
        void remove_edge(js_nga_edge* edge);
        const char* label(js_unit* focused, bool front);
        void label(js_unit* node, js_unit* parent, js_unit* focused, bool front, std::ostream& os);
        void disconnect(js_nga_status* status);

    public:
        void gen(js_unit* root);
        void dump(std::ostream& os);

    private:
        void gen_nga();
        void check_nga();
        void gen_pda(js_unit* root);

        static js_nga_edge* conv_nga(js_unit* u);
        js_nga_status* delete_epsilon(js_nga_edge* edge);

        void error(const std::string&);
        std::string print_unit(js_unit*);

    private:
        const char* str(const std::string& s);

    private:
        cjsmem nodes;
        std::unordered_set<std::string> strings;
        std::vector<std::string> labels;
        std::map<std::string, js_nga_rule> rules;
        std::unordered_map<const char*, js_coll_t> rulesMap;
        std::vector<js_pda_rule> pdas;
        js_unit_rule* current_rule{ nullptr };
        std::vector<js_pda_coll_t> adjusts;
    };
}

#endif //CLIBcjsunit_H
