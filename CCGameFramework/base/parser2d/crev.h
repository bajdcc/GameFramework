//
// Project: clibparser
// Author: bajdcc
//

#ifndef CLIBPARSER_CREV_H
#define CLIBPARSER_CREV_H

#include "types.h"

namespace clib {

    class crev {
    public:
        static void error(const string_t&);
        static std::vector<byte> conv(const std::vector<byte>& file);
    };
}

#endif //CMINILANG_VM_H
