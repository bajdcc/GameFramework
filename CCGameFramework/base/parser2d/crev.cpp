//
// Project: clibparser
// Author: bajdcc
//

#include "stdafx.h"
#include "crev.h"
#include "cgen.h"
#include "cexception.h"

namespace clib {

    void crev::error(const string_t& str) {
        throw cexception(ex_ast, str);
    }

    std::vector<byte> crev::conv(const std::vector<byte>& file)
    {
        PE* pe = (PE*)file.data();
        auto text_size = pe->text_len / sizeof(int);
        auto text_start = (uint32_t*)(&pe->data + pe->data_len);
        std::stringstream ss;
        static char sz[16];
        for (auto i = 0U; i < text_size;) {
            snprintf(sz, sizeof(sz), "%-4s", INS_STRING((ins_t)text_start[i]).c_str());
            ss << sz;
            auto n = INS_OPNUM((ins_t)text_start[i]);
            if (n >= 1) {
                snprintf(sz, sizeof(sz), " 0x%08X", text_start[i + 1]);
                ss << sz;
            }
            if (n >= 2) {
                snprintf(sz, sizeof(sz), " 0x%08X", text_start[i + 2]);
                ss << sz;
            }
            ss << std::endl;
            i += n + 1;
        }
        auto s = ss.str();
        std::vector<byte> v(s.begin(), s.end());
        return v;
    }
}