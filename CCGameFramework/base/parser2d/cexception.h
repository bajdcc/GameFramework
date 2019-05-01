//
// Project: clibparser
// Author: bajdcc
//
#ifndef CLIBVM_EXCEPTION_H
#define CLIBVM_EXCEPTION_H

#include <exception>
#include "types.h"

namespace clib {

    enum ex_t {
        ex_none,
        ex_unit,
        ex_ast,
        ex_parser,
        ex_vm,
        ex_gen,
        ex_gui,
        ex_mem,
        ex_vfs,
    };

    const string_t& ex_str(ex_t);

    class cexception : public std::exception {
    public:
        explicit cexception(ex_t e, const string_t& msg) noexcept;

        cexception(const cexception& e) = default;
        cexception& operator = (const cexception& e) = default;

        string_t message() const;

        string_t msg;
        ex_t e{ ex_none };
    };
}

#endif
