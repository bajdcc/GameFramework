//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSMEM_H
#define CLIBJS_CJSMEM_H

#include <vector>
#include <unordered_map>
#include <set>

namespace clib {

    class cjsmem {
    public:
        cjsmem() = default;
        ~cjsmem() = default;

        cjsmem(const cjsmem&) = delete;
        cjsmem& operator=(const cjsmem&) = delete;

        char *alloc(size_t size);
        void free(char *ptr);
        void gc();
        void reset();

    private:
        std::vector<std::shared_ptr<std::vector<char>>> data;
        std::unordered_map<char *, size_t> map_data;
        std::unordered_set<int> removed;
    };
}

#endif //CLIBJS_CJSMEM_H
