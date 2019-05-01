//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_CNET_H
#define CLIBPARSER_CNET_H

#include "types.h"

namespace clib {

    class cnet {
    public:
        cnet() = default;

        cnet(const cnet&) = delete;
        cnet& operator=(const cnet&) = delete;

        string_t http_get(const string_t& url);
    private:
    };
}

#endif //CLIBPARSER_CNET_H
