//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsmem.h"
#include <cassert>

namespace clib {

    char *cjsmem::alloc(size_t size) {
        auto da = std::make_shared<std::vector<char>>(size);
        if (removed.empty()) {
            map_data.insert({ da->data(), data.size() });
            data.push_back(da);
            return data.back()->data();
        }
        auto i = *removed.begin();
        map_data.insert({ da->data(), i });
        removed.erase(removed.begin());
        data[i] = da;
        return da->data();
    }

    void cjsmem::free(char *ptr) {
        auto f = map_data.find(ptr);
        if (f != map_data.end()) {
            assert(data[f->second]->data() == ptr);
            data[f->second] = nullptr;
            removed.insert(f->second);
            map_data.erase(ptr);
        }
        else {
            assert(!"invalid free");
        }
    }

    void cjsmem::gc() {

    }

    void cjsmem::reset() {
        data.clear();
        map_data.clear();
        removed.clear();
    }
}