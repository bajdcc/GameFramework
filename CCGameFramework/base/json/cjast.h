//
// Project: clibjson
// Created by bajdcc
//

#ifndef CLIBJSON_CAST_H
#define CLIBJSON_CAST_H

#include "../parser2d/memory.h"

#define JSON_AST_NODE_MEM (32 * 1024)
#define JSON_AST_STR_MEM (16 * 1024)

namespace clib {

    enum ast_json_t {
        ast_json_root,
        ast_json_obj,
        ast_json_pair,
        ast_json_list,
        ast_json_string,
        ast_json_char,
        ast_json_uchar,
        ast_json_short,
        ast_json_ushort,
        ast_json_int,
        ast_json_uint,
        ast_json_long,
        ast_json_ulong,
        ast_json_float,
        ast_json_double,
    };

    enum ast_to_json_t {
        to_json_parent,
        to_json_prev,
        to_json_next,
        to_json_child,
    };

    // 结点
    struct ast_node_json {
        // 类型
        uint32_t flag;

        union {
#define DEFINE_NODE_DATA(t) LEX_T(t) _##t;
            DEFINE_NODE_DATA(char)
            DEFINE_NODE_DATA(uchar)
            DEFINE_NODE_DATA(short)
            DEFINE_NODE_DATA(ushort)
            DEFINE_NODE_DATA(int)
            DEFINE_NODE_DATA(uint)
            DEFINE_NODE_DATA(long)
            DEFINE_NODE_DATA(ulong)
            DEFINE_NODE_DATA(float)
            DEFINE_NODE_DATA(double)
#undef DEFINE_NODE_DATA
            const char *_string;
        } data; // 数据

        // 树型数据结构，广义表
        ast_node_json *parent; // 父亲
        ast_node_json *prev; // 左兄弟
        ast_node_json *next; // 右兄弟
        ast_node_json *child; // 最左儿子
    };

    class cast_json {
    public:
        cast_json();
        ~cast_json() = default;

        cast_json(const cast_json&) = delete;
        cast_json&operator=(const cast_json&) = delete;

        ast_node_json *get_root() const;

        ast_node_json *new_node(ast_json_t type);
        ast_node_json *new_child(ast_json_t type, bool step = true);
        ast_node_json *new_sibling(ast_json_t type, bool step = true);

        ast_node_json *add_child(ast_node_json*);
        static ast_node_json *set_child(ast_node_json*, ast_node_json*);
        static ast_node_json *set_sibling(ast_node_json*, ast_node_json*);
        static int children_size(ast_node_json*);

        void set_id(ast_node_json *node, const string_t &str);
        void set_str(ast_node_json *node, const string_t &str);
        static std::string display_str(ast_node_json *node);

        void to(ast_to_json_t type);

        static void print(ast_node_json *node, int level, std::ostream &os);

        static ast_node_json *index(ast_node_json *node, int index);
        static ast_node_json *index(ast_node_json *node, const string_t &index);

        void reset();
    private:
        void init();

    private:
        memory_pool<JSON_AST_NODE_MEM> nodes; // 全局AST结点内存管理
        memory_pool<JSON_AST_STR_MEM> strings; // 全局字符串管理
        ast_node_json *root; // 根结点
        ast_node_json *current; // 当前结点
    };
}

#endif //CLIBJSON_CAST_H
