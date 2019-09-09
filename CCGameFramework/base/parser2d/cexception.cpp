//
// Project: clibparser
// Author: bajdcc
//

#include "stdafx.h"
#include <cassert>
#include "cexception.h"

namespace clib {

    std::tuple<ex_t, string_t> ex_string_list[] = {
            std::make_tuple(ex_none, ""),
            std::make_tuple(ex_unit, "UNIT ERROR"),
            std::make_tuple(ex_ast, "AST ERROR"),
            std::make_tuple(ex_parser, "PARSER ERROR"),
            std::make_tuple(ex_vm, "VM ERROR"),
            std::make_tuple(ex_gen, "GEN ERROR"),
            std::make_tuple(ex_gui, "GUI ERROR"),
            std::make_tuple(ex_mem, "MEMORY ERROR"),
            std::make_tuple(ex_vfs, "VFS ERROR"),
    };

    const string_t& ex_str(ex_t t) {
        assert(t >= ex_none && t <= ex_vfs);
        return std::get<1>(ex_string_list[t]);
    }

    cexception::cexception(ex_t e, const string_t & msg) noexcept : msg(msg), e(e) {}

    string_t cexception::message() const {
        return ex_str(e) + ": " + msg;
    }
}
