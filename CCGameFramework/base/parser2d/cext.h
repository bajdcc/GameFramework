//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CEXT_H
#define CPARSER_CEXT_H

#include "cvfs.h"

namespace clib {

    class cext {
    public:
        virtual int ext_load(const std::string& name, vfs_func_t* f) = 0;
        virtual int ext_unload(const std::string& name) = 0;
        virtual std::string ext_get_path(const std::string& name) const = 0;
    };
}

#endif //CPARSER_CEXT_H
