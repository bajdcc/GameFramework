//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CWND_H
#define CPARSER_CWND_H

#include "types.h"
#include <ui\window\Window.h>

namespace clib {
    class cwindow {
    public:
        explicit cwindow(const string_t& caption, const CRect& location);
        ~cwindow();

    private:
        string_t caption;
        CRect location;
    };
}

#endif //CPARSER_CWND_H
