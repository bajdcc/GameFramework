#include "pch.h"

#include "../CCGameFramework/base/parser2d/cexception.h"

namespace clib {

    string_t limit_string(const string_t& s, uint len) {
        if (s.length() <= len) {
            return s;
        }
        else {
            return s.substr(0, __max(0, len - 3)) + "...";
        }
    }
}