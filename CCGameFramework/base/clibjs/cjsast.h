//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSAST_H
#define CLIBJS_CJSAST_H

#include <cstdint>
#include "cjstypes.h"
#include "cjsmem.h"

namespace clib {

    using namespace types;

    struct js_ast_node_index {
        int line{0}, column{0}, start{0}, end{0};
    };

    // 结点
    struct js_ast_node : public js_ast_node_index {
        // 类型
        uint16_t flag{0};
        uint16_t attr{0};

        union {
            double _number;
            const char *_string;
            const char *_regex;
            const char *_identifier;
            js_lexer_t _keyword;
            js_lexer_t _op;
            js_coll_t _coll;
            struct {
                uint32_t _1, _2;
            } _ins;
        } data; // 数据

        // 树型数据结构，广义表
        js_ast_node *parent{nullptr}; // 父亲
        js_ast_node *prev{nullptr}; // 左兄弟
        js_ast_node *next{nullptr}; // 右兄弟
        js_ast_node *child{nullptr}; // 最左儿子
    };

    class cjsast {
    public:
        cjsast();
        ~cjsast() = default;

        cjsast(const cjsast &) = delete;
        cjsast &operator=(const cjsast &) = delete;

        js_ast_node *get_root() const;

        js_ast_node *new_node(js_ast_t type);
        js_ast_node *new_child(js_ast_t type, bool step = true);
        js_ast_node *new_sibling(js_ast_t type, bool step = true);

        void remove(js_ast_node *);

        js_ast_node *add_child(js_ast_node *);
        static js_ast_node *set_child(js_ast_node *, js_ast_node *);
        static js_ast_node *set_sibling(js_ast_node *, js_ast_node *);
        static int children_size(js_ast_node *);

        void set_str(js_ast_node *node, const std::string &str);
        static std::string display_str(const char *str);

        void to(js_ast_to_t type);

        static void print(js_ast_node *node, int level, const std::string &text, std::ostream &os);
        static std::string to_string(js_ast_node *node);
        static bool ast_equal(js_ast_t type, js_lexer_t lex);

        static void unlink(js_ast_node *node);

        static js_ast_node *index(js_ast_node *node, int index);
        static js_ast_node *index(js_ast_node *node, const std::string &index);

        void reset();

    private:
        void init();

    private:
        cjsmem nodes; // 全局AST结点内存管理
        cjsmem strings; // 全局字符串管理
        js_ast_node *root{nullptr}; // 根结点
        js_ast_node *current{nullptr}; // 当前结点
    };
};


#endif //CLIBJS_CJSAST_H
