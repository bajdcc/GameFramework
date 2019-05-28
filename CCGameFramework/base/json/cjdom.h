//
// Project: clibjson
// Created by CC
//

#ifndef CLIBJSON_CDOM_H
#define CLIBJSON_CDOM_H

#include "cjast.h"

namespace clib {

    // 对象访问
    class cdom {
    public:
        explicit cdom(cast_json *ast, ast_node_json *node);
        ~cdom() = default;

        cdom(const cdom &) = default;
        cdom &operator=(const cdom &) = default;

        cdom operator[](int idx) const;
        cdom operator[](string_t idx) const;

        void output(std::ostream &os) const;

#define DEFINE_NODE_ASSIGN(t) cdom &operator=(const LEX_T(t) &);
        DEFINE_NODE_ASSIGN(char)
        DEFINE_NODE_ASSIGN(uchar)
        DEFINE_NODE_ASSIGN(short)
        DEFINE_NODE_ASSIGN(ushort)
        DEFINE_NODE_ASSIGN(int)
        DEFINE_NODE_ASSIGN(uint)
        DEFINE_NODE_ASSIGN(long)
        DEFINE_NODE_ASSIGN(ulong)
        DEFINE_NODE_ASSIGN(float)
        DEFINE_NODE_ASSIGN(double)
        DEFINE_NODE_ASSIGN(string)
#undef DEFINE_NODE_ASSIGN

    private:
        cast_json *ast{nullptr};
        ast_node_json *node{nullptr};
    };

    std::ostream &operator<<(std::ostream &os, const cdom &dom);
}

#endif //CLIBJSON_CDOM_H
